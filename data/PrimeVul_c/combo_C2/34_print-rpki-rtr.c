/*
 * Copyright (c) 1998-2011 The TCPDUMP project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code
 * distributions retain the above copyright notice and this paragraph
 * in its entirety, and (2) distributions including binary code include
 * the above copyright notice and this paragraph in its entirety in
 * the documentation or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * Original code by Hannes Gredler (hannes@gredler.at)
 */

/* \summary: Resource Public Key Infrastructure (RPKI) to Router Protocol printer */

/* specification: RFC 6810 */

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
#define RPKI_RTR_PDU_OVERHEAD (offsetof(rpki_rtr_pdu, rpki_rtr_pdu_msg))

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

static char *
indent_string (u_int indent)
{
    static char buf[20];
    u_int idx;

    idx = 0;
    buf[idx] = '\0';

    if (sizeof(buf) < ((indent/8) + (indent %8) + 2)) {
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

static int
rpki_rtr_pdu_print (netdissect_options *ndo, const u_char *tptr, u_int indent)
{
    const rpki_rtr_pdu *pdu_header;
    u_int pdu_type, pdu_len, hexdump;
    const u_char *msg;
    int state = 0;
    int done = 0;

    while (!done) {
        switch (state) {
            case 0:
                pdu_header = (const rpki_rtr_pdu *)tptr;
                pdu_type = pdu_header->pdu_type;
                pdu_len = EXTRACT_32BITS(pdu_header->length);
                ND_TCHECK2(*tptr, pdu_len);
                hexdump = FALSE;
                ND_PRINT((ndo, "%sRPKI-RTRv%u, %s PDU (%u), length: %u",
                       indent_string(8),
                       pdu_header->version,
                       tok2str(rpki_rtr_pdu_values, "Unknown", pdu_type),
                       pdu_type, pdu_len));
                if (pdu_type == RPKI_RTR_SERIAL_NOTIFY_PDU ||
                    pdu_type == RPKI_RTR_SERIAL_QUERY_PDU ||
                    pdu_type == RPKI_RTR_END_OF_DATA_PDU) {
                    state = 1;
                } else if (pdu_type == RPKI_RTR_RESET_QUERY_PDU ||
                           pdu_type == RPKI_RTR_CACHE_RESET_PDU) {
                    state = 2;
                } else if (pdu_type == RPKI_RTR_CACHE_RESPONSE_PDU) {
                    state = 3;
                } else if (pdu_type == RPKI_RTR_IPV4_PREFIX_PDU) {
                    state = 4;
                } else if (pdu_type == RPKI_RTR_IPV6_PREFIX_PDU) {
                    state = 5;
                } else if (pdu_type == RPKI_RTR_ERROR_REPORT_PDU) {
                    state = 6;
                } else {
                    state = 7;
                }
                break;

            case 1:
                msg = (const u_char *)(pdu_header + 1);
                ND_PRINT((ndo, "%sSession ID: 0x%04x, Serial: %u",
                       indent_string(indent+2),
                       EXTRACT_16BITS(pdu_header->u.session_id),
                       EXTRACT_32BITS(msg)));
                done = 1;
                break;

            case 2:
                done = 1;
                break;

            case 3:
                ND_PRINT((ndo, "%sSession ID: 0x%04x",
                       indent_string(indent+2),
                       EXTRACT_16BITS(pdu_header->u.session_id)));
                done = 1;
                break;

            case 4: {
                const rpki_rtr_pdu_ipv4_prefix *pdu;
                pdu = (const rpki_rtr_pdu_ipv4_prefix *)tptr;
                ND_PRINT((ndo, "%sIPv4 Prefix %s/%u-%u, origin-as %u, flags 0x%02x",
                       indent_string(indent+2),
                       ipaddr_string(ndo, pdu->prefix),
                       pdu->prefix_length, pdu->max_length,
                       EXTRACT_32BITS(pdu->as), pdu->flags));
                done = 1;
                break;
            }

            case 5: {
                const rpki_rtr_pdu_ipv6_prefix *pdu;
                pdu = (const rpki_rtr_pdu_ipv6_prefix *)tptr;
                ND_PRINT((ndo, "%sIPv6 Prefix %s/%u-%u, origin-as %u, flags 0x%02x",
                       indent_string(indent+2),
                       ip6addr_string(ndo, pdu->prefix),
                       pdu->prefix_length, pdu->max_length,
                       EXTRACT_32BITS(pdu->as), pdu->flags));
                done = 1;
                break;
            }

            case 6: {
                const rpki_rtr_pdu_error_report *pdu;
                u_int encapsulated_pdu_length, text_length, tlen, error_code;
                pdu = (const rpki_rtr_pdu_error_report *)tptr;
                encapsulated_pdu_length = EXTRACT_32BITS(pdu->encapsulated_pdu_length);
                ND_TCHECK2(*tptr, encapsulated_pdu_length);
                tlen = pdu_len;
                error_code = EXTRACT_16BITS(pdu->pdu_header.u.error_code);
                ND_PRINT((ndo, "%sError code: %s (%u), Encapsulated PDU length: %u",
                       indent_string(indent+2),
                       tok2str(rpki_rtr_error_codes, "Unknown", error_code),
                       error_code, encapsulated_pdu_length));
                tptr += sizeof(*pdu);
                tlen -= sizeof(*pdu);
                if (encapsulated_pdu_length && (encapsulated_pdu_length <= tlen)) {
                    ND_PRINT((ndo, "%s-----encapsulated PDU-----", indent_string(indent+4)));
                    if (rpki_rtr_pdu_print(ndo, tptr, indent+2))
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
                if (text_length && (text_length <= tlen )) {
                    ND_PRINT((ndo, "%sError text: ", indent_string(indent+2)));
                    if (fn_printn(ndo, tptr, text_length, ndo->ndo_snapend))
                        goto trunc;
                }
                done = 1;
                break;
            }

            case 7:
                hexdump = TRUE;
                done = 1;
                break;
        }
    }

    if (ndo->ndo_vflag > 1 || (ndo->ndo_vflag && hexdump)) {
	print_unknown_data(ndo,tptr,"\n\t  ", pdu_len);
    }
    return 0;

trunc:
    return 1;
}

void
rpki_rtr_print(netdissect_options *ndo, register const u_char *pptr, register u_int len)
{
    u_int tlen, pdu_type, pdu_len;
    const u_char *tptr;
    const rpki_rtr_pdu *pdu_header;
    int state = 0;
    int done = 0;

    tptr = pptr;
    tlen = len;

    if (!ndo->ndo_vflag) {
	ND_PRINT((ndo, ", RPKI-RTR"));
	return;
    }

    while (tlen >= sizeof(rpki_rtr_pdu)) {
        switch (state) {
            case 0:
                ND_TCHECK2(*tptr, sizeof(rpki_rtr_pdu));
                pdu_header = (const rpki_rtr_pdu *)tptr;
                pdu_type = pdu_header->pdu_type;
                pdu_len = EXTRACT_32BITS(pdu_header->length);
                ND_TCHECK2(*tptr, pdu_len);
                if (!pdu_type || !pdu_len) {
                    done = 1;
                } else if (tlen < pdu_len) {
                    goto trunc;
                } else {
                    if (rpki_rtr_pdu_print(ndo, tptr, 8))
                        goto trunc;
                    tlen -= pdu_len;
                    tptr += pdu_len;
                }
                break;
            default:
                done = 1;
                break;
        }
        if (done) break;
    }
    return;
trunc:
    ND_PRINT((ndo, "\n\t%s", tstr));
}