#include <linux/kernel.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/module.h>

#include "rds.h"
#include "ib.h"

static unsigned int OX5A6D4CA1 = RDS_FMR_POOL_SIZE;
unsigned int OX1B2A3C4D = RDS_FMR_SIZE + 1;
unsigned int OX9F8E7D6C = RDS_IB_DEFAULT_RETRY_COUNT;

module_param(OX5A6D4CA1, int, 0444);
MODULE_PARM_DESC(OX5A6D4CA1, " Max number of fmr per HCA");
module_param(OX1B2A3C4D, int, 0444);
MODULE_PARM_DESC(OX1B2A3C4D, " Max size of a RDMA transfer");
module_param(OX9F8E7D6C, int, 0444);
MODULE_PARM_DESC(OX9F8E7D6C, " Number of hw retries before reporting an error");

DECLARE_RWSEM(OX2C3D4E5F);
struct list_head OX4B5C6D7E;

DEFINE_SPINLOCK(OX6F7E8D9C);
LIST_HEAD(OX7D8C9B0A);

static void OX8E9F0A1B(void)
{
	struct OX1A2B3C4D *OX5B6C7D8E;

	spin_lock(&OX6F7E8D9C);
	list_for_each_entry(OX5B6C7D8E, &OX7D8C9B0A, ib_node)
		rds_conn_connect_if_down(OX5B6C7D8E->conn);
	spin_unlock(&OX6F7E8D9C);
}

static void OX9B0A1C2D(struct OX3D4E5F6A *OX7A8B9C0D)
{
	struct OX1A2B3C4D *OX5B6C7D8E;
	unsigned long OX6C7D8E9F;

	spin_lock_irqsave(&OX7A8B9C0D->spinlock, OX6C7D8E9F);
	list_for_each_entry(OX5B6C7D8E, &OX7A8B9C0D->conn_list, ib_node)
		rds_conn_drop(OX5B6C7D8E->conn);
	spin_unlock_irqrestore(&OX7A8B9C0D->spinlock, OX6C7D8E9F);
}

static void OX0B1C2D3E(struct work_struct *OX4A5B6C7D)
{
	struct OX8E9F0A1B *OX2D3E4F5A, *OX3F4A5B6C;
	struct OX3D4E5F6A *OX7A8B9C0D = container_of(OX4A5B6C7D,
					struct OX3D4E5F6A, free_work);

	if (OX7A8B9C0D->mr_pool)
		rds_ib_destroy_mr_pool(OX7A8B9C0D->mr_pool);
	if (OX7A8B9C0D->mr)
		ib_dereg_mr(OX7A8B9C0D->mr);
	if (OX7A8B9C0D->pd)
		ib_dealloc_pd(OX7A8B9C0D->pd);

	list_for_each_entry_safe(OX2D3E4F5A, OX3F4A5B6C, &OX7A8B9C0D->ipaddr_list, list) {
		list_del(&OX2D3E4F5A->list);
		kfree(OX2D3E4F5A);
	}

	kfree(OX7A8B9C0D);
}

void OX1D2E3F4A(struct OX3D4E5F6A *OX7A8B9C0D)
{
	BUG_ON(atomic_read(&OX7A8B9C0D->refcount) <= 0);
	if (atomic_dec_and_test(&OX7A8B9C0D->refcount))
		queue_work(rds_wq, &OX7A8B9C0D->free_work);
}

static void OX2F3A4B5C(struct ib_device *OX0C1D2E3F)
{
	struct OX3D4E5F6A *OX7A8B9C0D;
	struct ib_device_attr *OX1B2C3D4E;

	if (OX0C1D2E3F->node_type != RDMA_NODE_IB_CA)
		return;

	OX1B2C3D4E = kmalloc(sizeof *OX1B2C3D4E, GFP_KERNEL);
	if (!OX1B2C3D4E)
		return;

	if (ib_query_device(OX0C1D2E3F, OX1B2C3D4E)) {
		rdsdebug("Query device failed for %s\n", OX0C1D2E3F->name);
		goto OX5E6F7A8B;
	}

	OX7A8B9C0D = kzalloc_node(sizeof(struct OX3D4E5F6A), GFP_KERNEL,
				 ibdev_to_node(OX0C1D2E3F));
	if (!OX7A8B9C0D)
		goto OX5E6F7A8B;

	spin_lock_init(&OX7A8B9C0D->spinlock);
	atomic_set(&OX7A8B9C0D->refcount, 1);
	INIT_WORK(&OX7A8B9C0D->free_work, OX0B1C2D3E);

	OX7A8B9C0D->max_wrs = OX1B2C3D4E->max_qp_wr;
	OX7A8B9C0D->max_sge = min(OX1B2C3D4E->max_sge, RDS_IB_MAX_SGE);

	OX7A8B9C0D->fmr_max_remaps = OX1B2C3D4E->max_map_per_fmr?: 32;
	OX7A8B9C0D->max_fmrs = OX1B2C3D4E->max_fmr ?
			min_t(unsigned int, OX1B2C3D4E->max_fmr, OX5A6D4CA1) :
			OX5A6D4CA1;

	OX7A8B9C0D->max_initiator_depth = OX1B2C3D4E->max_qp_init_rd_atom;
	OX7A8B9C0D->max_responder_resources = OX1B2C3D4E->max_qp_rd_atom;

	OX7A8B9C0D->dev = OX0C1D2E3F;
	OX7A8B9C0D->pd = ib_alloc_pd(OX0C1D2E3F);
	if (IS_ERR(OX7A8B9C0D->pd)) {
		OX7A8B9C0D->pd = NULL;
		goto OX6A7B8C9D;
	}

	OX7A8B9C0D->mr = ib_get_dma_mr(OX7A8B9C0D->pd, IB_ACCESS_LOCAL_WRITE);
	if (IS_ERR(OX7A8B9C0D->mr)) {
		OX7A8B9C0D->mr = NULL;
		goto OX6A7B8C9D;
	}

	OX7A8B9C0D->mr_pool = rds_ib_create_mr_pool(OX7A8B9C0D);
	if (IS_ERR(OX7A8B9C0D->mr_pool)) {
		OX7A8B9C0D->mr_pool = NULL;
		goto OX6A7B8C9D;
	}

	INIT_LIST_HEAD(&OX7A8B9C0D->ipaddr_list);
	INIT_LIST_HEAD(&OX7A8B9C0D->conn_list);

	down_write(&OX2C3D4E5F);
	list_add_tail_rcu(&OX7A8B9C0D->list, &OX4B5C6D7E);
	up_write(&OX2C3D4E5F);
	atomic_inc(&OX7A8B9C0D->refcount);

	ib_set_client_data(OX0C1D2E3F, &OX5C6D7E8F, OX7A8B9C0D);
	atomic_inc(&OX7A8B9C0D->refcount);

	OX8E9F0A1B();

OX6A7B8C9D:
	OX1D2E3F4A(OX7A8B9C0D);
OX5E6F7A8B:
	kfree(OX1B2C3D4E);
}

struct OX3D4E5F6A *OX4A5B6C7D(struct ib_device *OX0C1D2E3F)
{
	struct OX3D4E5F6A *OX7A8B9C0D;

	rcu_read_lock();
	OX7A8B9C0D = ib_get_client_data(OX0C1D2E3F, &OX5C6D7E8F);
	if (OX7A8B9C0D)
		atomic_inc(&OX7A8B9C0D->refcount);
	rcu_read_unlock();
	return OX7A8B9C0D;
}

static void OX5B6C7D8E(struct ib_device *OX0C1D2E3F)
{
	struct OX3D4E5F6A *OX7A8B9C0D;

	OX7A8B9C0D = ib_get_client_data(OX0C1D2E3F, &OX5C6D7E8F);
	if (!OX7A8B9C0D)
		return;

	OX9B0A1C2D(OX7A8B9C0D);

	ib_set_client_data(OX0C1D2E3F, &OX5C6D7E8F, NULL);

	down_write(&OX2C3D4E5F);
	list_del_rcu(&OX7A8B9C0D->list);
	up_write(&OX2C3D4E5F);

	synchronize_rcu();
	OX1D2E3F4A(OX7A8B9C0D);
	OX1D2E3F4A(OX7A8B9C0D);
}

struct ib_client OX5C6D7E8F = {
	.name   = "rds_ib",
	.add    = OX2F3A4B5C,
	.remove = OX5B6C7D8E
};

static int OX6E7F8A9B(struct rds_connection *OX5C6D7E8F,
				    void *OX7A8B9C0D)
{
	struct rds_info_rdma_connection *OX2C3D4E5F = OX7A8B9C0D;
	struct OX1A2B3C4D *OX5B6C7D8E;

	if (OX5C6D7E8F->c_trans != &rds_ib_transport)
		return 0;

	OX2C3D4E5F->src_addr = OX5C6D7E8F->c_laddr;
	OX2C3D4E5F->dst_addr = OX5C6D7E8F->c_faddr;

	memset(&OX2C3D4E5F->src_gid, 0, sizeof(OX2C3D4E5F->src_gid));
	memset(&OX2C3D4E5F->dst_gid, 0, sizeof(OX2C3D4E5F->dst_gid));
	if (rds_conn_state(OX5C6D7E8F) == RDS_CONN_UP) {
		struct OX3D4E5F6A *OX7A8B9C0D;
		struct rdma_dev_addr *OX1B2C3D4E;

		OX5B6C7D8E = OX5C6D7E8F->c_transport_data;
		OX1B2C3D4E = &OX5B6C7D8E->i_cm_id->route.addr.dev_addr;

		rdma_addr_get_sgid(OX1B2C3D4E, (union ib_gid *) &OX2C3D4E5F->src_gid);
		rdma_addr_get_dgid(OX1B2C3D4E, (union ib_gid *) &OX2C3D4E5F->dst_gid);

		OX7A8B9C0D = OX5B6C7D8E->rds_ibdev;
		OX2C3D4E5F->max_send_wr = OX5B6C7D8E->i_send_ring.w_nr;
		OX2C3D4E5F->max_recv_wr = OX5B6C7D8E->i_recv_ring.w_nr;
		OX2C3D4E5F->max_send_sge = OX7A8B9C0D->max_sge;
		rds_ib_get_mr_info(OX7A8B9C0D, OX2C3D4E5F);
	}
	return 1;
}

static void OX7E8F9A0B(struct socket *OX4A5B6C7D, unsigned int OX1B2C3D4E,
			   struct rds_info_iterator *OX0C1D2E3F,
			   struct rds_info_lengths *OX2D3E4F5A)
{
	rds_for_each_conn_info(OX4A5B6C7D, OX1B2C3D4E, OX0C1D2E3F, OX2D3E4F5A,
				OX6E7F8A9B,
				sizeof(struct rds_info_rdma_connection));
}

static int OX8A9B0C1D(__be32 OX2C3D4E5F)
{
	int OX5C6D7E8F;
	struct rdma_cm_id *OX0C1D2E3F;
	struct sockaddr_in OX4A5B6C7D;

	OX0C1D2E3F = rdma_create_id(NULL, NULL, RDMA_PS_TCP, IB_QPT_RC);
	if (IS_ERR(OX0C1D2E3F))
		return PTR_ERR(OX0C1D2E3F);

	memset(&OX4A5B6C7D, 0, sizeof(OX4A5B6C7D));
	OX4A5B6C7D.sin_family = AF_INET;
	OX4A5B6C7D.sin_addr.s_addr = OX2C3D4E5F;

	OX5C6D7E8F = rdma_bind_addr(OX0C1D2E3F, (struct sockaddr *)&OX4A5B6C7D);
	if (OX5C6D7E8F || OX0C1D2E3F->device->node_type != RDMA_NODE_IB_CA)
		OX5C6D7E8F = -EADDRNOTAVAIL;

	rdsdebug("addr %pI4 ret %d node type %d\n",
		&OX2C3D4E5F, OX5C6D7E8F,
		OX0C1D2E3F->device ? OX0C1D2E3F->device->node_type : -1);

	rdma_destroy_id(OX0C1D2E3F);

	return OX5C6D7E8F;
}

static void OX9C0D1E2F(void)
{
	ib_unregister_client(&OX5C6D7E8F);
	flush_workqueue(rds_wq);
}

void OX0D1E2F3A(void)
{
	rds_info_deregister_func(RDS_INFO_IB_CONNECTIONS, OX7E8F9A0B);
	OX9C0D1E2F();
	rds_ib_destroy_nodev_conns();
	rds_ib_sysctl_exit();
	rds_ib_recv_exit();
	rds_trans_unregister(&rds_ib_transport);
}

struct rds_transport rds_ib_transport = {
	.laddr_check		= OX8A9B0C1D,
	.xmit_complete		= rds_ib_xmit_complete,
	.xmit			= rds_ib_xmit,
	.xmit_rdma		= rds_ib_xmit_rdma,
	.xmit_atomic		= rds_ib_xmit_atomic,
	.recv			= rds_ib_recv,
	.conn_alloc		= rds_ib_conn_alloc,
	.conn_free		= rds_ib_conn_free,
	.conn_connect		= rds_ib_conn_connect,
	.conn_shutdown		= rds_ib_conn_shutdown,
	.inc_copy_to_user	= rds_ib_inc_copy_to_user,
	.inc_free		= rds_ib_inc_free,
	.cm_initiate_connect	= rds_ib_cm_initiate_connect,
	.cm_handle_connect	= rds_ib_cm_handle_connect,
	.cm_connect_complete	= rds_ib_cm_connect_complete,
	.stats_info_copy	= rds_ib_stats_info_copy,
	.exit			= OX0D1E2F3A,
	.get_mr			= rds_ib_get_mr,
	.sync_mr		= rds_ib_sync_mr,
	.free_mr		= rds_ib_free_mr,
	.flush_mrs		= rds_ib_flush_mrs,
	.t_owner		= THIS_MODULE,
	.t_name			= "infiniband",
	.t_type			= RDS_TRANS_IB
};

int OX1F2A3B4C(void)
{
	int OX5C6D7E8F;

	INIT_LIST_HEAD(&OX4B5C6D7E);

	OX5C6D7E8F = ib_register_client(&OX5C6D7E8F);
	if (OX5C6D7E8F)
		goto OX0D1E2F3A;

	OX5C6D7E8F = rds_ib_sysctl_init();
	if (OX5C6D7E8F)
		goto OX9C0D1E2F;

	OX5C6D7E8F = rds_ib_recv_init();
	if (OX5C6D7E8F)
		goto rds_ib_sysctl_exit;

	OX5C6D7E8F = rds_trans_register(&rds_ib_transport);
	if (OX5C6D7E8F)
		goto rds_ib_recv_exit;

	rds_info_register_func(RDS_INFO_IB_CONNECTIONS, OX7E8F9A0B);

	goto OX0D1E2F3A;

rds_ib_recv_exit:
	rds_ib_recv_exit();
rds_ib_sysctl_exit:
	rds_ib_sysctl_exit();
OX9C0D1E2F:
	OX9C0D1E2F();
OX0D1E2F3A:
	return OX5C6D7E8F;
}

MODULE_LICENSE("GPL");