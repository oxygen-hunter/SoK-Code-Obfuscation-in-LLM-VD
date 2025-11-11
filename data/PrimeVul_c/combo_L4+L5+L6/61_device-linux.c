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

int setup_deviceinfo_recursive(struct AdvPrefix *prefix, int if_prefix_len, char *iface_name) {
	if (!prefix) 
		return 0;
	if ((if_prefix_len != -1) && (if_prefix_len != prefix->PrefixLen)) {
		flog(LOG_WARNING, "prefix length should be %d for %s", if_prefix_len, iface_name);
	}
	return setup_deviceinfo_recursive(prefix->next, if_prefix_len, iface_name);
}

int setup_deviceinfo(struct Interface *iface)
{
	struct ifreq	ifr;
	char zero[sizeof(iface->if_addr)];

	strncpy(ifr.ifr_name, iface->Name, IFNAMSIZ-1);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';

	if (ioctl(sock, SIOCGIFMTU, &ifr) < 0) {
		flog(LOG_ERR, "ioctl(SIOCGIFMTU) failed for %s: %s",
			iface->Name, strerror(errno));
		return (-1);
	}

	dlog(LOG_DEBUG, 3, "mtu for %s is %d", iface->Name, ifr.ifr_mtu);
	iface->if_maxmtu = ifr.ifr_mtu;

	if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
	{
		flog(LOG_ERR, "ioctl(SIOCGIFHWADDR) failed for %s: %s",
			iface->Name, strerror(errno));
		return (-1);
	}

	dlog(LOG_DEBUG, 3, "hardware type for %s is %d", iface->Name,
		ifr.ifr_hwaddr.sa_family);

	int hw_family = ifr.ifr_hwaddr.sa_family;
	if (hw_family == ARPHRD_ETHER) {
		iface->if_hwaddr_len = 48;
		iface->if_prefix_len = 64;
	} else if (hw_family == ARPHRD_FDDI) {
		iface->if_hwaddr_len = 48;
		iface->if_prefix_len = 64;
	} else if (hw_family == ARPHRD_ARCNET) {
		iface->if_hwaddr_len = 8;
		iface->if_prefix_len = -1;
		iface->if_maxmtu = -1;
	} else {
		iface->if_hwaddr_len = -1;
		iface->if_prefix_len = -1;
		iface->if_maxmtu = -1;
	}

	dlog(LOG_DEBUG, 3, "link layer token length for %s is %d", iface->Name,
		iface->if_hwaddr_len);

	dlog(LOG_DEBUG, 3, "prefix length for %s is %d", iface->Name,
		iface->if_prefix_len);

	if (iface->if_hwaddr_len != -1) {
		unsigned int if_hwaddr_len_bytes = (iface->if_hwaddr_len + 7) >> 3;

		if (if_hwaddr_len_bytes > sizeof(iface->if_hwaddr)) {
			flog(LOG_ERR, "address length %d too big for %s", if_hwaddr_len_bytes, iface->Name);
			return(-2);
		}
		memcpy(iface->if_hwaddr, ifr.ifr_hwaddr.sa_data, if_hwaddr_len_bytes);

		memset(zero, 0, sizeof(zero));
		if (!memcmp(iface->if_hwaddr, zero, if_hwaddr_len_bytes))
			flog(LOG_WARNING, "WARNING, MAC address on %s is all zero!",
				iface->Name);
	}

	return setup_deviceinfo_recursive(iface->AdvPrefixList, iface->if_prefix_len, iface->Name);
}

int setup_linklocal_addr_recursive(FILE *fp, struct Interface *iface) {
	char str_addr[40];
	unsigned int plen, scope, dad_status, if_idx;
	char devname[IFNAMSIZ];

	if (fscanf(fp, "%32s %x %02x %02x %02x %15s\n",
		      str_addr, &if_idx, &plen, &scope, &dad_status,
		      devname) == EOF) {
		flog(LOG_ERR, "no linklocal address configured for %s", iface->Name);
		fclose(fp);
		return (-1);
	}

	if (scope == IPV6_ADDR_LINKLOCAL && strcmp(devname, iface->Name) == 0) {
		struct in6_addr addr;
		unsigned int ap;
		int i;
		for (i = 0; i < 16; i++) {
			sscanf(str_addr + i * 2, "%02x", &ap);
			addr.s6_addr[i] = (unsigned char)ap;
		}
		memcpy(&iface->if_addr, &addr, sizeof(iface->if_addr));

		iface->if_index = if_idx;
		fclose(fp);
		return 0;
	}

	return setup_linklocal_addr_recursive(fp, iface);
}

int setup_linklocal_addr(struct Interface *iface)
{
	FILE *fp;
	if ((fp = fopen(PATH_PROC_NET_IF_INET6, "r")) == NULL)
	{
		flog(LOG_ERR, "can't open %s: %s", PATH_PROC_NET_IF_INET6,
			strerror(errno));
		return (-1);
	}

	return setup_linklocal_addr_recursive(fp, iface);
}

int check_allrouters_membership_recursive(FILE *fp, struct Interface *iface, unsigned int *allrouters_ok) {
	char addr[32+1];
	char buffer[301] = {""}, *str;
	int ret = 0;
	unsigned int if_idx;

	str = fgets(buffer, 300, fp);
	if (!str)
		return 0;

	ret = sscanf(str, "%u %*s %32[0-9A-Fa-f]", &if_idx, addr);
	if (ret == 2 && iface->if_index == if_idx && strncmp(addr, "ff020000000000000000000000000002", sizeof(addr)) == 0) {
		*allrouters_ok = 1;
		return 0;
	}

	return check_allrouters_membership_recursive(fp, iface, allrouters_ok);
}

int check_allrouters_membership(struct Interface *iface)
{
	FILE *fp;
	unsigned int allrouters_ok = 0;

	if ((fp = fopen(PATH_PROC_NET_IGMP6, "r")) == NULL)
	{
		flog(LOG_ERR, "can't open %s: %s", PATH_PROC_NET_IGMP6,
			strerror(errno));
		return (-1);
	}

	check_allrouters_membership_recursive(fp, iface, &allrouters_ok);
	fclose(fp);

	if (!allrouters_ok) {
		flog(LOG_WARNING, "resetting ipv6-allrouters membership on %s", iface->Name);
		setup_allrouters_membership(iface);
	}

	return(0);
}