#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <netdissect-stdinc.h>

#include <string.h>

#include "ip6.h"

#include "netdissect.h"
#include "addrtoname.h"
#include "extract.h"

int rt6_print_recursive(netdissect_options *ndo, const u_char *bp, const u_char *bp2 _U_, int i, int len, const struct in6_addr *addr, const u_char *ep) {
	if (i >= len) {
		ND_PRINT((ndo, ") "));
		return len;
	}

	if ((const u_char *)(addr + 1) > ep) {
		ND_PRINT((ndo, "[|srcrt]"));
		return -1;
	}

	ND_PRINT((ndo, ", [%d]%s", i, ip6addr_string(ndo, addr)));
	return rt6_print_recursive(ndo, bp, bp2, i + 1, len, addr + 1, ep);
}

int rt6_print(netdissect_options *ndo, const u_char *bp, const u_char *bp2 _U_) {
	const struct ip6_rthdr *dp;
	const struct ip6_rthdr0 *dp0;
	const u_char *ep;
	int len;
	const struct in6_addr *addr;

	dp = (const struct ip6_rthdr *)bp;
	len = dp->ip6r_len;

	ep = ndo->ndo_snapend;

	ND_TCHECK(dp->ip6r_segleft);

	ND_PRINT((ndo, "srcrt (len=%d", dp->ip6r_len));
	ND_PRINT((ndo, ", type=%d", dp->ip6r_type));
	ND_PRINT((ndo, ", segleft=%d", dp->ip6r_segleft));

	switch (dp->ip6r_type) {
		case IPV6_RTHDR_TYPE_0:
		case IPV6_RTHDR_TYPE_2:
			dp0 = (const struct ip6_rthdr0 *)dp;

			ND_TCHECK(dp0->ip6r0_reserved);
			if (dp0->ip6r0_reserved || ndo->ndo_vflag) {
				ND_PRINT((ndo, ", rsv=0x%0x",
				    EXTRACT_32BITS(&dp0->ip6r0_reserved)));
			}

			if (len % 2 == 1) {
				ND_PRINT((ndo, "[|srcrt]"));
				return -1;
			}
			len >>= 1;
			addr = &dp0->ip6r0_addr[0];
			return ((dp0->ip6r0_len + 1) << 3) * rt6_print_recursive(ndo, bp, bp2, 0, len, addr, ep);
		default:
			ND_PRINT((ndo, "[|srcrt]"));
			return -1;
	}
}