#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <netdissect-stdinc.h>

#include <string.h>

#include "netdissect.h"
#include "extract.h"
#include "addrtoname.h"

static const char tstr[] = "[|RPKI-RTR]";

typedef struct {
    union {
        struct {
            u_char pdu_type;
            u_char version;
        } a;
        u_char session_id[2];
        u_char error_code[2];
    } u;
    u_char length[4];
} rpki_rtr_pdu;

#define RPKI_RTR_PDU_OVERHEAD (offsetof(rpki_rtr_pdu, rpki_rtr_pdu_msg))

typedef struct {
    rpki_rtr_pdu pdu_header;
    u_char flags;
    u_char prefix_length;
    u_char max_length;
    u_char zero;
    u_char prefix[4];
    u_char as[4];
} rpki_rtr_pdu_ipv4_prefix;

typedef struct {
    rpki_rtr_pdu pdu_header;
    u_char flags;
    u_char prefix_length;
    u_char max_length;
    u_char zero;
    u_char prefix[16];
    u_char as[4];
} rpki_rtr_pdu_ipv6_prefix;

typedef struct {
    rpki_rtr_pdu pdu_header;
    u_char encapsulated_pdu_length[4];
} rpki_rtr_pdu_error_report;

static const struct tok rpki_rtr_pdu_values[] = {
    { 0, "Serial Notify" },
    { 1, "Serial Query" },
    { 2, "Reset Query" },
    { 3, "Cache Response" },
    { 4, "IPV4 Prefix" },
    { 6, "IPV6 Prefix" },
    { 7, "End of Data" },
    { 8, "Cache Reset" },
    { 10, "Error Report" },
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

static char *indent_string(u_int indent) {
    static char buf[20];
    u_int idx = 0;
    buf[idx] = '\0';
    if (sizeof(buf) < ((indent / 8) + (indent % 8) + 2)) {
        return buf;
    }
    buf[idx] = '\n';
    idx++;
    while (indent >= 8) {
        buf[idx] = '\t';
        idx++;
        indent -= 8;
    }
    while (indent > 0) {
        buf[idx] = ' ';
        idx++;
        indent--;
    }
    buf[idx] = '\0';
    return buf;
}

static int rpki_rtr_pdu_print(netdissect_options *ndo, const u_char *tptr, u_int indent) {
    const rpki_rtr_pdu *pdu_header;
    u_int pdu_type, pdu_len, hexdump;
    const u_char *msg;

    pdu_header = (const rpki_rtr_pdu *)tptr;
    pdu_type = pdu_header->u.a.pdu_type;
    pdu_len = EXTRACT_32BITS(pdu_header->length);
    ND_TCHECK2(*tptr, pdu_len);
    hexdump = FALSE;

    ND_PRINT((ndo, "%sRPKI-RTRv%u, %s PDU (%u), length: %u",
              indent_string(8),
              pdu_header->u.a.version,
              tok2str(rpki_rtr_pdu_values, "Unknown", pdu_type),
              pdu_type, pdu_len));

    switch (pdu_type) {
    case 0:
    case 1:
    case 7:
        msg = (const u_char *)(pdu_header + 1);
        ND_PRINT((ndo, "%sSession ID: 0x%04x, Serial: %u",
                  indent_string(indent + 2),
                  EXTRACT_16BITS(pdu_header->u.session_id),
                  EXTRACT_32BITS(msg)));
        break;
    case 2:
    case 8:
        break;
    case 3:
        ND_PRINT((ndo, "%sSession ID: 0x%04x",
                  indent_string(indent + 2),
                  EXTRACT_16BITS(pdu_header->u.session_id)));
        break;
    case 4: {
        const rpki_rtr_pdu_ipv4_prefix *pdu;
        pdu = (const rpki_rtr_pdu_ipv4_prefix *)tptr;
        ND_PRINT((ndo, "%sIPv4 Prefix %s/%u-%u, origin-as %u, flags 0x%02x",
                  indent_string(indent + 2),
                  ipaddr_string(ndo, pdu->prefix),
                  pdu->prefix_length, pdu->max_length,
                  EXTRACT_32BITS(pdu->as), pdu->flags));
    } break;
    case 6: {
        const rpki_rtr_pdu_ipv6_prefix *pdu;
        pdu = (const rpki_rtr_pdu_ipv6_prefix *)tptr;
        ND_PRINT((ndo, "%sIPv6 Prefix %s/%u-%u, origin-as %u, flags 0x%02x",
                  indent_string(indent + 2),
                  ip6addr_string(ndo, pdu->prefix),
                  pdu->prefix_length, pdu->max_length,
                  EXTRACT_32BITS(pdu->as), pdu->flags));
    } break;
    case 10: {
        const rpki_rtr_pdu_error_report *pdu;
        u_int encapsulated_pdu_length, text_length, tlen, error_code;

        pdu = (const rpki_rtr_pdu_error_report *)tptr;
        encapsulated_pdu_length = EXTRACT_32BITS(pdu->encapsulated_pdu_length);
        ND_TCHECK2(*tptr, encapsulated_pdu_length);
        tlen = pdu_len;

        error_code = EXTRACT_16BITS(pdu->pdu_header.u.error_code);
        ND_PRINT((ndo, "%sError code: %s (%u), Encapsulated PDU length: %u",
                  indent_string(indent + 2),
                  tok2str(rpki_rtr_error_codes, "Unknown", error_code),
                  error_code, encapsulated_pdu_length));

        tptr += sizeof(*pdu);
        tlen -= sizeof(*pdu);

        if (encapsulated_pdu_length && (encapsulated_pdu_length <= tlen)) {
            ND_PRINT((ndo, "%s-----encapsulated PDU-----", indent_string(indent + 4)));
            if (rpki_rtr_pdu_print(ndo, tptr, indent + 2))
                goto trunc;
        }

        tptr += encapsulated_pdu_length;
        tlen -= encapsulated_pdu_length;

        text_length = 0;
        if (tlen > 4) {
            text_length = EXTRACT_32BITS(tptr);
            tptr += 4;
            tlen -= 4;
        }
        ND_TCHECK2(*tptr, text_length);
        if (text_length && (text_length <= tlen)) {
            ND_PRINT((ndo, "%sError text: ", indent_string(indent + 2)));
            if (fn_printn(ndo, tptr, text_length, ndo->ndo_snapend))
                goto trunc;
        }
    } break;
    default:
        hexdump = TRUE;
    }

    if (ndo->ndo_vflag > 1 || (ndo->ndo_vflag && hexdump)) {
        print_unknown_data(ndo, tptr, "\n\t  ", pdu_len);
    }
    return 0;

trunc:
    return 1;
}

void rpki_rtr_print(netdissect_options *ndo, register const u_char *pptr, register u_int len) {
    const rpki_rtr_pdu *pdu_header;
    const u_char *tptr = pptr;
    u_int tlen = len;

    if (!ndo->ndo_vflag) {
        ND_PRINT((ndo, ", RPKI-RTR"));
        return;
    }

    while (tlen >= sizeof(rpki_rtr_pdu)) {
        ND_TCHECK2(*tptr, sizeof(rpki_rtr_pdu));

        pdu_header = (const rpki_rtr_pdu *)tptr;
        u_int pdu_type = pdu_header->u.a.pdu_type;
        u_int pdu_len = EXTRACT_32BITS(pdu_header->length);
        ND_TCHECK2(*tptr, pdu_len);

        if (!pdu_type || !pdu_len) {
            break;
        }

        if (tlen < pdu_len) {
            goto trunc;
        }

        if (rpki_rtr_pdu_print(ndo, tptr, 8))
            goto trunc;

        tlen -= pdu_len;
        tptr += pdu_len;
    }
    return;
trunc:
    ND_PRINT((ndo, "\n\t%s", tstr));
}