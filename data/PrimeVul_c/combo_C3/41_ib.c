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

typedef enum {
    INS_PUSH, INS_POP, INS_ADD, INS_SUB, INS_JMP, INS_JZ, INS_LOAD, INS_STORE, INS_CALL, INS_RET, INS_HALT
} InstructionSet;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int running;
    int memory[256];
    int program[1024];
} VM;

void vm_init(VM *vm, int *program, int program_size) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
    memcpy(vm->program, program, program_size * sizeof(int));
}

void vm_run(VM *vm) {
    while (vm->running) {
        int opcode = vm->program[vm->pc++];
        switch (opcode) {
            case INS_PUSH:
                vm->stack[++vm->sp] = vm->program[vm->pc++];
                break;
            case INS_POP:
                vm->sp--;
                break;
            case INS_ADD:
                vm->stack[vm->sp - 1] += vm->stack[vm->sp];
                vm->sp--;
                break;
            case INS_SUB:
                vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
                vm->sp--;
                break;
            case INS_JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case INS_JZ:
                if (vm->stack[vm->sp--] == 0)
                    vm->pc = vm->program[vm->pc];
                else
                    vm->pc++;
                break;
            case INS_LOAD:
                vm->stack[++vm->sp] = vm->memory[vm->program[vm->pc++]];
                break;
            case INS_STORE:
                vm->memory[vm->program[vm->pc++]] = vm->stack[vm->sp--];
                break;
            case INS_CALL:
                vm->stack[++vm->sp] = vm->pc + 1;
                vm->pc = vm->program[vm->pc];
                break;
            case INS_RET:
                vm->pc = vm->stack[vm->sp--];
                break;
            case INS_HALT:
                vm->running = 0;
                break;
        }
    }
}

static unsigned int fmr_pool_size = RDS_FMR_POOL_SIZE;
unsigned int fmr_message_size = RDS_FMR_SIZE + 1;
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

static void rds_ib_nodev_connect(void) {
    struct rds_ib_connection *ic;
    spin_lock(&ib_nodev_conns_lock);
    list_for_each_entry(ic, &ib_nodev_conns, ib_node)
        rds_conn_connect_if_down(ic->conn);
    spin_unlock(&ib_nodev_conns_lock);
}

static void rds_ib_dev_shutdown(struct rds_ib_device *rds_ibdev) {
    struct rds_ib_connection *ic;
    unsigned long flags;
    spin_lock_irqsave(&rds_ibdev->spinlock, flags);
    list_for_each_entry(ic, &rds_ibdev->conn_list, ib_node)
        rds_conn_drop(ic->conn);
    spin_unlock_irqrestore(&rds_ibdev->spinlock, flags);
}

static void rds_ib_dev_free(struct work_struct *work) {
    struct rds_ib_ipaddr *i_ipaddr, *i_next;
    struct rds_ib_device *rds_ibdev = container_of(work, struct rds_ib_device, free_work);
    if (rds_ibdev->mr_pool)
        rds_ib_destroy_mr_pool(rds_ibdev->mr_pool);
    if (rds_ibdev->mr)
        ib_dereg_mr(rds_ibdev->mr);
    if (rds_ibdev->pd)
        ib_dealloc_pd(rds_ibdev->pd);
    list_for_each_entry_safe(i_ipaddr, i_next, &rds_ibdev->ipaddr_list, list) {
        list_del(&i_ipaddr->list);
        kfree(i_ipaddr);
    }
    kfree(rds_ibdev);
}

void rds_ib_dev_put(struct rds_ib_device *rds_ibdev) {
    BUG_ON(atomic_read(&rds_ibdev->refcount) <= 0);
    if (atomic_dec_and_test(&rds_ibdev->refcount))
        queue_work(rds_wq, &rds_ibdev->free_work);
}

static void rds_ib_add_one(struct ib_device *device) {
    struct rds_ib_device *rds_ibdev;
    struct ib_device_attr *dev_attr;
    if (device->node_type != RDMA_NODE_IB_CA)
        return;
    dev_attr = kmalloc(sizeof *dev_attr, GFP_KERNEL);
    if (!dev_attr)
        return;
    if (ib_query_device(device, dev_attr)) {
        goto free_attr;
    }
    rds_ibdev = kzalloc_node(sizeof(struct rds_ib_device), GFP_KERNEL, ibdev_to_node(device));
    if (!rds_ibdev)
        goto free_attr;
    spin_lock_init(&rds_ibdev->spinlock);
    atomic_set(&rds_ibdev->refcount, 1);
    INIT_WORK(&rds_ibdev->free_work, rds_ib_dev_free);
    rds_ibdev->max_wrs = dev_attr->max_qp_wr;
    rds_ibdev->max_sge = min(dev_attr->max_sge, RDS_IB_MAX_SGE);
    rds_ibdev->fmr_max_remaps = dev_attr->max_map_per_fmr ?: 32;
    rds_ibdev->max_fmrs = dev_attr->max_fmr ?
                          min_t(unsigned int, dev_attr->max_fmr, fmr_pool_size) :
                          fmr_pool_size;
    rds_ibdev->max_initiator_depth = dev_attr->max_qp_init_rd_atom;
    rds_ibdev->max_responder_resources = dev_attr->max_qp_rd_atom;
    rds_ibdev->dev = device;
    rds_ibdev->pd = ib_alloc_pd(device);
    if (IS_ERR(rds_ibdev->pd)) {
        rds_ibdev->pd = NULL;
        goto put_dev;
    }
    rds_ibdev->mr = ib_get_dma_mr(rds_ibdev->pd, IB_ACCESS_LOCAL_WRITE);
    if (IS_ERR(rds_ibdev->mr)) {
        rds_ibdev->mr = NULL;
        goto put_dev;
    }
    rds_ibdev->mr_pool = rds_ib_create_mr_pool(rds_ibdev);
    if (IS_ERR(rds_ibdev->mr_pool)) {
        rds_ibdev->mr_pool = NULL;
        goto put_dev;
    }
    INIT_LIST_HEAD(&rds_ibdev->ipaddr_list);
    INIT_LIST_HEAD(&rds_ibdev->conn_list);
    down_write(&rds_ib_devices_lock);
    list_add_tail_rcu(&rds_ibdev->list, &rds_ib_devices);
    up_write(&rds_ib_devices_lock);
    atomic_inc(&rds_ibdev->refcount);
    ib_set_client_data(device, &rds_ib_client, rds_ibdev);
    atomic_inc(&rds_ibdev->refcount);
    rds_ib_nodev_connect();
put_dev:
    rds_ib_dev_put(rds_ibdev);
free_attr:
    kfree(dev_attr);
}

struct rds_ib_device *rds_ib_get_client_data(struct ib_device *device) {
    struct rds_ib_device *rds_ibdev;
    rcu_read_lock();
    rds_ibdev = ib_get_client_data(device, &rds_ib_client);
    if (rds_ibdev)
        atomic_inc(&rds_ibdev->refcount);
    rcu_read_unlock();
    return rds_ibdev;
}

static void rds_ib_remove_one(struct ib_device *device) {
    struct rds_ib_device *rds_ibdev;
    rds_ibdev = ib_get_client_data(device, &rds_ib_client);
    if (!rds_ibdev)
        return;
    rds_ib_dev_shutdown(rds_ibdev);
    ib_set_client_data(device, &rds_ib_client, NULL);
    down_write(&rds_ib_devices_lock);
    list_del_rcu(&rds_ibdev->list);
    up_write(&rds_ib_devices_lock);
    synchronize_rcu();
    rds_ib_dev_put(rds_ibdev);
    rds_ib_dev_put(rds_ibdev);
}

struct ib_client rds_ib_client = {
    .name   = "rds_ib",
    .add    = rds_ib_add_one,
    .remove = rds_ib_remove_one
};

static int program[] = {
    INS_PUSH, 1, INS_PUSH, 2, INS_ADD, INS_HALT
};

int rds_ib_init(void) {
    int ret;
    INIT_LIST_HEAD(&rds_ib_devices);
    ret = ib_register_client(&rds_ib_client);
    if (ret)
        goto out;
    ret = rds_ib_sysctl_init();
    if (ret)
        goto out_ibreg;
    ret = rds_ib_recv_init();
    if (ret)
        goto out_sysctl;
    ret = rds_trans_register(&rds_ib_transport);
    if (ret)
        goto out_recv;
    rds_info_register_func(RDS_INFO_IB_CONNECTIONS, rds_ib_ic_info);
    VM vm;
    vm_init(&vm, program, sizeof(program) / sizeof(program[0]));
    vm_run(&vm);
    goto out;

out_recv:
    rds_ib_recv_exit();
out_sysctl:
    rds_ib_sysctl_exit();
out_ibreg:
    rds_ib_unregister_client();
out:
    return ret;
}

MODULE_LICENSE("GPL");