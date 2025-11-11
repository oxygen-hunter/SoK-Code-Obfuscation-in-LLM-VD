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

static int br_rports_fill_info(struct sk_buff *A, struct netlink_callback *B,
			       struct net_device *C)
{
	struct net_bridge *D = netdev_priv(C);
	struct net_bridge_port *E;
	struct nlattr *F;

	if (!D->multicast_router || hlist_empty(&D->router_list))
		return 0;

	F = nla_nest_start(A, MDBA_ROUTER);
	if (F == NULL)
		return -EMSGSIZE;

	hlist_for_each_entry_rcu(E, &D->router_list, rlist) {
		if (E && nla_put_u32(A, MDBA_ROUTER_PORT, E->dev->ifindex))
			goto G;
	}

	nla_nest_end(A, F);
	return 0;
G:
	nla_nest_cancel(A, F);
	return -EMSGSIZE;
}

static int br_mdb_fill_info(struct sk_buff *H, struct netlink_callback *I,
			    struct net_device *J)
{
	struct net_bridge *K = netdev_priv(J);
	struct net_bridge_mdb_htable *L;
	struct nlattr *M, *N;
	int O = 0, P = 0, Q = I->args[1];
	int R = 0;

	if (K->multicast_disabled)
		return 0;

	L = rcu_dereference(K->mdb);
	if (!L)
		return 0;

	M = nla_nest_start(H, MDBA_MDB);
	if (M == NULL)
		return -EMSGSIZE;

	for (O = 0; O < L->max; O++) {
		struct net_bridge_mdb_entry *S;
		struct net_bridge_port_group *T, **U;
		struct net_bridge_port *V;

		hlist_for_each_entry_rcu(S, &L->mhash[O], hlist[L->ver]) {
			if (P < Q)
				goto W;

			N = nla_nest_start(H, MDBA_MDB_ENTRY);
			if (N == NULL) {
				R = -EMSGSIZE;
				goto X;
			}

			for (U = &S->ports;
			     (T = rcu_dereference(*U)) != NULL;
			      U = &T->next) {
				V = T->port;
				if (V) {
					struct br_mdb_entry Y;
					memset(&Y, 0, sizeof(Y));
					Y.ifindex = V->dev->ifindex;
					Y.state = T->state;
					if (T->addr.proto == htons(ETH_P_IP))
						Y.addr.u.ip4 = T->addr.u.ip4;
#if IS_ENABLED(CONFIG_IPV6)
					if (T->addr.proto == htons(ETH_P_IPV6))
						Y.addr.u.ip6 = T->addr.u.ip6;
#endif
					Y.addr.proto = T->addr.proto;
					if (nla_put(H, MDBA_MDB_ENTRY_INFO, sizeof(Y), &Y)) {
						nla_nest_cancel(H, N);
						R = -EMSGSIZE;
						goto X;
					}
				}
			}
			nla_nest_end(H, N);
		W:
			P++;
		}
	}

X:
	I->args[1] = P;
	nla_nest_end(H, M);
	return R;
}

static int br_mdb_dump(struct sk_buff *Z, struct netlink_callback *a)
{
	struct net_device *b;
	struct net *c = sock_net(Z->sk);
	struct nlmsghdr *d = NULL;
	int e = 0, f;

	f = a->args[0];

	rcu_read_lock();

	a->seq = c->dev_base_seq + br_mdb_rehash_seq;

	for_each_netdev_rcu(c, b) {
		if (b->priv_flags & IFF_EBRIDGE) {
			struct br_port_msg *g;

			if (e < f)
				goto h;

			d = nlmsg_put(Z, NETLINK_CB(a->skb).portid,
					a->nlh->nlmsg_seq, RTM_GETMDB,
					sizeof(*g), NLM_F_MULTI);
			if (d == NULL)
				break;

			g = nlmsg_data(d);
			memset(g, 0, sizeof(*g));
			g->ifindex = b->ifindex;
			if (br_mdb_fill_info(Z, a, b) < 0)
				goto i;
			if (br_rports_fill_info(Z, a, b) < 0)
				goto i;

			a->args[1] = 0;
			nlmsg_end(Z, d);
		h:
			e++;
		}
	}

i:
	if (d)
		nlmsg_end(Z, d);
	rcu_read_unlock();
	a->args[0] = e;
	return Z->len;
}

static int nlmsg_populate_mdb_fill(struct sk_buff *j,
				   struct net_device *k,
				   struct br_mdb_entry *l, u32 m,
				   u32 n, int o, unsigned int p)
{
	struct nlmsghdr *q;
	struct br_port_msg *r;
	struct nlattr *s, *t;

	q = nlmsg_put(j, m, n, o, sizeof(*r), NLM_F_MULTI);
	if (!q)
		return -EMSGSIZE;

	r = nlmsg_data(q);
	memset(r, 0, sizeof(*r));
	r->family  = AF_BRIDGE;
	r->ifindex = k->ifindex;
	s = nla_nest_start(j, MDBA_MDB);
	if (s == NULL)
		goto u;
	t = nla_nest_start(j, MDBA_MDB_ENTRY);
	if (t == NULL)
		goto v;

	if (nla_put(j, MDBA_MDB_ENTRY_INFO, sizeof(*l), l))
		goto v;

	nla_nest_end(j, t);
	nla_nest_end(j, s);
	return nlmsg_end(j, q);

v:
	nla_nest_end(j, s);
u:
	nlmsg_cancel(j, q);
	return -EMSGSIZE;
}

static inline size_t rtnl_mdb_nlmsg_size(void)
{
	return NLMSG_ALIGN(sizeof(struct br_port_msg))
		+ nla_total_size(sizeof(struct br_mdb_entry));
}

static void __br_mdb_notify(struct net_device *w, struct br_mdb_entry *x,
			    int y)
{
	struct net *z = dev_net(w);
	struct sk_buff *aa;
	int ab = -ENOBUFS;

	aa = nlmsg_new(rtnl_mdb_nlmsg_size(), GFP_ATOMIC);
	if (!aa)
		goto ac;

	ab = nlmsg_populate_mdb_fill(aa, w, x, 0, 0, y, NTF_SELF);
	if (ab < 0) {
		kfree_skb(aa);
		goto ac;
	}

	rtnl_notify(aa, z, 0, RTNLGRP_MDB, NULL, GFP_ATOMIC);
	return;
ac:
	rtnl_set_sk_err(z, RTNLGRP_MDB, ab);
}

void br_mdb_notify(struct net_device *ad, struct net_bridge_port *ae,
		   struct br_ip *af, int ag)
{
	struct br_mdb_entry ah;

	memset(&ah, 0, sizeof(ah));
	ah.ifindex = ae->dev->ifindex;
	ah.addr.proto = af->proto;
	ah.addr.u.ip4 = af->u.ip4;
#if IS_ENABLED(CONFIG_IPV6)
	ah.addr.u.ip6 = af->u.ip6;
#endif
	__br_mdb_notify(ad, &ah, ag);
}

static bool is_valid_mdb_entry(struct br_mdb_entry *ai)
{
	if (ai->ifindex == 0)
		return false;

	if (ai->addr.proto == htons(ETH_P_IP)) {
		if (!ipv4_is_multicast(ai->addr.u.ip4))
			return false;
		if (ipv4_is_local_multicast(ai->addr.u.ip4))
			return false;
#if IS_ENABLED(CONFIG_IPV6)
	} else if (ai->addr.proto == htons(ETH_P_IPV6)) {
		if (!ipv6_is_transient_multicast(&ai->addr.u.ip6))
			return false;
#endif
	} else
		return false;
	if (ai->state != MDB_PERMANENT && ai->state != MDB_TEMPORARY)
		return false;

	return true;
}

static int br_mdb_parse(struct sk_buff *aj, struct nlmsghdr *ak,
			struct net_device **al, struct br_mdb_entry **am)
{
	struct net *an = sock_net(aj->sk);
	struct br_mdb_entry *ao;
	struct br_port_msg *ap;
	struct nlattr *aq[MDBA_SET_ENTRY_MAX+1];
	struct net_device *ar;
	int as;

	as = nlmsg_parse(ak, sizeof(*ap), aq, MDBA_SET_ENTRY, NULL);
	if (as < 0)
		return as;

	ap = nlmsg_data(ak);
	if (ap->ifindex == 0) {
		pr_info("PF_BRIDGE: br_mdb_parse() with invalid ifindex\n");
		return -EINVAL;
	}

	ar = __dev_get_by_index(an, ap->ifindex);
	if (ar == NULL) {
		pr_info("PF_BRIDGE: br_mdb_parse() with unknown ifindex\n");
		return -ENODEV;
	}

	if (!(ar->priv_flags & IFF_EBRIDGE)) {
		pr_info("PF_BRIDGE: br_mdb_parse() with non-bridge\n");
		return -EOPNOTSUPP;
	}

	*al = ar;

	if (!aq[MDBA_SET_ENTRY] ||
	    nla_len(aq[MDBA_SET_ENTRY]) != sizeof(struct br_mdb_entry)) {
		pr_info("PF_BRIDGE: br_mdb_parse() with invalid attr\n");
		return -EINVAL;
	}

	ao = nla_data(aq[MDBA_SET_ENTRY]);
	if (!is_valid_mdb_entry(ao)) {
		pr_info("PF_BRIDGE: br_mdb_parse() with invalid entry\n");
		return -EINVAL;
	}

	*am = ao;
	return 0;
}

static int br_mdb_add_group(struct net_bridge *at, struct net_bridge_port *au,
			    struct br_ip *av, unsigned char aw)
{
	struct net_bridge_mdb_entry *ax;
	struct net_bridge_port_group *ay;
	struct net_bridge_port_group __rcu **az;
	struct net_bridge_mdb_htable *ba;
	int bb;

	ba = mlock_dereference(at->mdb, at);
	ax = br_mdb_ip_get(ba, av);
	if (!ax) {
		ax = br_multicast_new_group(at, au, av);
		bb = PTR_ERR(ax);
		if (IS_ERR(ax))
			return bb;
	}

	for (az = &ax->ports;
	     (ay = mlock_dereference(*az, at)) != NULL;
	     az = &ay->next) {
		if (ay->port == au)
			return -EEXIST;
		if ((unsigned long)ay->port < (unsigned long)au)
			break;
	}

	ay = br_multicast_new_port_group(au, av, *az, aw);
	if (unlikely(!ay))
		return -ENOMEM;
	rcu_assign_pointer(*az, ay);

	br_mdb_notify(at->dev, au, av, RTM_NEWMDB);
	return 0;
}

static int __br_mdb_add(struct net *bc, struct net_bridge *bd,
			struct br_mdb_entry *be)
{
	struct br_ip bf;
	struct net_device *bg;
	struct net_bridge_port *bh;
	int bi;

	if (!netif_running(bd->dev) || bd->multicast_disabled)
		return -EINVAL;

	bg = __dev_get_by_index(bc, be->ifindex);
	if (!bg)
		return -ENODEV;

	bh = br_port_get_rtnl(bg);
	if (!bh || bh->br != bd || bh->state == BR_STATE_DISABLED)
		return -EINVAL;

	bf.proto = be->addr.proto;
	if (bf.proto == htons(ETH_P_IP))
		bf.u.ip4 = be->addr.u.ip4;
#if IS_ENABLED(CONFIG_IPV6)
	else
		bf.u.ip6 = be->addr.u.ip6;
#endif

	spin_lock_bh(&bd->multicast_lock);
	bi = br_mdb_add_group(bd, bh, &bf, be->state);
	spin_unlock_bh(&bd->multicast_lock);
	return bi;
}

static int br_mdb_add(struct sk_buff *bj, struct nlmsghdr *bk)
{
	struct net *bl = sock_net(bj->sk);
	struct br_mdb_entry *bm;
	struct net_device *bn;
	struct net_bridge *bo;
	int bp;

	bp = br_mdb_parse(bj, bk, &bn, &bm);
	if (bp < 0)
		return bp;

	bo = netdev_priv(bn);

	bp = __br_mdb_add(bl, bo, bm);
	if (!bp)
		__br_mdb_notify(bn, bm, RTM_NEWMDB);
	return bp;
}

static int __br_mdb_del(struct net_bridge *bq, struct br_mdb_entry *br)
{
	struct net_bridge_mdb_htable *bs;
	struct net_bridge_mdb_entry *bt;
	struct net_bridge_port_group *bu;
	struct net_bridge_port_group __rcu **bv;
	struct br_ip bw;
	int bx = -EINVAL;

	if (!netif_running(bq->dev) || bq->multicast_disabled)
		return -EINVAL;

	if (timer_pending(&bq->multicast_querier_timer))
		return -EBUSY;

	bw.proto = br->addr.proto;
	if (bw.proto == htons(ETH_P_IP))
		bw.u.ip4 = br->addr.u.ip4;
#if IS_ENABLED(CONFIG_IPV6)
	else
		bw.u.ip6 = br->addr.u.ip6;
#endif

	spin_lock_bh(&bq->multicast_lock);
	bs = mlock_dereference(bq->mdb, bq);

	bt = br_mdb_ip_get(bs, &bw);
	if (!bt)
		goto by;

	for (bv = &bt->ports;
	     (bu = mlock_dereference(*bv, bq)) != NULL;
	     bv = &bu->next) {
		if (!bu->port || bu->port->dev->ifindex != br->ifindex)
			continue;

		if (bu->port->state == BR_STATE_DISABLED)
			goto by;

		rcu_assign_pointer(*bv, bu->next);
		hlist_del_init(&bu->mglist);
		del_timer(&bu->timer);
		call_rcu_bh(&bu->rcu, br_multicast_free_pg);
		bx = 0;

		if (!bt->ports && !bt->mglist &&
		    netif_running(bq->dev))
			mod_timer(&bt->timer, jiffies);
		break;
	}

by:
	spin_unlock_bh(&bq->multicast_lock);
	return bx;
}

static int br_mdb_del(struct sk_buff *bz, struct nlmsghdr *ca)
{
	struct net_device *cb;
	struct br_mdb_entry *cc;
	struct net_bridge *cd;
	int ce;

	ce = br_mdb_parse(bz, ca, &cb, &cc);
	if (ce < 0)
		return ce;

	cd = netdev_priv(cb);

	ce = __br_mdb_del(cd, cc);
	if (!ce)
		__br_mdb_notify(cb, cc, RTM_DELMDB);
	return ce;
}

void br_mdb_init(void)
{
	rtnl_register(PF_BRIDGE, RTM_GETMDB, NULL, br_mdb_dump, NULL);
	rtnl_register(PF_BRIDGE, RTM_NEWMDB, br_mdb_add, NULL, NULL);
	rtnl_register(PF_BRIDGE, RTM_DELMDB, br_mdb_del, NULL, NULL);
}

void br_mdb_uninit(void)
{
	rtnl_unregister(PF_BRIDGE, RTM_GETMDB);
	rtnl_unregister(PF_BRIDGE, RTM_NEWMDB);
	rtnl_unregister(PF_BRIDGE, RTM_DELMDB);
}