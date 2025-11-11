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

static unsigned int fmr_pool_size = RDS_FMR_POOL_SIZE;
unsigned int fmr_message_size = RDS_FMR_SIZE + 1; /* +1 allows for unaligned MRs */
unsigned int rds_ib_retry_count = RDS_IB_DEFAULT_RETRY_COUNT;

module_param(fmr_pool_size, int, 0444);
MODULE_PARM_DESC(fmr_pool_size, " Max number of fmr per HCA");
module_param(fmr_message_size, int, 0444);
MODULE_PARM_DESC(fmr_message_size, " Max size of a RDMA transfer");
module_param(rds_ib_retry_count, int, 0444);
MODULE_PARM_DESC(rds_ib_retry_count, " Number of hw retries before reporting an error");

DECLARE_RWSEM(rds_ib_devices_lock);
struct list_head rds_ib_devices;

DEFINE_SPINLOCK(ib_nodev_conns_lock);
LIST_HEAD(ib_nodev_conns);

static void rds_ib_nodev_connect(void)
{
	struct rds_ib_connection *ic;
	int state = 0;

	while (state != 2) {
		switch (state) {
			case 0:
				spin_lock(&ib_nodev_conns_lock);
				state = 1;
				break;
			case 1:
				list_for_each_entry(ic, &ib_nodev_conns, ib_node)
					rds_conn_connect_if_down(ic->conn);
				spin_unlock(&ib_nodev_conns_lock);
				state = 2;
				break;
		}
	}
}

static void rds_ib_dev_shutdown(struct rds_ib_device *rds_ibdev)
{
	struct rds_ib_connection *ic;
	unsigned long flags;
	int state = 0;

	while (state != 2) {
		switch (state) {
			case 0:
				spin_lock_irqsave(&rds_ibdev->spinlock, flags);
				state = 1;
				break;
			case 1:
				list_for_each_entry(ic, &rds_ibdev->conn_list, ib_node)
					rds_conn_drop(ic->conn);
				spin_unlock_irqrestore(&rds_ibdev->spinlock, flags);
				state = 2;
				break;
		}
	}
}

static void rds_ib_dev_free(struct work_struct *work)
{
	struct rds_ib_ipaddr *i_ipaddr, *i_next;
	struct rds_ib_device *rds_ibdev = container_of(work, struct rds_ib_device, free_work);
	int state = 0;

	while (state != 5) {
		switch (state) {
			case 0:
				if (rds_ibdev->mr_pool) {
					rds_ib_destroy_mr_pool(rds_ibdev->mr_pool);
				}
				state = 1;
				break;
			case 1:
				if (rds_ibdev->mr) {
					ib_dereg_mr(rds_ibdev->mr);
				}
				state = 2;
				break;
			case 2:
				if (rds_ibdev->pd) {
					ib_dealloc_pd(rds_ibdev->pd);
				}
				state = 3;
				break;
			case 3:
				list_for_each_entry_safe(i_ipaddr, i_next, &rds_ibdev->ipaddr_list, list) {
					list_del(&i_ipaddr->list);
					kfree(i_ipaddr);
				}
				state = 4;
				break;
			case 4:
				kfree(rds_ibdev);
				state = 5;
				break;
		}
	}
}

void rds_ib_dev_put(struct rds_ib_device *rds_ibdev)
{
	int state = 0;

	while (state != 2) {
		switch (state) {
			case 0:
				BUG_ON(atomic_read(&rds_ibdev->refcount) <= 0);
				state = 1;
				break;
			case 1:
				if (atomic_dec_and_test(&rds_ibdev->refcount))
					queue_work(rds_wq, &rds_ibdev->free_work);
				state = 2;
				break;
		}
	}
}

static void rds_ib_add_one(struct ib_device *device)
{
	struct rds_ib_device *rds_ibdev;
	struct ib_device_attr *dev_attr;
	int state = 0;

	while (state != 9) {
		switch (state) {
			case 0:
				if (device->node_type != RDMA_NODE_IB_CA) {
					state = 9;
				} else {
					state = 1;
				}
				break;
			case 1:
				dev_attr = kmalloc(sizeof *dev_attr, GFP_KERNEL);
				if (!dev_attr) {
					state = 9;
				} else {
					state = 2;
				}
				break;
			case 2:
				if (ib_query_device(device, dev_attr)) {
					rdsdebug("Query device failed for %s\n", device->name);
					state = 8;
				} else {
					state = 3;
				}
				break;
			case 3:
				rds_ibdev = kzalloc_node(sizeof(struct rds_ib_device), GFP_KERNEL, ibdev_to_node(device));
				if (!rds_ibdev) {
					state = 8;
				} else {
					state = 4;
				}
				break;
			case 4:
				spin_lock_init(&rds_ibdev->spinlock);
				atomic_set(&rds_ibdev->refcount, 1);
				INIT_WORK(&rds_ibdev->free_work, rds_ib_dev_free);
				rds_ibdev->max_wrs = dev_attr->max_qp_wr;
				rds_ibdev->max_sge = min(dev_attr->max_sge, RDS_IB_MAX_SGE);
				rds_ibdev->fmr_max_remaps = dev_attr->max_map_per_fmr ?: 32;
				rds_ibdev->max_fmrs = dev_attr->max_fmr ? min_t(unsigned int, dev_attr->max_fmr, fmr_pool_size) : fmr_pool_size;
				rds_ibdev->max_initiator_depth = dev_attr->max_qp_init_rd_atom;
				rds_ibdev->max_responder_resources = dev_attr->max_qp_rd_atom;
				rds_ibdev->dev = device;
				rds_ibdev->pd = ib_alloc_pd(device);
				if (IS_ERR(rds_ibdev->pd)) {
					rds_ibdev->pd = NULL;
					state = 7;
				} else {
					state = 5;
				}
				break;
			case 5:
				rds_ibdev->mr = ib_get_dma_mr(rds_ibdev->pd, IB_ACCESS_LOCAL_WRITE);
				if (IS_ERR(rds_ibdev->mr)) {
					rds_ibdev->mr = NULL;
					state = 7;
				} else {
					state = 6;
				}
				break;
			case 6:
				rds_ibdev->mr_pool = rds_ib_create_mr_pool(rds_ibdev);
				if (IS_ERR(rds_ibdev->mr_pool)) {
					rds_ibdev->mr_pool = NULL;
					state = 7;
				} else {
					INIT_LIST_HEAD(&rds_ibdev->ipaddr_list);
					INIT_LIST_HEAD(&rds_ibdev->conn_list);
					down_write(&rds_ib_devices_lock);
					list_add_tail_rcu(&rds_ibdev->list, &rds_ib_devices);
					up_write(&rds_ib_devices_lock);
					atomic_inc(&rds_ibdev->refcount);
					ib_set_client_data(device, &rds_ib_client, rds_ibdev);
					atomic_inc(&rds_ibdev->refcount);
					rds_ib_nodev_connect();
					state = 7;
				}
				break;
			case 7:
				rds_ib_dev_put(rds_ibdev);
				state = 8;
				break;
			case 8:
				kfree(dev_attr);
				state = 9;
				break;
		}
	}
}

struct rds_ib_device *rds_ib_get_client_data(struct ib_device *device)
{
	struct rds_ib_device *rds_ibdev;
	int state = 0;

	while (state != 3) {
		switch (state) {
			case 0:
				rcu_read_lock();
				state = 1;
				break;
			case 1:
				rds_ibdev = ib_get_client_data(device, &rds_ib_client);
				if (rds_ibdev) {
					atomic_inc(&rds_ibdev->refcount);
				}
				state = 2;
				break;
			case 2:
				rcu_read_unlock();
				state = 3;
				break;
		}
	}
	return rds_ibdev;
}

static void rds_ib_remove_one(struct ib_device *device)
{
	struct rds_ib_device *rds_ibdev;
	int state = 0;

	while (state != 6) {
		switch (state) {
			case 0:
				rds_ibdev = ib_get_client_data(device, &rds_ib_client);
				if (!rds_ibdev) {
					state = 6;
				} else {
					state = 1;
				}
				break;
			case 1:
				rds_ib_dev_shutdown(rds_ibdev);
				ib_set_client_data(device, &rds_ib_client, NULL);
				state = 2;
				break;
			case 2:
				down_write(&rds_ib_devices_lock);
				list_del_rcu(&rds_ibdev->list);
				up_write(&rds_ib_devices_lock);
				state = 3;
				break;
			case 3:
				synchronize_rcu();
				state = 4;
				break;
			case 4:
				rds_ib_dev_put(rds_ibdev);
				state = 5;
				break;
			case 5:
				rds_ib_dev_put(rds_ibdev);
				state = 6;
				break;
		}
	}
}

struct ib_client rds_ib_client = {
	.name   = "rds_ib",
	.add    = rds_ib_add_one,
	.remove = rds_ib_remove_one
};

static int rds_ib_conn_info_visitor(struct rds_connection *conn, void *buffer)
{
	struct rds_info_rdma_connection *iinfo = buffer;
	struct rds_ib_connection *ic;
	int state = 0;

	while (state != 5) {
		switch (state) {
			case 0:
				if (conn->c_trans != &rds_ib_transport) {
					state = 5;
				} else {
					state = 1;
				}
				break;
			case 1:
				iinfo->src_addr = conn->c_laddr;
				iinfo->dst_addr = conn->c_faddr;
				memset(&iinfo->src_gid, 0, sizeof(iinfo->src_gid));
				memset(&iinfo->dst_gid, 0, sizeof(iinfo->dst_gid));
				if (rds_conn_state(conn) == RDS_CONN_UP) {
					state = 2;
				} else {
					state = 5;
				}
				break;
			case 2:
				{
					struct rds_ib_device *rds_ibdev;
					struct rdma_dev_addr *dev_addr;
					ic = conn->c_transport_data;
					dev_addr = &ic->i_cm_id->route.addr.dev_addr;
					rdma_addr_get_sgid(dev_addr, (union ib_gid *) &iinfo->src_gid);
					rdma_addr_get_dgid(dev_addr, (union ib_gid *) &iinfo->dst_gid);
					rds_ibdev = ic->rds_ibdev;
					iinfo->max_send_wr = ic->i_send_ring.w_nr;
					iinfo->max_recv_wr = ic->i_recv_ring.w_nr;
					iinfo->max_send_sge = rds_ibdev->max_sge;
					state = 3;
				}
				break;
			case 3:
				rds_ib_get_mr_info(rds_ibdev, iinfo);
				state = 4;
				break;
			case 4:
				state = 5;
				break;
		}
	}
	return 1;
}

static void rds_ib_ic_info(struct socket *sock, unsigned int len, struct rds_info_iterator *iter, struct rds_info_lengths *lens)
{
	rds_for_each_conn_info(sock, len, iter, lens, rds_ib_conn_info_visitor, sizeof(struct rds_info_rdma_connection));
}

static int rds_ib_laddr_check(__be32 addr)
{
	int ret;
	struct rdma_cm_id *cm_id;
	struct sockaddr_in sin;
	int state = 0;

	while (state != 3) {
		switch (state) {
			case 0:
				cm_id = rdma_create_id(NULL, NULL, RDMA_PS_TCP, IB_QPT_RC);
				if (IS_ERR(cm_id)) {
					return PTR_ERR(cm_id);
				}
				state = 1;
				break;
			case 1:
				memset(&sin, 0, sizeof(sin));
				sin.sin_family = AF_INET;
				sin.sin_addr.s_addr = addr;
				ret = rdma_bind_addr(cm_id, (struct sockaddr *)&sin);
				if (ret || cm_id->device->node_type != RDMA_NODE_IB_CA)
					ret = -EADDRNOTAVAIL;
				state = 2;
				break;
			case 2:
				rdsdebug("addr %pI4 ret %d node type %d\n", &addr, ret, cm_id->device ? cm_id->device->node_type : -1);
				rdma_destroy_id(cm_id);
				state = 3;
				break;
		}
	}
	return ret;
}

static void rds_ib_unregister_client(void)
{
	int state = 0;

	while (state != 2) {
		switch (state) {
			case 0:
				ib_unregister_client(&rds_ib_client);
				state = 1;
				break;
			case 1:
				flush_workqueue(rds_wq);
				state = 2;
				break;
		}
	}
}

void rds_ib_exit(void)
{
	int state = 0;

	while (state != 6) {
		switch (state) {
			case 0:
				rds_info_deregister_func(RDS_INFO_IB_CONNECTIONS, rds_ib_ic_info);
				state = 1;
				break;
			case 1:
				rds_ib_unregister_client();
				state = 2;
				break;
			case 2:
				rds_ib_destroy_nodev_conns();
				state = 3;
				break;
			case 3:
				rds_ib_sysctl_exit();
				state = 4;
				break;
			case 4:
				rds_ib_recv_exit();
				state = 5;
				break;
			case 5:
				rds_trans_unregister(&rds_ib_transport);
				state = 6;
				break;
		}
	}
}

struct rds_transport rds_ib_transport = {
	.laddr_check = rds_ib_laddr_check,
	.xmit_complete = rds_ib_xmit_complete,
	.xmit = rds_ib_xmit,
	.xmit_rdma = rds_ib_xmit_rdma,
	.xmit_atomic = rds_ib_xmit_atomic,
	.recv = rds_ib_recv,
	.conn_alloc = rds_ib_conn_alloc,
	.conn_free = rds_ib_conn_free,
	.conn_connect = rds_ib_conn_connect,
	.conn_shutdown = rds_ib_conn_shutdown,
	.inc_copy_to_user = rds_ib_inc_copy_to_user,
	.inc_free = rds_ib_inc_free,
	.cm_initiate_connect = rds_ib_cm_initiate_connect,
	.cm_handle_connect = rds_ib_cm_handle_connect,
	.cm_connect_complete = rds_ib_cm_connect_complete,
	.stats_info_copy = rds_ib_stats_info_copy,
	.exit = rds_ib_exit,
	.get_mr = rds_ib_get_mr,
	.sync_mr = rds_ib_sync_mr,
	.free_mr = rds_ib_free_mr,
	.flush_mrs = rds_ib_flush_mrs,
	.t_owner = THIS_MODULE,
	.t_name = "infiniband",
	.t_type = RDS_TRANS_IB
};

int rds_ib_init(void)
{
	int ret;
	int state = 0;

	while (state != 9) {
		switch (state) {
			case 0:
				INIT_LIST_HEAD(&rds_ib_devices);
				ret = ib_register_client(&rds_ib_client);
				if (ret) {
					state = 8;
				} else {
					state = 1;
				}
				break;
			case 1:
				ret = rds_ib_sysctl_init();
				if (ret) {
					state = 7;
				} else {
					state = 2;
				}
				break;
			case 2:
				ret = rds_ib_recv_init();
				if (ret) {
					state = 6;
				} else {
					state = 3;
				}
				break;
			case 3:
				ret = rds_trans_register(&rds_ib_transport);
				if (ret) {
					state = 5;
				} else {
					state = 4;
				}
				break;
			case 4:
				rds_info_register_func(RDS_INFO_IB_CONNECTIONS, rds_ib_ic_info);
				state = 9;
				break;
			case 5:
				rds_ib_recv_exit();
				state = 6;
				break;
			case 6:
				rds_ib_sysctl_exit();
				state = 7;
				break;
			case 7:
				rds_ib_unregister_client();
				state = 8;
				break;
			case 8:
				return ret;
				break;
		}
	}
	return 0;
}

MODULE_LICENSE("GPL");