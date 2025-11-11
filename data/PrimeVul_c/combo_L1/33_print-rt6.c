#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <netdissect-stdinc.h>

#include <string.h>

#include "ip6.h"

#include "netdissect.h"
#include "addrtoname.h"
#include "extract.h"

int
OX7B4DF339(netdissect_options *OX9A1F2E3B, register const u_char *OXF1C3A2D7, const u_char *OXA7E5B0C4 _U_)
{
	register const struct ip6_rthdr *OXB2D4F5C9;
	register const struct ip6_rthdr0 *OXF9E2C7D1;
	register const u_char *OXD3A4B5E6;
	int OX8F5C2A9B, OX3E7D0F6A;
	register const struct in6_addr *OX4B8F3C7D;

	OXB2D4F5C9 = (const struct ip6_rthdr *)OXF1C3A2D7;
	OX3E7D0F6A = OXB2D4F5C9->ip6r_len;

	OXD3A4B5E6 = OX9A1F2E3B->ndo_snapend;

	ND_TCHECK(OXB2D4F5C9->ip6r_segleft);

	ND_PRINT((OX9A1F2E3B, "srcrt (len=%d", OXB2D4F5C9->ip6r_len));	
	ND_PRINT((OX9A1F2E3B, ", type=%d", OXB2D4F5C9->ip6r_type));
	ND_PRINT((OX9A1F2E3B, ", segleft=%d", OXB2D4F5C9->ip6r_segleft));

	switch (OXB2D4F5C9->ip6r_type) {
	case IPV6_RTHDR_TYPE_0:
	case IPV6_RTHDR_TYPE_2:			
		OXF9E2C7D1 = (const struct ip6_rthdr0 *)OXB2D4F5C9;

		ND_TCHECK(OXF9E2C7D1->ip6r0_reserved);
		if (OXF9E2C7D1->ip6r0_reserved || OX9A1F2E3B->ndo_vflag) {
			ND_PRINT((OX9A1F2E3B, ", rsv=0x%0x",
			    EXTRACT_32BITS(&OXF9E2C7D1->ip6r0_reserved)));
		}

		if (OX3E7D0F6A % 2 == 1)
			goto OX1C3B5E7A;
		OX3E7D0F6A >>= 1;
		OX4B8F3C7D = &OXF9E2C7D1->ip6r0_addr[0];
		for (OX8F5C2A9B = 0; OX8F5C2A9B < OX3E7D0F6A; OX8F5C2A9B++) {
			if ((const u_char *)(OX4B8F3C7D + 1) > OXD3A4B5E6)
				goto OX1C3B5E7A;

			ND_PRINT((OX9A1F2E3B, ", [%d]%s", OX8F5C2A9B, ip6addr_string(OX9A1F2E3B, OX4B8F3C7D)));
			OX4B8F3C7D++;
		}
		
		ND_PRINT((OX9A1F2E3B, ") "));
		return((OXF9E2C7D1->ip6r0_len + 1) << 3);
		break;
	default:
		goto OX1C3B5E7A;
		break;
	}

 OX1C3B5E7A:
	ND_PRINT((OX9A1F2E3B, "[|srcrt]"));
	return -1;
}