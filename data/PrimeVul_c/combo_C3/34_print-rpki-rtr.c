#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <netdissect-stdinc.h>
#include <string.h>
#include "netdissect.h"
#include "extract.h"
#include "addrtoname.h"

static const char tstr[] = "[|RPKI-RTR]";

typedef struct rpki_rtr_pdu_ {
    u_char version;
    u_char pdu_type;
    union {
        u_char session_id[2];
        u_char error_code[2];
    } u;
    u_char length[4];
} rpki_rtr_pdu;

typedef struct rpki_rtr_pdu_ipv4_prefix_ {
    rpki_rtr_pdu pdu_header;
    u_char flags;
    u_char prefix_length;
    u_char max_length;
    u_char zero;
    u_char prefix[4];
    u_char as[4];
} rpki_rtr_pdu_ipv4_prefix;

typedef struct rpki_rtr_pdu_ipv6_prefix_ {
    rpki_rtr_pdu pdu_header;
    u_char flags;
    u_char prefix_length;
    u_char max_length;
    u_char zero;
    u_char prefix[16];
    u_char as[4];
} rpki_rtr_pdu_ipv6_prefix;

typedef struct rpki_rtr_pdu_error_report_ {
    rpki_rtr_pdu pdu_header;
    u_char encapsulated_pdu_length[4];
} rpki_rtr_pdu_error_report;

#define RPKI_RTR_SERIAL_NOTIFY_PDU	0
#define RPKI_RTR_SERIAL_QUERY_PDU	1
#define RPKI_RTR_RESET_QUERY_PDU	2
#define RPKI_RTR_CACHE_RESPONSE_PDU	3
#define RPKI_RTR_IPV4_PREFIX_PDU	4
#define RPKI_RTR_IPV6_PREFIX_PDU	6
#define RPKI_RTR_END_OF_DATA_PDU	7
#define RPKI_RTR_CACHE_RESET_PDU	8
#define RPKI_RTR_ERROR_REPORT_PDU	10

static const struct tok rpki_rtr_pdu_values[] = {
    { RPKI_RTR_SERIAL_NOTIFY_PDU, "Serial Notify" },
    { RPKI_RTR_SERIAL_QUERY_PDU, "Serial Query" },
    { RPKI_RTR_RESET_QUERY_PDU, "Reset Query" },
    { RPKI_RTR_CACHE_RESPONSE_PDU, "Cache Response" },
    { RPKI_RTR_IPV4_PREFIX_PDU, "IPV4 Prefix" },
    { RPKI_RTR_IPV6_PREFIX_PDU, "IPV6 Prefix" },
    { RPKI_RTR_END_OF_DATA_PDU, "End of Data" },
    { RPKI_RTR_CACHE_RESET_PDU, "Cache Reset" },
    { RPKI_RTR_ERROR_REPORT_PDU, "Error Report" },
    { 0, NULL}
};

static const struct tok rpki_rtr_error_codes[] = {
    { 0, "Corrupt Data" },
    { 1, "Internal Error" },
    { 2, "No Data Available" },
    { 3, "Invalid Request" },
    { 4, "Unsupported Protocol Version" },
    { 5, "Unsupported PDU Type" },
    { 6, "Withdrawal of Unknown Record" },
    { 7, "Duplicate Announcement Received" },
    { 0, NULL}
};

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, PRINT
} Instruction;

typedef struct {
    int stack[256];
    int sp;
    int pc;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_run(VM *vm, const Instruction *program, int program_len, netdissect_options *ndo, const u_char *tptr, u_int indent) {
    while (vm->pc < program_len) {
        switch (program[vm->pc]) {
            case PUSH:
                vm->stack[++vm->sp] = (int)tptr;
                vm->pc++;
                break;
            case POP:
                vm->sp--;
                vm->pc++;
                break;
            case PRINT: {
                const rpki_rtr_pdu *pdu_header = (const rpki_rtr_pdu *)vm->stack[vm->sp];
                u_int pdu_type = pdu_header->pdu_type;
                u_int pdu_len = EXTRACT_32BITS(pdu_header->length);
                ND_TCHECK2(*tptr, pdu_len);
                ND_PRINT((ndo, "%sRPKI-RTRv%u, %s PDU (%u), length: %u",
                   indent_string(8),
                   pdu_header->version,
                   tok2str(rpki_rtr_pdu_values, "Unknown", pdu_type),
                   pdu_type, pdu_len));
                vm->pc++;
                break;
            }
            default:
                vm->pc++;
                break;
        }
    }
}

void rpki_rtr_print(netdissect_options *ndo, register const u_char *pptr, register u_int len) {
    VM vm;
    vm_init(&vm);

    const Instruction program[] = {PUSH, PRINT};
    const int program_len = sizeof(program) / sizeof(program[0]);

    u_int tlen = len;
    const u_char *tptr = pptr;

    if (!ndo->ndo_vflag) {
        ND_PRINT((ndo, ", RPKI-RTR"));
        return;
    }

    while (tlen >= sizeof(rpki_rtr_pdu)) {
        ND_TCHECK2(*tptr, sizeof(rpki_rtr_pdu));

        const rpki_rtr_pdu *pdu_header = (const rpki_rtr_pdu *)tptr;
        u_int pdu_type = pdu_header->pdu_type;
        u_int pdu_len = EXTRACT_32BITS(pdu_header->length);
        ND_TCHECK2(*tptr, pdu_len);

        if (!pdu_type || !pdu_len) {
            break;
        }

        if (tlen < pdu_len) {
            goto trunc;
        }

        vm_run(&vm, program, program_len, ndo, tptr, 8);

        tlen -= pdu_len;
        tptr += pdu_len;
    }
    return;

trunc:
    ND_PRINT((ndo, "\n\t%s", tstr));
}