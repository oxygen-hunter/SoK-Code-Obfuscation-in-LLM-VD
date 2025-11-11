#include "htc.h"

static const char *wmi_cmd_to_name(enum wmi_cmd_id wmi_cmd)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				if (wmi_cmd == WMI_ECHO_CMDID) return "WMI_ECHO_CMDID";
				state = 1;
				break;
			case 1:
				if (wmi_cmd == WMI_ACCESS_MEMORY_CMDID) return "WMI_ACCESS_MEMORY_CMDID";
				state = 2;
				break;
			case 2:
				if (wmi_cmd == WMI_GET_FW_VERSION) return "WMI_GET_FW_VERSION";
				state = 3;
				break;
			case 3:
				if (wmi_cmd == WMI_DISABLE_INTR_CMDID) return "WMI_DISABLE_INTR_CMDID";
				state = 4;
				break;
			case 4:
				if (wmi_cmd == WMI_ENABLE_INTR_CMDID) return "WMI_ENABLE_INTR_CMDID";
				state = 5;
				break;
			case 5:
				if (wmi_cmd == WMI_ATH_INIT_CMDID) return "WMI_ATH_INIT_CMDID";
				state = 6;
				break;
			case 6:
				if (wmi_cmd == WMI_ABORT_TXQ_CMDID) return "WMI_ABORT_TXQ_CMDID";
				state = 7;
				break;
			case 7:
				if (wmi_cmd == WMI_STOP_TX_DMA_CMDID) return "WMI_STOP_TX_DMA_CMDID";
				state = 8;
				break;
			case 8:
				if (wmi_cmd == WMI_ABORT_TX_DMA_CMDID) return "WMI_ABORT_TX_DMA_CMDID";
				state = 9;
				break;
			case 9:
				if (wmi_cmd == WMI_DRAIN_TXQ_CMDID) return "WMI_DRAIN_TXQ_CMDID";
				state = 10;
				break;
			case 10:
				if (wmi_cmd == WMI_DRAIN_TXQ_ALL_CMDID) return "WMI_DRAIN_TXQ_ALL_CMDID";
				state = 11;
				break;
			case 11:
				if (wmi_cmd == WMI_START_RECV_CMDID) return "WMI_START_RECV_CMDID";
				state = 12;
				break;
			case 12:
				if (wmi_cmd == WMI_STOP_RECV_CMDID) return "WMI_STOP_RECV_CMDID";
				state = 13;
				break;
			case 13:
				if (wmi_cmd == WMI_FLUSH_RECV_CMDID) return "WMI_FLUSH_RECV_CMDID";
				state = 14;
				break;
			case 14:
				if (wmi_cmd == WMI_SET_MODE_CMDID) return "WMI_SET_MODE_CMDID";
				state = 15;
				break;
			case 15:
				if (wmi_cmd == WMI_NODE_CREATE_CMDID) return "WMI_NODE_CREATE_CMDID";
				state = 16;
				break;
			case 16:
				if (wmi_cmd == WMI_NODE_REMOVE_CMDID) return "WMI_NODE_REMOVE_CMDID";
				state = 17;
				break;
			case 17:
				if (wmi_cmd == WMI_VAP_REMOVE_CMDID) return "WMI_VAP_REMOVE_CMDID";
				state = 18;
				break;
			case 18:
				if (wmi_cmd == WMI_VAP_CREATE_CMDID) return "WMI_VAP_CREATE_CMDID";
				state = 19;
				break;
			case 19:
				if (wmi_cmd == WMI_REG_READ_CMDID) return "WMI_REG_READ_CMDID";
				state = 20;
				break;
			case 20:
				if (wmi_cmd == WMI_REG_WRITE_CMDID) return "WMI_REG_WRITE_CMDID";
				state = 21;
				break;
			case 21:
				if (wmi_cmd == WMI_REG_RMW_CMDID) return "WMI_REG_RMW_CMDID";
				state = 22;
				break;
			case 22:
				if (wmi_cmd == WMI_RC_STATE_CHANGE_CMDID) return "WMI_RC_STATE_CHANGE_CMDID";
				state = 23;
				break;
			case 23:
				if (wmi_cmd == WMI_RC_RATE_UPDATE_CMDID) return "WMI_RC_RATE_UPDATE_CMDID";
				state = 24;
				break;
			case 24:
				if (wmi_cmd == WMI_TARGET_IC_UPDATE_CMDID) return "WMI_TARGET_IC_UPDATE_CMDID";
				state = 25;
				break;
			case 25:
				if (wmi_cmd == WMI_TX_AGGR_ENABLE_CMDID) return "WMI_TX_AGGR_ENABLE_CMDID";
				state = 26;
				break;
			case 26:
				if (wmi_cmd == WMI_TGT_DETACH_CMDID) return "WMI_TGT_DETACH_CMDID";
				state = 27;
				break;
			case 27:
				if (wmi_cmd == WMI_NODE_UPDATE_CMDID) return "WMI_NODE_UPDATE_CMDID";
				state = 28;
				break;
			case 28:
				if (wmi_cmd == WMI_INT_STATS_CMDID) return "WMI_INT_STATS_CMDID";
				state = 29;
				break;
			case 29:
				if (wmi_cmd == WMI_TX_STATS_CMDID) return "WMI_TX_STATS_CMDID";
				state = 30;
				break;
			case 30:
				if (wmi_cmd == WMI_RX_STATS_CMDID) return "WMI_RX_STATS_CMDID";
				state = 31;
				break;
			case 31:
				if (wmi_cmd == WMI_BITRATE_MASK_CMDID) return "WMI_BITRATE_MASK_CMDID";
				state = 32;
				break;
			default:
				return "Bogus";
		}
	}
}

struct wmi *ath9k_init_wmi(struct ath9k_htc_priv *priv)
{
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

void ath9k_deinit_wmi(struct ath9k_htc_priv *priv)
{
	struct wmi *wmi = priv->wmi;

	mutex_lock(&wmi->op_mutex);
	wmi->stopped = true;
	mutex_unlock(&wmi->op_mutex);

	kfree(priv->wmi);
}

void ath9k_wmi_event_drain(struct ath9k_htc_priv *priv)
{
	unsigned long flags;

	tasklet_kill(&priv->wmi->wmi_event_tasklet);
	spin_lock_irqsave(&priv->wmi->wmi_lock, flags);
	__skb_queue_purge(&priv->wmi->wmi_event_queue);
	spin_unlock_irqrestore(&priv->wmi->wmi_lock, flags);
}

void ath9k_wmi_event_tasklet(unsigned long data)
{
	struct wmi *wmi = (struct wmi *)data;
	struct ath9k_htc_priv *priv = wmi->drv_priv;
	struct wmi_cmd_hdr *hdr;
	void *wmi_event;
	struct wmi_event_swba *swba;
	struct sk_buff *skb = NULL;
	unsigned long flags;
	u16 cmd_id;
	int state = 0;

	do {
		spin_lock_irqsave(&wmi->wmi_lock, flags);
		skb = __skb_dequeue(&wmi->wmi_event_queue);
		if (!skb) {
			spin_unlock_irqrestore(&wmi->wmi_lock, flags);
			return;
		}
		spin_unlock_irqrestore(&wmi->wmi_lock, flags);

		hdr = (struct wmi_cmd_hdr *) skb->data;
		cmd_id = be16_to_cpu(hdr->command_id);
		wmi_event = skb_pull(skb, sizeof(struct wmi_cmd_hdr));

		while (1) {
			switch (state) {
				case 0:
					if (cmd_id == WMI_SWBA_EVENTID) {
						swba = wmi_event;
						ath9k_htc_swba(priv, swba);
						state = 1;
					} else {
						state = 2;
					}
					break;
				case 1:
					state = 5;
					break;
				case 2:
					if (cmd_id == WMI_FATAL_EVENTID) {
						ieee80211_queue_work(wmi->drv_priv->hw,
								     &wmi->drv_priv->fatal_work);
						state = 3;
					} else {
						state = 4;
					}
					break;
				case 3:
					state = 5;
					break;
				case 4:
					if (cmd_id == WMI_TXSTATUS_EVENTID) {
						spin_lock_bh(&priv->tx.tx_lock);
						if (priv->tx.flags & ATH9K_HTC_OP_TX_DRAIN) {
							spin_unlock_bh(&priv->tx.tx_lock);
							state = 5;
							break;
						}
						spin_unlock_bh(&priv->tx.tx_lock);

						ath9k_htc_txstatus(priv, wmi_event);
					}
					state = 5;
					break;
				case 5:
					kfree_skb(skb);
					state = 6;
					break;
				case 6:
					break;
			}
			if (state == 6) break;
		}
	} while (1);
}

void ath9k_fatal_work(struct work_struct *work)
{
	struct ath9k_htc_priv *priv = container_of(work, struct ath9k_htc_priv,
						   fatal_work);
	struct ath_common *common = ath9k_hw_common(priv->ah);

	ath_dbg(common, FATAL, "FATAL Event received, resetting device\n");
	ath9k_htc_reset(priv);
}

static void ath9k_wmi_rsp_callback(struct wmi *wmi, struct sk_buff *skb)
{
	skb_pull(skb, sizeof(struct wmi_cmd_hdr));

	if (wmi->cmd_rsp_buf != NULL && wmi->cmd_rsp_len != 0)
		memcpy(wmi->cmd_rsp_buf, skb->data, wmi->cmd_rsp_len);

	complete(&wmi->cmd_wait);
}

static void ath9k_wmi_ctrl_rx(void *priv, struct sk_buff *skb,
			      enum htc_endpoint_id epid)
{
	struct wmi *wmi = priv;
	struct wmi_cmd_hdr *hdr;
	unsigned long flags;
	u16 cmd_id;

	if (unlikely(wmi->stopped))
		goto free_skb;

	hdr = (struct wmi_cmd_hdr *) skb->data;
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
			      enum htc_endpoint_id epid, bool txok)
{
	kfree_skb(skb);
}

int ath9k_wmi_connect(struct htc_target *htc, struct wmi *wmi,
		      enum htc_endpoint_id *wmi_ctrl_epid)
{
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
			       enum wmi_cmd_id cmd, u16 len)
{
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
		  u32 timeout)
{
	struct ath_hw *ah = wmi->drv_priv->ah;
	struct ath_common *common = ath9k_hw_common(ah);
	u16 headroom = sizeof(struct htc_frame_hdr) +
		       sizeof(struct wmi_cmd_hdr);
	struct sk_buff *skb;
	unsigned long time_left;
	int ret = 0;

	int state = 0;

	if (ah->ah_flags & AH_UNPLUGGED)
		return 0;

	skb = alloc_skb(headroom + cmd_len, GFP_ATOMIC);
	if (!skb)
		return -ENOMEM;

	skb_reserve(skb, headroom);

	if (cmd_len != 0 && cmd_buf != NULL) {
		skb_put_data(skb, cmd_buf, cmd_len);
	}

	while (1) {
		switch (state) {
			case 0:
				mutex_lock(&wmi->op_mutex);
				if (unlikely(wmi->stopped)) {
					ret = -EPROTO;
					state = 6;
				} else {
					wmi->cmd_rsp_buf = rsp_buf;
					wmi->cmd_rsp_len = rsp_len;
					state = 1;
				}
				break;
			case 1:
				ret = ath9k_wmi_cmd_issue(wmi, skb, cmd_id, cmd_len);
				if (ret) state = 6;
				else state = 2;
				break;
			case 2:
				time_left = wait_for_completion_timeout(&wmi->cmd_wait, timeout);
				if (!time_left) {
					ath_dbg(common, WMI, "Timeout waiting for WMI command: %s\n",
						wmi_cmd_to_name(cmd_id));
					mutex_unlock(&wmi->op_mutex);
					return -ETIMEDOUT;
				}
				state = 3;
				break;
			case 3:
				mutex_unlock(&wmi->op_mutex);
				return 0;
			case 6:
				ath_dbg(common, WMI, "WMI failure for: %s\n", wmi_cmd_to_name(cmd_id));
				mutex_unlock(&wmi->op_mutex);
				kfree_skb(skb);
				return ret;
		}
	}
}