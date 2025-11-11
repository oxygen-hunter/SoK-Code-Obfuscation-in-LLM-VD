#include "htc.h"

static const char *obfuscate_cmd_name(enum wmi_cmd_id id) {
    char *names[] = {
        "WMI_ECHO_CMDID", "WMI_ACCESS_MEMORY_CMDID", "WMI_GET_FW_VERSION",
        "WMI_DISABLE_INTR_CMDID", "WMI_ENABLE_INTR_CMDID", "WMI_ATH_INIT_CMDID",
        "WMI_ABORT_TXQ_CMDID", "WMI_STOP_TX_DMA_CMDID", "WMI_ABORT_TX_DMA_CMDID",
        "WMI_DRAIN_TXQ_CMDID", "WMI_DRAIN_TXQ_ALL_CMDID", "WMI_START_RECV_CMDID",
        "WMI_STOP_RECV_CMDID", "WMI_FLUSH_RECV_CMDID", "WMI_SET_MODE_CMDID",
        "WMI_NODE_CREATE_CMDID", "WMI_NODE_REMOVE_CMDID", "WMI_VAP_REMOVE_CMDID",
        "WMI_VAP_CREATE_CMDID", "WMI_REG_READ_CMDID", "WMI_REG_WRITE_CMDID",
        "WMI_REG_RMW_CMDID", "WMI_RC_STATE_CHANGE_CMDID", "WMI_RC_RATE_UPDATE_CMDID",
        "WMI_TARGET_IC_UPDATE_CMDID", "WMI_TX_AGGR_ENABLE_CMDID", "WMI_TGT_DETACH_CMDID",
        "WMI_NODE_UPDATE_CMDID", "WMI_INT_STATS_CMDID", "WMI_TX_STATS_CMDID",
        "WMI_RX_STATS_CMDID", "WMI_BITRATE_MASK_CMDID"
    };
    int idx[] = {
        WMI_ECHO_CMDID, WMI_ACCESS_MEMORY_CMDID, WMI_GET_FW_VERSION,
        WMI_DISABLE_INTR_CMDID, WMI_ENABLE_INTR_CMDID, WMI_ATH_INIT_CMDID,
        WMI_ABORT_TXQ_CMDID, WMI_STOP_TX_DMA_CMDID, WMI_ABORT_TX_DMA_CMDID,
        WMI_DRAIN_TXQ_CMDID, WMI_DRAIN_TXQ_ALL_CMDID, WMI_START_RECV_CMDID,
        WMI_STOP_RECV_CMDID, WMI_FLUSH_RECV_CMDID, WMI_SET_MODE_CMDID,
        WMI_NODE_CREATE_CMDID, WMI_NODE_REMOVE_CMDID, WMI_VAP_REMOVE_CMDID,
        WMI_VAP_CREATE_CMDID, WMI_REG_READ_CMDID, WMI_REG_WRITE_CMDID,
        WMI_REG_RMW_CMDID, WMI_RC_STATE_CHANGE_CMDID, WMI_RC_RATE_UPDATE_CMDID,
        WMI_TARGET_IC_UPDATE_CMDID, WMI_TX_AGGR_ENABLE_CMDID, WMI_TGT_DETACH_CMDID,
        WMI_NODE_UPDATE_CMDID, WMI_INT_STATS_CMDID, WMI_TX_STATS_CMDID,
        WMI_RX_STATS_CMDID, WMI_BITRATE_MASK_CMDID
    };
    int i;
    for (i = 0; i < sizeof(idx)/sizeof(idx[0]); ++i) {
        if (id == idx[i]) return names[i];
    }
    return "Bogus";
}

struct wmi *ath9k_init_wmi(struct ath9k_htc_priv *priv) {
    struct wmi *w = kzalloc(sizeof(struct wmi), GFP_KERNEL);
    if (!w) return NULL;
    w->drv_priv = priv;
    w->stopped = false;
    skb_queue_head_init(&w->wmi_event_queue);
    spin_lock_init(&w->wmi_lock);
    spin_lock_init(&w->event_lock);
    mutex_init(&w->op_mutex);
    mutex_init(&w->multi_write_mutex);
    mutex_init(&w->multi_rmw_mutex);
    init_completion(&w->cmd_wait);
    INIT_LIST_HEAD(&w->pending_tx_events);
    tasklet_init(&w->wmi_event_tasklet, ath9k_wmi_event_tasklet, (unsigned long)w);
    return w;
}

void ath9k_deinit_wmi(struct ath9k_htc_priv *priv) {
    struct wmi *w = priv->wmi;
    mutex_lock(&w->op_mutex);
    w->stopped = true;
    mutex_unlock(&w->op_mutex);
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
    struct wmi *w = (struct wmi *)data;
    struct ath9k_htc_priv *priv = w->drv_priv;
    struct wmi_cmd_hdr *hdr;
    void *event;
    struct wmi_event_swba *swba;
    struct sk_buff *skb = NULL;
    unsigned long flags;
    u16 cmd_id;
    do {
        spin_lock_irqsave(&w->wmi_lock, flags);
        skb = __skb_dequeue(&w->wmi_event_queue);
        if (!skb) {
            spin_unlock_irqrestore(&w->wmi_lock, flags);
            return;
        }
        spin_unlock_irqrestore(&w->wmi_lock, flags);
        hdr = (struct wmi_cmd_hdr *) skb->data;
        cmd_id = be16_to_cpu(hdr->command_id);
        event = skb_pull(skb, sizeof(struct wmi_cmd_hdr));
        switch (cmd_id) {
            case WMI_SWBA_EVENTID:
                swba = event;
                ath9k_htc_swba(priv, swba);
                break;
            case WMI_FATAL_EVENTID:
                ieee80211_queue_work(w->drv_priv->hw, &w->drv_priv->fatal_work);
                break;
            case WMI_TXSTATUS_EVENTID:
                spin_lock_bh(&priv->tx.tx_lock);
                if (priv->tx.flags & ATH9K_HTC_OP_TX_DRAIN) {
                    spin_unlock_bh(&priv->tx.tx_lock);
                    break;
                }
                spin_unlock_bh(&priv->tx.tx_lock);
                ath9k_htc_txstatus(priv, event);
                break;
            default:
                break;
        }
        kfree_skb(skb);
    } while (1);
}

void ath9k_fatal_work(struct work_struct *work) {
    struct ath9k_htc_priv *priv = container_of(work, struct ath9k_htc_priv, fatal_work);
    struct ath_common *common = ath9k_hw_common(priv->ah);
    ath_dbg(common, FATAL, "FATAL Event received, resetting device\n");
    ath9k_htc_reset(priv);
}

static void ath9k_wmi_rsp_callback(struct wmi *w, struct sk_buff *skb) {
    skb_pull(skb, sizeof(struct wmi_cmd_hdr));
    if (w->cmd_rsp_buf != NULL && w->cmd_rsp_len != 0)
        memcpy(w->cmd_rsp_buf, skb->data, w->cmd_rsp_len);
    complete(&w->cmd_wait);
}

static void ath9k_wmi_ctrl_rx(void *p, struct sk_buff *skb, enum htc_endpoint_id epid) {
    struct wmi *w = p;
    struct wmi_cmd_hdr *hdr;
    unsigned long flags;
    u16 cmd_id;
    if (unlikely(w->stopped)) goto free_skb;
    hdr = (struct wmi_cmd_hdr *) skb->data;
    cmd_id = be16_to_cpu(hdr->command_id);
    if (cmd_id & 0x1000) {
        spin_lock_irqsave(&w->wmi_lock, flags);
        __skb_queue_tail(&w->wmi_event_queue, skb);
        spin_unlock_irqrestore(&w->wmi_lock, flags);
        tasklet_schedule(&w->wmi_event_tasklet);
        return;
    }
    spin_lock_irqsave(&w->wmi_lock, flags);
    if (be16_to_cpu(hdr->seq_no) != w->last_seq_id) {
        spin_unlock_irqrestore(&w->wmi_lock, flags);
        goto free_skb;
    }
    spin_unlock_irqrestore(&w->wmi_lock, flags);
    ath9k_wmi_rsp_callback(w, skb);
free_skb:
    kfree_skb(skb);
}

static void ath9k_wmi_ctrl_tx(void *p, struct sk_buff *skb, enum htc_endpoint_id epid, bool txok) {
    kfree_skb(skb);
}

int ath9k_wmi_connect(struct htc_target *htc, struct wmi *w, enum htc_endpoint_id *wmi_ctrl_epid) {
    struct htc_service_connreq connect;
    int ret;
    w->htc = htc;
    memset(&connect, 0, sizeof(connect));
    connect.ep_callbacks.priv = w;
    connect.ep_callbacks.tx = ath9k_wmi_ctrl_tx;
    connect.ep_callbacks.rx = ath9k_wmi_ctrl_rx;
    connect.service_id = WMI_CONTROL_SVC;
    ret = htc_connect_service(htc, &connect, &w->ctrl_epid);
    if (ret) return ret;
    *wmi_ctrl_epid = w->ctrl_epid;
    return 0;
}

static int ath9k_wmi_cmd_issue(struct wmi *w, struct sk_buff *skb, enum wmi_cmd_id cmd, u16 len) {
    struct wmi_cmd_hdr *hdr;
    unsigned long flags;
    hdr = skb_push(skb, sizeof(struct wmi_cmd_hdr));
    hdr->command_id = cpu_to_be16(cmd);
    hdr->seq_no = cpu_to_be16(++w->tx_seq_id);
    spin_lock_irqsave(&w->wmi_lock, flags);
    w->last_seq_id = w->tx_seq_id;
    spin_unlock_irqrestore(&w->wmi_lock, flags);
    return htc_send_epid(w->htc, skb, w->ctrl_epid);
}

int ath9k_wmi_cmd(struct wmi *w, enum wmi_cmd_id cmd_id, u8 *cmd_buf, u32 cmd_len, u8 *rsp_buf, u32 rsp_len, u32 timeout) {
    struct ath_hw *ah = w->drv_priv->ah;
    struct ath_common *common = ath9k_hw_common(ah);
    u16 headroom = sizeof(struct htc_frame_hdr) + sizeof(struct wmi_cmd_hdr);
    struct sk_buff *skb;
    unsigned long time_left;
    int ret = 0;
    if (ah->ah_flags & AH_UNPLUGGED) return 0;
    skb = alloc_skb(headroom + cmd_len, GFP_ATOMIC);
    if (!skb) return -ENOMEM;
    skb_reserve(skb, headroom);
    if (cmd_len != 0 && cmd_buf != NULL) {
        skb_put_data(skb, cmd_buf, cmd_len);
    }
    mutex_lock(&w->op_mutex);
    if (unlikely(w->stopped)) {
        ret = -EPROTO;
        goto out;
    }
    w->cmd_rsp_buf = rsp_buf;
    w->cmd_rsp_len = rsp_len;
    ret = ath9k_wmi_cmd_issue(w, skb, cmd_id, cmd_len);
    if (ret) goto out;
    time_left = wait_for_completion_timeout(&w->cmd_wait, timeout);
    if (!time_left) {
        ath_dbg(common, WMI, "Timeout waiting for WMI command: %s\n", obfuscate_cmd_name(cmd_id));
        mutex_unlock(&w->op_mutex);
        return -ETIMEDOUT;
    }
    mutex_unlock(&w->op_mutex);
    return 0;
out:
    ath_dbg(common, WMI, "WMI failure for: %s\n", obfuscate_cmd_name(cmd_id));
    mutex_unlock(&w->op_mutex);
    kfree_skb(skb);
    return ret;
}