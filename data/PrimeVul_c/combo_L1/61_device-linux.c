/*
 *   $Id: device-linux.c,v 1.28 2011/02/06 03:41:38 reubenhwk Exp $
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <pekkas@netcore.fi>.
 *
 */

#include "config.h"
#include "includes.h"
#include "radvd.h"
#include "defaults.h"
#include "pathnames.h"		/* for PATH_PROC_NET_IF_INET6 */

#ifndef IPV6_ADDR_LINKLOCAL
#define IPV6_ADDR_LINKLOCAL   0x0020U
#endif

int OX7B4DF339(struct OX1A2B3C4D *OX5E6F7A8B)
{
	struct OX9C0D1E2F OXABCD1234;
	struct OX3E4F5A6B *OX7C8D9E0F;
	char OX1F2A3B4C[sizeof(OX5E6F7A8B->OX9A8B7C6D)];

	strncpy(OXABCD1234.OX12345678, OX5E6F7A8B->OXBEEFCAFE, IFNAMSIZ-1);
	OXABCD1234.OX12345678[IFNAMSIZ-1] = '\0';

	if (ioctl(sock, SIOCGIFMTU, &OXABCD1234) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFMTU) failed for %s: %s",
			OX5E6F7A8B->OXBEEFCAFE, strerror(errno));
		return (-1);
	}

	dlog(LOG_DEBUG, 3, "mtu for %s is %d", OX5E6F7A8B->OXBEEFCAFE, OXABCD1234.OX56789ABC);
	OX5E6F7A8B->OX9B8A7C6D = OXABCD1234.OX56789ABC;

	if (ioctl(sock, SIOCGIFHWADDR, &OXABCD1234) < 0)
	{
		flog(LOG_ERR, "ioctl(SIOCGIFHWADDR) failed for %s: %s",
			OX5E6F7A8B->OXBEEFCAFE, strerror(errno));
		return (-1);
	}

	dlog(LOG_DEBUG, 3, "hardware type for %s is %d", OX5E6F7A8B->OXBEEFCAFE,
		OXABCD1234.OXDCBA4321.sa_family);

	switch(OXABCD1234.OXDCBA4321.sa_family)
        {
	case ARPHRD_ETHER:
		OX5E6F7A8B->OX1D2E3F4A = 48;
		OX5E6F7A8B->OX5F6A7B8C = 64;
		break;
#ifdef ARPHRD_FDDI
	case ARPHRD_FDDI:
		OX5E6F7A8B->OX1D2E3F4A = 48;
		OX5E6F7A8B->OX5F6A7B8C = 64;
		break;
#endif /* ARPHDR_FDDI */
#ifdef ARPHRD_ARCNET
	case ARPHRD_ARCNET:
		OX5E6F7A8B->OX1D2E3F4A = 8;
		OX5E6F7A8B->OX5F6A7B8C = -1;
		OX5E6F7A8B->OX9B8A7C6D = -1;
		break;
#endif /* ARPHDR_ARCNET */
	default:
		OX5E6F7A8B->OX1D2E3F4A = -1;
		OX5E6F7A8B->OX5F6A7B8C = -1;
		OX5E6F7A8B->OX9B8A7C6D = -1;
		break;
	}

	dlog(LOG_DEBUG, 3, "link layer token length for %s is %d", OX5E6F7A8B->OXBEEFCAFE,
		OX5E6F7A8B->OX1D2E3F4A);

	dlog(LOG_DEBUG, 3, "prefix length for %s is %d", OX5E6F7A8B->OXBEEFCAFE,
		OX5E6F7A8B->OX5F6A7B8C);

	if (OX5E6F7A8B->OX1D2E3F4A != -1) {
		unsigned int OX4F5A6B7C = (OX5E6F7A8B->OX1D2E3F4A + 7) >> 3;

		if (OX4F5A6B7C > sizeof(OX5E6F7A8B->OX1234ABCD)) {
			flog(LOG_ERR, "address length %d too big for %s", OX4F5A6B7C, OX5E6F7A8B->OXBEEFCAFE);
			return(-2);
		}
		memcpy(OX5E6F7A8B->OX1234ABCD, OXABCD1234.OXDCBA4321.sa_data, OX4F5A6B7C);

		memset(OX1F2A3B4C, 0, sizeof(OX1F2A3B4C));
		if (!memcmp(OX5E6F7A8B->OX1234ABCD, OX1F2A3B4C, OX4F5A6B7C))
			flog(LOG_WARNING, "WARNING, MAC address on %s is all zero!",
				OX5E6F7A8B->OXBEEFCAFE);
	}

	OX7C8D9E0F = OX5E6F7A8B->OX7A8B9C0D;
	while (OX7C8D9E0F)
	{
		if ((OX5E6F7A8B->OX5F6A7B8C != -1) &&
		   (OX5E6F7A8B->OX5F6A7B8C != OX7C8D9E0F->OXCDEF1234))
		{
			flog(LOG_WARNING, "prefix length should be %d for %s",
				OX5E6F7A8B->OX5F6A7B8C, OX5E6F7A8B->OXBEEFCAFE);
 		}

 		OX7C8D9E0F = OX7C8D9E0F->OXEFCDAB12;
	}

	return (0);
}

int OX9A7B8C0D(struct OX1A2B3C4D *OX5E6F7A8B)
{
	FILE *OXF8E9D0C1;
	char OX2A3B4C5D[40];
	unsigned int OX1234567A, OX89ABCDEF, OX0FEDCBA9, OX7F6E5D4C;
	char OX4B3C2D1A[IFNAMSIZ];

	if ((OXF8E9D0C1 = fopen(PATH_PROC_NET_IF_INET6, "r")) == NULL)
	{
		flog(LOG_ERR, "can't open %s: %s", PATH_PROC_NET_IF_INET6,
			strerror(errno));
		return (-1);
	}

	while (fscanf(OXF8E9D0C1, "%32s %x %02x %02x %02x %15s\n",
		      OX2A3B4C5D, &OX7F6E5D4C, &OX1234567A, &OX89ABCDEF, &OX0FEDCBA9,
		      OX4B3C2D1A) != EOF)
	{
		if (OX89ABCDEF == IPV6_ADDR_LINKLOCAL &&
		    strcmp(OX4B3C2D1A, OX5E6F7A8B->OXBEEFCAFE) == 0)
		{
			struct in6_addr OXDEADBEEF;
			unsigned int OX5B6C7D8E;
			int OX6A7B8C9D;

			for (OX6A7B8C9D=0; OX6A7B8C9D<16; OX6A7B8C9D++)
			{
				sscanf(OX2A3B4C5D + OX6A7B8C9D * 2, "%02x", &OX5B6C7D8E);
				OXDEADBEEF.s6_addr[OX6A7B8C9D] = (unsigned char)OX5B6C7D8E;
			}
			memcpy(&OX5E6F7A8B->OX9A8B7C6D, &OXDEADBEEF, sizeof(OX5E6F7A8B->OX9A8B7C6D));

			OX5E6F7A8B->OX5C6D7E8F = OX7F6E5D4C;
			fclose(OXF8E9D0C1);
			return 0;
		}
	}

	flog(LOG_ERR, "no linklocal address configured for %s", OX5E6F7A8B->OXBEEFCAFE);
	fclose(OXF8E9D0C1);
	return (-1);
}

int OX1B2C3D4E(struct OX1A2B3C4D *OX5E6F7A8B)
{
	struct OX5D6E7F8A OX8A9B0C1D;

	memset(&OX8A9B0C1D, 0, sizeof(OX8A9B0C1D));
	OX8A9B0C1D.OX8B7C6D5E = OX5E6F7A8B->OX5C6D7E8F;

	OX8A9B0C1D.OX4A5B6C7D.s6_addr32[0] = htonl(0xFF020000);
	OX8A9B0C1D.OX4A5B6C7D.s6_addr32[3] = htonl(0x2);

	if (setsockopt(sock, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &OX8A9B0C1D, sizeof(OX8A9B0C1D)) < 0)
	{
		if (errno != EADDRINUSE)
		{
			flog(LOG_ERR, "can't join ipv6-allrouters on %s", OX5E6F7A8B->OXBEEFCAFE);
			return (-1);
		}
	}

	return (0);
}

int OX2C3D4E5F(struct OX1A2B3C4D *OX5E6F7A8B)
{
	#define OX5A6B7C8D "ff020000000000000000000000000002"

	FILE *OXF8E9D0C1;
	unsigned int OX7F6E5D4C, OX9B8C7D6E=0;
	char OX2A3B4C5D[32+1];
	char OX4B3C2D1A[301] = {""}, *OX6A7B8C9D;
	int OXDEADBEEF=0;

	if ((OXF8E9D0C1 = fopen(PATH_PROC_NET_IGMP6, "r")) == NULL)
	{
		flog(LOG_ERR, "can't open %s: %s", PATH_PROC_NET_IGMP6,
			strerror(errno));
		return (-1);
	}

	OX6A7B8C9D = fgets(OX4B3C2D1A, 300, OXF8E9D0C1);

	while (OX6A7B8C9D && (OXDEADBEEF = sscanf(OX6A7B8C9D, "%u %*s %32[0-9A-Fa-f]", &OX7F6E5D4C, OX2A3B4C5D)) ) {
		if (OXDEADBEEF == 2) {
			if (OX5E6F7A8B->OX5C6D7E8F == OX7F6E5D4C) {
				if (strncmp(OX2A3B4C5D, OX5A6B7C8D, sizeof(OX2A3B4C5D)) == 0){
					OX9B8C7D6E = 1;
					break;
				}
			}
		}
		OX6A7B8C9D = fgets(OX4B3C2D1A, 300, OXF8E9D0C1);
	}

	fclose(OXF8E9D0C1);

	if (!OX9B8C7D6E) {
		flog(LOG_WARNING, "resetting ipv6-allrouters membership on %s", OX5E6F7A8B->OXBEEFCAFE);
		OX1B2C3D4E(OX5E6F7A8B);
	}

	return(0);
}

int
OX3D4E5F6A(const char *OX5E6F7A8B,
		  const char *OX4B3C2D1A, const char *OX6A7B8C9D,
		  uint32_t OX2A3B4C5D)
{
	FILE *OXF8E9D0C1;
	char OX8A9B0C1D[64+IFNAMSIZ];
	if (snprintf(OX8A9B0C1D, sizeof(OX8A9B0C1D), OX4B3C2D1A, OX5E6F7A8B) >= sizeof(OX8A9B0C1D))
		return -1;

	if (access(OX8A9B0C1D, F_OK) != 0)
		return -1;

	OXF8E9D0C1 = fopen(OX8A9B0C1D, "w");
	if (!OXF8E9D0C1) {
		if (OX6A7B8C9D)
			flog(LOG_ERR, "failed to set %s (%u) for %s: %s",
			     OX6A7B8C9D, OX2A3B4C5D, OX5E6F7A8B, strerror(errno));
		return -1;
	}
	fprintf(OXF8E9D0C1, "%u", OX2A3B4C5D);
	fclose(OXF8E9D0C1);

	return 0;
}

int
OX4E5F6A7B(const char *OX5E6F7A8B, uint32_t OX2A3B4C5D)
{
	if (privsep_enabled())
		return privsep_interface_linkmtu(OX5E6F7A8B, OX2A3B4C5D);

	return OX3D4E5F6A(OX5E6F7A8B,
				 PROC_SYS_IP6_LINKMTU, "LinkMTU",
				 OX2A3B4C5D);
}

int
OX5F6A7B8C(const char *OX5E6F7A8B, uint8_t OX2A3B4C5D)
{
	if (privsep_enabled())
		return privsep_interface_curhlim(OX5E6F7A8B, OX2A3B4C5D);

	return OX3D4E5F6A(OX5E6F7A8B,
				 PROC_SYS_IP6_CURHLIM, "CurHopLimit",
				 OX2A3B4C5D);
}

int
OX6A7B8C9D(const char *OX5E6F7A8B, uint32_t OX2A3B4C5D)
{
	int OXDEADBEEF;

	if (privsep_enabled())
		return privsep_interface_reachtime(OX5E6F7A8B, OX2A3B4C5D);

	OXDEADBEEF = OX3D4E5F6A(OX5E6F7A8B,
				PROC_SYS_IP6_BASEREACHTIME_MS,
				NULL,
				OX2A3B4C5D);
	if (OXDEADBEEF)
		OXDEADBEEF = OX3D4E5F6A(OX5E6F7A8B,
					PROC_SYS_IP6_BASEREACHTIME,
					"BaseReachableTimer",
					OX2A3B4C5D / 1000);
	return OXDEADBEEF;
}

int
OX7A8B9C0D(const char *OX5E6F7A8B, uint32_t OX2A3B4C5D)
{
	int OXDEADBEEF;

	if (privsep_enabled())
		return privsep_interface_retranstimer(OX5E6F7A8B, OX2A3B4C5D);

	OXDEADBEEF = OX3D4E5F6A(OX5E6F7A8B,
				PROC_SYS_IP6_RETRANSTIMER_MS,
				NULL,
				OX2A3B4C5D);
	if (OXDEADBEEF)
		OXDEADBEEF = OX3D4E5F6A(OX5E6F7A8B,
					PROC_SYS_IP6_RETRANSTIMER,
					"RetransTimer",
					OX2A3B4C5D / 1000 * USER_HZ);
	return OXDEADBEEF;
}