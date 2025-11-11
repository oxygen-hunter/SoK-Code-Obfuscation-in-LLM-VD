#include "htc.h"

enum VM_Instructions {
    VM_PUSH,
    VM_POP,
    VM_ADD,
    VM_SUB,
    VM_JMP,
    VM_JZ,
    VM_LOAD,
    VM_STORE,
    VM_HALT
};

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int running;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(VM *vm, int *program) {
    while (vm->running) {
        int instr = program[vm->pc++];
        switch (instr) {
            case VM_PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case VM_POP:
                vm_pop(vm);
                break;
            case VM_ADD: {
                int a = vm_pop(vm);
                int b = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case VM_SUB: {
                int a = vm_pop(vm);
                int b = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case VM_JMP:
                vm->pc = program[vm->pc];
                break;
            case VM_JZ: {
                int cond = vm_pop(vm);
                if (cond == 0) {
                    vm->pc = program[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            }
            case VM_LOAD: {
                int idx = program[vm->pc++];
                vm_push(vm, vm->stack[idx]);
                break;
            }
            case VM_STORE: {
                int idx = program[vm->pc++];
                vm->stack[idx] = vm_pop(vm);
                break;
            }
            case VM_HALT:
                vm->running = 0;
                break;
        }
    }
}

static const char *wmi_cmd_to_name(enum wmi_cmd_id wmi_cmd) {
    VM vm;
    int program[] = {
        VM_PUSH, WMI_ECHO_CMDID, VM_PUSH, (int)"WMI_ECHO_CMDID",
        VM_PUSH, WMI_ACCESS_MEMORY_CMDID, VM_PUSH, (int)"WMI_ACCESS_MEMORY_CMDID",
        VM_PUSH, WMI_GET_FW_VERSION, VM_PUSH, (int)"WMI_GET_FW_VERSION",
        VM_PUSH, WMI_DISABLE_INTR_CMDID, VM_PUSH, (int)"WMI_DISABLE_INTR_CMDID",
        VM_PUSH, WMI_ENABLE_INTR_CMDID, VM_PUSH, (int)"WMI_ENABLE_INTR_CMDID",
        VM_PUSH, WMI_ATH_INIT_CMDID, VM_PUSH, (int)"WMI_ATH_INIT_CMDID",
        VM_PUSH, WMI_ABORT_TXQ_CMDID, VM_PUSH, (int)"WMI_ABORT_TXQ_CMDID",
        VM_PUSH, WMI_STOP_TX_DMA_CMDID, VM_PUSH, (int)"WMI_STOP_TX_DMA_CMDID",
        VM_PUSH, WMI_ABORT_TX_DMA_CMDID, VM_PUSH, (int)"WMI_ABORT_TX_DMA_CMDID",
        VM_PUSH, WMI_DRAIN_TXQ_CMDID, VM_PUSH, (int)"WMI_DRAIN_TXQ_CMDID",
        VM_PUSH, WMI_DRAIN_TXQ_ALL_CMDID, VM_PUSH, (int)"WMI_DRAIN_TXQ_ALL_CMDID",
        VM_PUSH, WMI_START_RECV_CMDID, VM_PUSH, (int)"WMI_START_RECV_CMDID",
        VM_PUSH, WMI_STOP_RECV_CMDID, VM_PUSH, (int)"WMI_STOP_RECV_CMDID",
        VM_PUSH, WMI_FLUSH_RECV_CMDID, VM_PUSH, (int)"WMI_FLUSH_RECV_CMDID",
        VM_PUSH, WMI_SET_MODE_CMDID, VM_PUSH, (int)"WMI_SET_MODE_CMDID",
        VM_PUSH, WMI_NODE_CREATE_CMDID, VM_PUSH, (int)"WMI_NODE_CREATE_CMDID",
        VM_PUSH, WMI_NODE_REMOVE_CMDID, VM_PUSH, (int)"WMI_NODE_REMOVE_CMDID",
        VM_PUSH, WMI_VAP_REMOVE_CMDID, VM_PUSH, (int)"WMI_VAP_REMOVE_CMDID",
        VM_PUSH, WMI_VAP_CREATE_CMDID, VM_PUSH, (int)"WMI_VAP_CREATE_CMDID",
        VM_PUSH, WMI_REG_READ_CMDID, VM_PUSH, (int)"WMI_REG_READ_CMDID",
        VM_PUSH, WMI_REG_WRITE_CMDID, VM_PUSH, (int)"WMI_REG_WRITE_CMDID",
        VM_PUSH, WMI_REG_RMW_CMDID, VM_PUSH, (int)"WMI_REG_RMW_CMDID",
        VM_PUSH, WMI_RC_STATE_CHANGE_CMDID, VM_PUSH, (int)"WMI_RC_STATE_CHANGE_CMDID",
        VM_PUSH, WMI_RC_RATE_UPDATE_CMDID, VM_PUSH, (int)"WMI_RC_RATE_UPDATE_CMDID",
        VM_PUSH, WMI_TARGET_IC_UPDATE_CMDID, VM_PUSH, (int)"WMI_TARGET_IC_UPDATE_CMDID",
        VM_PUSH, WMI_TX_AGGR_ENABLE_CMDID, VM_PUSH, (int)"WMI_TX_AGGR_ENABLE_CMDID",
        VM_PUSH, WMI_TGT_DETACH_CMDID, VM_PUSH, (int)"WMI_TGT_DETACH_CMDID",
        VM_PUSH, WMI_NODE_UPDATE_CMDID, VM_PUSH, (int)"WMI_NODE_UPDATE_CMDID",
        VM_PUSH, WMI_INT_STATS_CMDID, VM_PUSH, (int)"WMI_INT_STATS_CMDID",
        VM_PUSH, WMI_TX_STATS_CMDID, VM_PUSH, (int)"WMI_TX_STATS_CMDID",
        VM_PUSH, WMI_RX_STATS_CMDID, VM_PUSH, (int)"WMI_RX_STATS_CMDID",
        VM_PUSH, WMI_BITRATE_MASK_CMDID, VM_PUSH, (int)"WMI_BITRATE_MASK_CMDID",
        VM_PUSH, wmi_cmd, VM_LOAD, 0, VM_JZ, 130, VM_LOAD, 1, VM_RETURN, 
        VM_HALT
    };
    vm_init(&vm);
    vm_execute(&vm, program);
    return (const char *)vm_pop(&vm);
}

struct wmi *ath9k_init_wmi(struct ath9k_htc_priv *priv) {
    struct wmi *wmi;

    wmi = kzalloc(sizeof(struct wmi), GFP_KERNEL);
    if (!wmi)
        return NULL;

    wmi->drv_priv = priv;
    wmi->stopped = false;
    skb_queue_head_init(&wmi->wmi_event_queue);
    spin_lock_init(&wmi->wmi_lock);
    spin_lock_init(&wmi->event_lock);
    mutex_init(&wmi->op_mutex);
    mutex_init(&wmi->multi_write_mutex);
    mutex_init(&wmi->multi_rmw_mutex);
    init_completion(&wmi->cmd_wait);
    INIT_LIST_HEAD(&wmi->pending_tx_events);
    tasklet_init(&wmi->wmi_event_tasklet, ath9k_wmi_event_tasklet,
                 (unsigned long)wmi);

    return wmi;
}

void ath9k_deinit_wmi(struct ath9k_htc_priv *priv) {
    struct wmi *wmi = priv->wmi;

    mutex_lock(&wmi->op_mutex);
    wmi->stopped = true;
    mutex_unlock(&wmi->op_mutex);

    kfree(priv->wmi);
}

void ath9k_wmi_event_drain(struct ath9k_htc_priv *priv) {
    unsigned long flags;

    tasklet_kill(&priv->wmi->wmi_event_tasklet);
    spin_lock_irqsave(&priv->wmi->wmi_lock, flags);
    __skb_queue_purge(&priv->wmi->wmi_event_queue);
    spin_unlock_irqrestore(&priv->wmi->wmi_lock, flags);
}

void ath9k_wmi_event_tasklet(unsigned long data) {
    struct wmi *wmi = (struct wmi *)data;
    struct ath9k_htc_priv *priv = wmi->drv_priv;
    struct wmi_cmd_hdr *hdr;
    void *wmi_event;
    struct wmi_event_swba *swba;
    struct sk_buff *skb = NULL;
    unsigned long flags;
    u16 cmd_id;

    do {
        spin_lock_irqsave(&wmi->wmi_lock, flags);
        skb = __skb_dequeue(&wmi->wmi_event_queue);
        if (!skb) {
            spin_unlock_irqrestore(&wmi->wmi_lock, flags);
            return;
        }
        spin_unlock_irqrestore(&wmi->wmi_lock, flags);

        hdr = (struct wmi_cmd_hdr *)skb->data;
        cmd_id = be16_to_cpu(hdr->command_id);
        wmi_event = skb_pull(skb, sizeof(struct wmi_cmd_hdr));

        VM vm;
        int program[] = {
            VM_PUSH, cmd_id, VM_PUSH, WMI_SWBA_EVENTID, VM_JZ, 8,
            VM_PUSH, (int)priv, VM_PUSH, (int)swba, VM_HALT,
            VM_PUSH, cmd_id, VM_PUSH, WMI_FATAL_EVENTID, VM_JZ, 24,
            VM_PUSH, (int)&wmi->drv_priv->hw, VM_PUSH, (int)&wmi->drv_priv->fatal_work, VM_HALT,
            VM_PUSH, cmd_id, VM_PUSH, WMI_TXSTATUS_EVENTID, VM_JZ, 38,
            VM_PUSH, (int)&priv->tx.tx_lock, VM_HALT, VM_HALT
        };
        vm_init(&vm);
        vm_execute(&vm, program);
        int action = vm_pop(&vm);

        switch (action) {
            case 0:
                swba = wmi_event;
                ath9k_htc_swba(priv, swba);
                break;
            case 1:
                ieee80211_queue_work(wmi->drv_priv->hw,
                                     &wmi->drv_priv->fatal_work);
                break;
            case 2:
                spin_lock_bh(&priv->tx.tx_lock);
                if (priv->tx.flags & ATH9K_HTC_OP_TX_DRAIN) {
                    spin_unlock_bh(&priv->tx.tx_lock);
                    break;
                }
                spin_unlock_bh(&priv->tx.tx_lock);
                ath9k_htc_txstatus(priv, wmi_event);
                break;
            default:
                break;
        }

        kfree_skb(skb);
    } while (1);
}

void ath9k_fatal_work(struct work_struct *work) {
    struct ath9k_htc_priv *priv = container_of(work, struct ath9k_htc_priv,
                                               fatal_work);
    struct ath_common *common = ath9k_hw_common(priv->ah);

    ath_dbg(common, FATAL, "FATAL Event received, resetting device\n");
    ath9k_htc_reset(priv);
}

static void ath9k_wmi_rsp_callback(struct wmi *wmi, struct sk_buff *skb) {
    skb_pull(skb, sizeof(struct wmi_cmd_hdr));

    if (wmi->cmd_rsp_buf != NULL && wmi->cmd_rsp_len != 0)
        memcpy(wmi->cmd_rsp_buf, skb->data, wmi->cmd_rsp_len);

    complete(&wmi->cmd_wait);
}

static void ath9k_wmi_ctrl_rx(void *priv, struct sk_buff *skb,
                              enum htc_endpoint_id epid) {
    struct wmi *wmi = priv;
    struct wmi_cmd_hdr *hdr;
    unsigned long flags;
    u16 cmd_id;

    if (unlikely(wmi->stopped))
        goto free_skb;

    hdr = (struct wmi_cmd_hdr *)skb->data;
    cmd_id = be16_to_cpu(hdr->command_id);

    if (cmd_id & 0x1000) {
        spin_lock_irqsave(&wmi->wmi_lock, flags);
        __skb_queue_tail(&wmi->wmi_event_queue, skb);
        spin_unlock_irqrestore(&wmi->wmi_lock, flags);
        tasklet_schedule(&wmi->wmi_event_tasklet);
        return;
    }

    spin_lock_irqsave(&wmi->wmi_lock, flags);
    if (be16_to_cpu(hdr->seq_no) != wmi->last_seq_id) {
        spin_unlock_irqrestore(&wmi->wmi_lock, flags);
        goto free_skb;
    }
    spin_unlock_irqrestore(&wmi->wmi_lock, flags);

    ath9k_wmi_rsp_callback(wmi, skb);

free_skb:
    kfree_skb(skb);
}

static void ath9k_wmi_ctrl_tx(void *priv, struct sk_buff *skb,
                              enum htc_endpoint_id epid, bool txok) {
    kfree_skb(skb);
}

int ath9k_wmi_connect(struct htc_target *htc, struct wmi *wmi,
                      enum htc_endpoint_id *wmi_ctrl_epid) {
    struct htc_service_connreq connect;
    int ret;

    wmi->htc = htc;

    memset(&connect, 0, sizeof(connect));

    connect.ep_callbacks.priv = wmi;
    connect.ep_callbacks.tx = ath9k_wmi_ctrl_tx;
    connect.ep_callbacks.rx = ath9k_wmi_ctrl_rx;
    connect.service_id = WMI_CONTROL_SVC;

    ret = htc_connect_service(htc, &connect, &wmi->ctrl_epid);
    if (ret)
        return ret;

    *wmi_ctrl_epid = wmi->ctrl_epid;

    return 0;
}

static int ath9k_wmi_cmd_issue(struct wmi *wmi,
                               struct sk_buff *skb,
                               enum wmi_cmd_id cmd, u16 len) {
    struct wmi_cmd_hdr *hdr;
    unsigned long flags;

    hdr = skb_push(skb, sizeof(struct wmi_cmd_hdr));
    hdr->command_id = cpu_to_be16(cmd);
    hdr->seq_no = cpu_to_be16(++wmi->tx_seq_id);

    spin_lock_irqsave(&wmi->wmi_lock, flags);
    wmi->last_seq_id = wmi->tx_seq_id;
    spin_unlock_irqrestore(&wmi->wmi_lock, flags);

    return htc_send_epid(wmi->htc, skb, wmi->ctrl_epid);
}

int ath9k_wmi_cmd(struct wmi *wmi, enum wmi_cmd_id cmd_id,
                  u8 *cmd_buf, u32 cmd_len,
                  u8 *rsp_buf, u32 rsp_len,
                  u32 timeout) {
    struct ath_hw *ah = wmi->drv_priv->ah;
    struct ath_common *common = ath9k_hw_common(ah);
    u16 headroom = sizeof(struct htc_frame_hdr) +
                   sizeof(struct wmi_cmd_hdr);
    struct sk_buff *skb;
    unsigned long time_left;
    int ret = 0;

    if (ah->ah_flags & AH_UNPLUGGED)
        return 0;

    skb = alloc_skb(headroom + cmd_len, GFP_ATOMIC);
    if (!skb)
        return -ENOMEM;

    skb_reserve(skb, headroom);

    if (cmd_len != 0 && cmd_buf != NULL) {
        skb_put_data(skb, cmd_buf, cmd_len);
    }

    mutex_lock(&wmi->op_mutex);

    if (unlikely(wmi->stopped)) {
        ret = -EPROTO;
        goto out;
    }

    wmi->cmd_rsp_buf = rsp_buf;
    wmi->cmd_rsp_len = rsp_len;

    ret = ath9k_wmi_cmd_issue(wmi, skb, cmd_id, cmd_len);
    if (ret)
        goto out;

    time_left = wait_for_completion_timeout(&wmi->cmd_wait, timeout);
    if (!time_left) {
        ath_dbg(common, WMI, "Timeout waiting for WMI command: %s\n",
                wmi_cmd_to_name(cmd_id));
        mutex_unlock(&wmi->op_mutex);
        return -ETIMEDOUT;
    }

    mutex_unlock(&wmi->op_mutex);

    return 0;

out:
    ath_dbg(common, WMI, "WMI failure for: %s\n", wmi_cmd_to_name(cmd_id));
    mutex_unlock(&wmi->op_mutex);
    kfree_skb(skb);

    return ret;
}