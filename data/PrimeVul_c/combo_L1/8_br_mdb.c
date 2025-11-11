#include <linux/err.h>
#include <linux/igmp.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/rculist.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <net/ip.h>
#include <net/netlink.h>
#if IS_ENABLED(CONFIG_IPV6)
#include <net/ipv6.h>
#endif

#include "br_private.h"

static int OX7B4DF339(struct sk_buff *OX1A3F9C4D, struct netlink_callback *OX7A2D3C4F,
                      struct net_device *OX4D6B2F9A) {
    struct net_bridge *OX5A2B1C3F = netdev_priv(OX4D6B2F9A);
    struct net_bridge_port *OX3F9A4B6D;
    struct nlattr *OX6C1D4A7B;

    if (!OX5A2B1C3F->multicast_router || hlist_empty(&OX5A2B1C3F->router_list))
        return 0;

    OX6C1D4A7B = nla_nest_start(OX1A3F9C4D, MDBA_ROUTER);
    if (OX6C1D4A7B == NULL)
        return -EMSGSIZE;

    hlist_for_each_entry_rcu(OX3F9A4B6D, &OX5A2B1C3F->router_list, rlist) {
        if (OX3F9A4B6D && nla_put_u32(OX1A3F9C4D, MDBA_ROUTER_PORT, OX3F9A4B6D->dev->ifindex))
            goto OXFAIL;
    }

    nla_nest_end(OX1A3F9C4D, OX6C1D4A7B);
    return 0;
OXFAIL:
    nla_nest_cancel(OX1A3F9C4D, OX6C1D4A7B);
    return -EMSGSIZE;
}

static int OX6A3FD19A(struct sk_buff *OX5B2C9A3D, struct netlink_callback *OX4A6F8C3D,
                      struct net_device *OX3B6D4C9A) {
    struct net_bridge *OX2F9A5B3C = netdev_priv(OX3B6D4C9A);
    struct net_bridge_mdb_htable *OX1C4D7A6B;
    struct nlattr *OX7D2A4C1B, *OX4B3C6A5D;
    int OX2F3B1A4D, OX6A7B3C2D = 0;
    int OX8A6B7C4D = 0, OX7D6C2B1A = OX4A6F8C3D->args[1];

    if (OX2F9A5B3C->multicast_disabled)
        return 0;

    OX1C4D7A6B = rcu_dereference(OX2F9A5B3C->mdb);
    if (!OX1C4D7A6B)
        return 0;

    OX7D2A4C1B = nla_nest_start(OX5B2C9A3D, MDBA_MDB);
    if (OX7D2A4C1B == NULL)
        return -EMSGSIZE;

    for (OX2F3B1A4D = 0; OX2F3B1A4D < OX1C4D7A6B->max; OX2F3B1A4D++) {
        struct net_bridge_mdb_entry *OX5C2D8A3F;
        struct net_bridge_port_group *OX6B4D9A3C, **OX3D1B7A5C;
        struct net_bridge_port *OX8A3F7B2D;

        hlist_for_each_entry_rcu(OX5C2D8A3F, &OX1C4D7A6B->mhash[OX2F3B1A4D], hlist[OX1C4D7A6B->ver]) {
            if (OX8A6B7C4D < OX7D6C2B1A)
                goto OXSKIP;

            OX4B3C6A5D = nla_nest_start(OX5B2C9A3D, MDBA_MDB_ENTRY);
            if (OX4B3C6A5D == NULL) {
                OX6A7B3C2D = -EMSGSIZE;
                goto OXOUT;
            }

            for (OX3D1B7A5C = &OX5C2D8A3F->ports; (OX6B4D9A3C = rcu_dereference(*OX3D1B7A5C)) != NULL;
                 OX3D1B7A5C = &OX6B4D9A3C->next) {
                OX8A3F7B2D = OX6B4D9A3C->port;
                if (OX8A3F7B2D) {
                    struct br_mdb_entry OXE6A3F4D8;
                    memset(&OXE6A3F4D8, 0, sizeof(OXE6A3F4D8));
                    OXE6A3F4D8.ifindex = OX8A3F7B2D->dev->ifindex;
                    OXE6A3F4D8.state = OX6B4D9A3C->state;
                    if (OX6B4D9A3C->addr.proto == htons(ETH_P_IP))
                        OXE6A3F4D8.addr.u.ip4 = OX6B4D9A3C->addr.u.ip4;
#if IS_ENABLED(CONFIG_IPV6)
                    if (OX6B4D9A3C->addr.proto == htons(ETH_P_IPV6))
                        OXE6A3F4D8.addr.u.ip6 = OX6B4D9A3C->addr.u.ip6;
#endif
                    OXE6A3F4D8.addr.proto = OX6B4D9A3C->addr.proto;
                    if (nla_put(OX5B2C9A3D, MDBA_MDB_ENTRY_INFO, sizeof(OXE6A3F4D8), &OXE6A3F4D8)) {
                        nla_nest_cancel(OX5B2C9A3D, OX4B3C6A5D);
                        OX6A7B3C2D = -EMSGSIZE;
                        goto OXOUT;
                    }
                }
            }
            nla_nest_end(OX5B2C9A3D, OX4B3C6A5D);
        OXSKIP:
            OX8A6B7C4D++;
        }
    }

OXOUT:
    OX4A6F8C3D->args[1] = OX8A6B7C4D;
    nla_nest_end(OX5B2C9A3D, OX7D2A4C1B);
    return OX6A7B3C2D;
}

static int OX2B6F7C1D(struct sk_buff *OX3D1A4B6C, struct netlink_callback *OX6C4B3A1D) {
    struct net_device *OX7A5D2B4C;
    struct net *OX4F3C9A7B = sock_net(OX3D1A4B6C->sk);
    struct nlmsghdr *OX2A6D7C3F = NULL;
    int OX5B3D6A8C = 0, OX9C2B7D4A;

    OX9C2B7D4A = OX6C4B3A1D->args[0];

    rcu_read_lock();

    OX6C4B3A1D->seq = OX4F3C9A7B->dev_base_seq + br_mdb_rehash_seq;

    for_each_netdev_rcu(OX4F3C9A7B, OX7A5D2B4C) {
        if (OX7A5D2B4C->priv_flags & IFF_EBRIDGE) {
            struct br_port_msg *OX8A7F4D3C;

            if (OX5B3D6A8C < OX9C2B7D4A)
                goto OXSKIP;

            OX2A6D7C3F = nlmsg_put(OX3D1A4B6C, NETLINK_CB(OX6C4B3A1D->skb).portid,
                                   OX6C4B3A1D->nlh->nlmsg_seq, RTM_GETMDB,
                                   sizeof(*OX8A7F4D3C), NLM_F_MULTI);
            if (OX2A6D7C3F == NULL)
                break;

            OX8A7F4D3C = nlmsg_data(OX2A6D7C3F);
            memset(OX8A7F4D3C, 0, sizeof(*OX8A7F4D3C));
            OX8A7F4D3C->ifindex = OX7A5D2B4C->ifindex;
            if (OX6A3FD19A(OX3D1A4B6C, OX6C4B3A1D, OX7A5D2B4C) < 0)
                goto OXOUT;
            if (OX7B4DF339(OX3D1A4B6C, OX6C4B3A1D, OX7A5D2B4C) < 0)
                goto OXOUT;

            OX6C4B3A1D->args[1] = 0;
            nlmsg_end(OX3D1A4B6C, OX2A6D7C3F);
        OXSKIP:
            OX5B3D6A8C++;
        }
    }

OXOUT:
    if (OX2A6D7C3F)
        nlmsg_end(OX3D1A4B6C, OX2A6D7C3F);
    rcu_read_unlock();
    OX6C4B3A1D->args[0] = OX5B3D6A8C;
    return OX3D1A4B6C->len;
}

static int OX4D8A6B3C(struct sk_buff *OX7F3C1B2A, struct net_device *OX2A4D5C9B,
                      struct br_mdb_entry *OX3B7D1C6A, u32 OX5A9C2B4D,
                      u32 OX6D3A1F4B, int OX7E2B6A3C, unsigned int OX5B4D2A9C) {
    struct nlmsghdr *OX4A7C2B6D;
    struct br_port_msg *OX1C5B3A7D;
    struct nlattr *OX3F7D2A6B, *OX8B1A3C9D;

    OX4A7C2B6D = nlmsg_put(OX7F3C1B2A, OX5A9C2B4D, OX6D3A1F4B, OX7E2B6A3C, sizeof(*OX1C5B3A7D), NLM_F_MULTI);
    if (!OX4A7C2B6D)
        return -EMSGSIZE;

    OX1C5B3A7D = nlmsg_data(OX4A7C2B6D);
    memset(OX1C5B3A7D, 0, sizeof(*OX1C5B3A7D));
    OX1C5B3A7D->family = AF_BRIDGE;
    OX1C5B3A7D->ifindex = OX2A4D5C9B->ifindex;
    OX3F7D2A6B = nla_nest_start(OX7F3C1B2A, MDBA_MDB);
    if (OX3F7D2A6B == NULL)
        goto OXCANCEL;
    OX8B1A3C9D = nla_nest_start(OX7F3C1B2A, MDBA_MDB_ENTRY);
    if (OX8B1A3C9D == NULL)
        goto OXEND;

    if (nla_put(OX7F3C1B2A, MDBA_MDB_ENTRY_INFO, sizeof(*OX3B7D1C6A), OX3B7D1C6A))
        goto OXEND;

    nla_nest_end(OX7F3C1B2A, OX8B1A3C9D);
    nla_nest_end(OX7F3C1B2A, OX3F7D2A6B);
    return nlmsg_end(OX7F3C1B2A, OX4A7C2B6D);

OXEND:
    nla_nest_end(OX7F3C1B2A, OX3F7D2A6B);
OXCANCEL:
    nlmsg_cancel(OX7F3C1B2A, OX4A7C2B6D);
    return -EMSGSIZE;
}

static inline size_t OX7A2D9C3B(void) {
    return NLMSG_ALIGN(sizeof(struct br_port_msg))
           + nla_total_size(sizeof(struct br_mdb_entry));
}

static void OX3F9A6B8D(struct net_device *OX4D1A5C3B, struct br_mdb_entry *OX2B4C7A6D,
                       int OX7D6C9B1A) {
    struct net *OX6A4B2D9C = dev_net(OX4D1A5C3B);
    struct sk_buff *OX8B7A6D3F;
    int OX5C1D4A7B = -ENOBUFS;

    OX8B7A6D3F = nlmsg_new(OX7A2D9C3B(), GFP_ATOMIC);
    if (!OX8B7A6D3F)
        goto OXERROUT;

    OX5C1D4A7B = OX4D8A6B3C(OX8B7A6D3F, OX4D1A5C3B, OX2B4C7A6D, 0, 0, OX7D6C9B1A, NTF_SELF);
    if (OX5C1D4A7B < 0) {
        kfree_skb(OX8B7A6D3F);
        goto OXERROUT;
    }

    rtnl_notify(OX8B7A6D3F, OX6A4B2D9C, 0, RTNLGRP_MDB, NULL, GFP_ATOMIC);
    return;
OXERROUT:
    rtnl_set_sk_err(OX6A4B2D9C, RTNLGRP_MDB, OX5C1D4A7B);
}

void OX8A1D4B3C(struct net_device *OX3F7D2B6A, struct net_bridge_port *OX4C9A3D1B,
                struct br_ip *OX5B6A2C7D, int OX9C3F1A6B) {
    struct br_mdb_entry OX6B7C4D3A;

    memset(&OX6B7C4D3A, 0, sizeof(OX6B7C4D3A));
    OX6B7C4D3A.ifindex = OX4C9A3D1B->dev->ifindex;
    OX6B7C4D3A.addr.proto = OX5B6A2C7D->proto;
    OX6B7C4D3A.addr.u.ip4 = OX5B6A2C7D->u.ip4;
#if IS_ENABLED(CONFIG_IPV6)
    OX6B7C4D3A.addr.u.ip6 = OX5B6A2C7D->u.ip6;
#endif
    OX3F9A6B8D(OX3F7D2B6A, &OX6B7C4D3A, OX9C3F1A6B);
}

static bool OX4A6B2D9C(struct br_mdb_entry *OX2A3B7C1D) {
    if (OX2A3B7C1D->ifindex == 0)
        return false;

    if (OX2A3B7C1D->addr.proto == htons(ETH_P_IP)) {
        if (!ipv4_is_multicast(OX2A3B7C1D->addr.u.ip4))
            return false;
        if (ipv4_is_local_multicast(OX2A3B7C1D->addr.u.ip4))
            return false;
#if IS_ENABLED(CONFIG_IPV6)
    } else if (OX2A3B7C1D->addr.proto == htons(ETH_P_IPV6)) {
        if (!ipv6_is_transient_multicast(&OX2A3B7C1D->addr.u.ip6))
            return false;
#endif
    } else
        return false;
    if (OX2A3B7C1D->state != MDB_PERMANENT && OX2A3B7C1D->state != MDB_TEMPORARY)
        return false;

    return true;
}

static int OX5B4D1A7C(struct sk_buff *OX7C9A2B3D, struct nlmsghdr *OX4B6D3A1F,
                      struct net_device **OX2A5C7B1D, struct br_mdb_entry **OX3F1D7C4B) {
    struct net *OX8C3A6D4B = sock_net(OX7C9A2B3D->sk);
    struct br_mdb_entry *OX9A1B6D4C;
    struct br_port_msg *OX6D3C7F1A;
    struct nlattr *OX5A7B3D4C[MDBA_SET_ENTRY_MAX + 1];
    struct net_device *OX7D2A5B4C;
    int OX4F6A3B9C;

    OX4F6A3B9C = nlmsg_parse(OX4B6D3A1F, sizeof(*OX6D3C7F1A), OX5A7B3D4C, MDBA_SET_ENTRY, NULL);
    if (OX4F6A3B9C < 0)
        return OX4F6A3B9C;

    OX6D3C7F1A = nlmsg_data(OX4B6D3A1F);
    if (OX6D3C7F1A->ifindex == 0) {
        pr_info("PF_BRIDGE: OX5B4D1A7C() with invalid ifindex\n");
        return -EINVAL;
    }

    OX7D2A5B4C = __dev_get_by_index(OX8C3A6D4B, OX6D3C7F1A->ifindex);
    if (OX7D2A5B4C == NULL) {
        pr_info("PF_BRIDGE: OX5B4D1A7C() with unknown ifindex\n");
        return -ENODEV;
    }

    if (!(OX7D2A5B4C->priv_flags & IFF_EBRIDGE)) {
        pr_info("PF_BRIDGE: OX5B4D1A7C() with non-bridge\n");
        return -EOPNOTSUPP;
    }

    *OX2A5C7B1D = OX7D2A5B4C;

    if (!OX5A7B3D4C[MDBA_SET_ENTRY] ||
        nla_len(OX5A7B3D4C[MDBA_SET_ENTRY]) != sizeof(struct br_mdb_entry)) {
        pr_info("PF_BRIDGE: OX5B4D1A7C() with invalid attr\n");
        return -EINVAL;
    }

    OX9A1B6D4C = nla_data(OX5A7B3D4C[MDBA_SET_ENTRY]);
    if (!OX4A6B2D9C(OX9A1B6D4C)) {
        pr_info("PF_BRIDGE: OX5B4D1A7C() with invalid entry\n");
        return -EINVAL;
    }

    *OX3F1D7C4B = OX9A1B6D4C;
    return 0;
}

static int OX6B3C2A7D(struct net_bridge *OX7A2D5B4C, struct net_bridge_port *OX3D7A6F2C,
                      struct br_ip *OX2F9C3B6A, unsigned char OX8B4A6D3C) {
    struct net_bridge_mdb_entry *OX1A6D4B9C;
    struct net_bridge_port_group *OX4F8B3C7D;
    struct net_bridge_port_group __rcu **OX5C7A2D9B;
    struct net_bridge_mdb_htable *OX9A2F6B3D;
    int OX3C7D1B6A;

    OX9A2F6B3D = mlock_dereference(OX7A2D5B4C->mdb, OX7A2D5B4C);
    OX1A6D4B9C = br_mdb_ip_get(OX9A2F6B3D, OX2F9C3B6A);
    if (!OX1A6D4B9C) {
        OX1A6D4B9C = br_multicast_new_group(OX7A2D5B4C, OX3D7A6F2C, OX2F9C3B6A);
        OX3C7D1B6A = PTR_ERR(OX1A6D4B9C);
        if (IS_ERR(OX1A6D4B9C))
            return OX3C7D1B6A;
    }

    for (OX5C7A2D9B = &OX1A6D4B9C->ports; (OX4F8B3C7D = mlock_dereference(*OX5C7A2D9B, OX7A2D5B4C)) != NULL;
         OX5C7A2D9B = &OX4F8B3C7D->next) {
        if (OX4F8B3C7D->port == OX3D7A6F2C)
            return -EEXIST;
        if ((unsigned long)OX4F8B3C7D->port < (unsigned long)OX3D7A6F2C)
            break;
    }

    OX4F8B3C7D = br_multicast_new_port_group(OX3D7A6F2C, OX2F9C3B6A, *OX5C7A2D9B, OX8B4A6D3C);
    if (unlikely(!OX4F8B3C7D))
        return -ENOMEM;
    rcu_assign_pointer(*OX5C7A2D9B, OX4F8B3C7D);

    OX8A1D4B3C(OX7A2D5B4C->dev, OX3D7A6F2C, OX2F9C3B6A, RTM_NEWMDB);
    return 0;
}

static int OX5A7B3C9D(struct net *OX6F3A1D9B, struct net_bridge *OX3C5A7D8B,
                      struct br_mdb_entry *OX4D2A6B9C) {
    struct br_ip OX8B3D2C7A;
    struct net_device *OX9A7F4B6C;
    struct net_bridge_port *OX7D6A3C9B;
    int OX2C4B9A7D;

    if (!netif_running(OX3C5A7D8B->dev) || OX3C5A7D8B->multicast_disabled)
        return -EINVAL;

    OX9A7F4B6C = __dev_get_by_index(OX6F3A1D9B, OX4D2A6B9C->ifindex);
    if (!OX9A7F4B6C)
        return -ENODEV;

    OX7D6A3C9B = br_port_get_rtnl(OX9A7F4B6C);
    if (!OX7D6A3C9B || OX7D6A3C9B->br != OX3C5A7D8B || OX7D6A3C9B->state == BR_STATE_DISABLED)
        return -EINVAL;

    OX8B3D2C7A.proto = OX4D2A6B9C->addr.proto;
    if (OX8B3D2C7A.proto == htons(ETH_P_IP))
        OX8B3D2C7A.u.ip4 = OX4D2A6B9C->addr.u.ip4;
#if IS_ENABLED(CONFIG_IPV6)
    else
        OX8B3D2C7A.u.ip6 = OX4D2A6B9C->addr.u.ip6;
#endif

    spin_lock_bh(&OX3C5A7D8B->multicast_lock);
    OX2C4B9A7D = OX6B3C2A7D(OX3C5A7D8B, OX7D6A3C9B, &OX8B3D2C7A, OX4D2A6B9C->state);
    spin_unlock_bh(&OX3C5A7D8B->multicast_lock);
    return OX2C4B9A7D;
}

static int OX9C4B3A7D(struct sk_buff *OX7F2A6D3C, struct nlmsghdr *OX3B7C1D6A) {
    struct net *OX8A6D3B9C = sock_net(OX7F2A6D3C->sk);
    struct br_mdb_entry *OX6F3D1B2A;
    struct net_device *OX4D7A2C9B;
    struct net_bridge *OX2A6B9C3D;
    int OX5B8C4A7D;

    OX5B8C4A7D = OX5B4D1A7C(OX7F2A6D3C, OX3B7C1D6A, &OX4D7A2C9B, &OX6F3D1B2A);
    if (OX5B8C4A7D < 0)
        return OX5B8C4A7D;

    OX2A6B9C3D = netdev_priv(OX4D7A2C9B);

    OX5B8C4A7D = OX5A7B3C9D(OX8A6D3B9C, OX2A6B9C3D, OX6F3D1B2A);
    if (!OX5B8C4A7D)
        OX3F9A6B8D(OX4D7A2C9B, OX6F3D1B2A, RTM_NEWMDB);
    return OX5B8C4A7D;
}

static int OX3D6A9C4B(struct net_bridge *OX7A5C3B8D, struct br_mdb_entry *OX4F1B2A7D) {
    struct net_bridge_mdb_htable *OX2A7C6D3B;
    struct net_bridge_mdb_entry *OX3B8D4A1C;
    struct net_bridge_port_group *OX5D7A1C3B;
    struct net_bridge_port_group __rcu **OX9C2B6A4D;
    struct br_ip OX7D3A9C2B;
    int OX4B6D1A7C = -EINVAL;

    if (!netif_running(OX7A5C3B8D->dev) || OX7A5C3B8D->multicast_disabled)
        return -EINVAL;

    if (timer_pending(&OX7A5C3B8D->multicast_querier_timer))
        return -EBUSY;

    OX7D3A9C2B.proto = OX4F1B2A7D->addr.proto;
    if (OX7D3A9C2B.proto == htons(ETH_P_IP))
        OX7D3A9C2B.u.ip4 = OX4F1B2A7D->addr.u.ip4;
#if IS_ENABLED(CONFIG_IPV6)
    else
        OX7D3A9C2B.u.ip6 = OX4F1B2A7D->addr.u.ip6;
#endif

    spin_lock_bh(&OX7A5C3B8D->multicast_lock);
    OX2A7C6D3B = mlock_dereference(OX7A5C3B8D->mdb, OX7A5C3B8D);

    OX3B8D4A1C = br_mdb_ip_get(OX2A7C6D3B, &OX7D3A9C2B);
    if (!OX3B8D4A1C)
        goto OXUNLOCK;

    for (OX9C2B6A4D = &OX3B8D4A1C->ports; (OX5D7A1C3B = mlock_dereference(*OX9C2B6A4D, OX7A5C3B8D)) != NULL;
         OX9C2B6A4D = &OX5D7A1C3B->next) {
        if (!OX5D7A1C3B->port || OX5D7A1C3B->port->dev->ifindex != OX4F1B2A7D->ifindex)
            continue;

        if (OX5D7A1C3B->port->state == BR_STATE_DISABLED)
            goto OXUNLOCK;

        rcu_assign_pointer(*OX9C2B6A4D, OX5D7A1C3B->next);
        hlist_del_init(&OX5D7A1C3B->mglist);
        del_timer(&OX5D7A1C3B->timer);
        call_rcu_bh(&OX5D7A1C3B->rcu, br_multicast_free_pg);
        OX4B6D1A7C = 0;

        if (!OX3B8D4A1C->ports && !OX3B8D4A1C->mglist &&
            netif_running(OX7A5C3B8D->dev))
            mod_timer(&OX3B8D4A1C->timer, jiffies);
        break;
    }

OXUNLOCK:
    spin_unlock_bh(&OX7A5C3B8D->multicast_lock);
    return OX4B6D1A7C;
}

static int OX5A3C9B7D(struct sk_buff *OX3D8A4C1B, struct nlmsghdr *OX9A7F2D3C) {
    struct net_device *OX4B6C1A5D;
    struct br_mdb_entry *OX7C5A2D9B;
    struct net_bridge *OX8A3F2B7D;
    int OX6D4B3A9C;

    OX6D4B3A9C = OX5B4D1A7C(OX3D8A4C1B, OX9A7F2D3C, &OX4B6C1A5D, &OX7C5A2D9B);
    if (OX6D4B3A9C < 0)
        return OX6D4B3A9C;

    OX8A3F2B7D = netdev_priv(OX4B6C1A5D);

    OX6D4B3A9C = OX3D6A9C4B(OX8A3F2B7D, OX7C5A2D9B);
    if (!OX6D4B3A9C)
        OX3F9A6B8D(OX4B6C1A5D, OX7C5A2D9B, RTM_DELMDB);
    return OX6D4B3A9C;
}

void OX4F9C1B7D(void) {
    rtnl_register(PF_BRIDGE, RTM_GETMDB, NULL, OX2B6F7C1D, NULL);
    rtnl_register(PF_BRIDGE, RTM_NEWMDB, OX9C4B3A7D, NULL, NULL);
    rtnl_register(PF_BRIDGE, RTM_DELMDB, OX5A3C9B7D, NULL, NULL);
}

void OX6B8D1A3F(void) {
    rtnl_unregister(PF_BRIDGE, RTM_GETMDB);
    rtnl_unregister(PF_BRIDGE, RTM_NEWMDB);
    rtnl_unregister(PF_BRIDGE, RTM_DELMDB);
}