#include "iwl-trans.h"
#include "iwl-fh.h"
#include "iwl-context-info-gen3.h"
#include "internal.h"
#include "iwl-prph.h"

struct GlobalData {
    struct iwl_context_info_gen3 *ctxt_info_gen3;
    struct iwl_prph_scratch *prph_scratch;
    struct iwl_prph_info *prph_info;
    u32 control_flags;
    int ret;
    int cmdq_size;
    void *iml_img;
} globalData;

int iwl_pcie_ctxt_info_gen3_init(struct iwl_trans *trans,
				 const struct fw_img *fw)
{
    struct iwl_trans_pcie *trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans);
    struct iwl_prph_scratch_ctrl_cfg *prph_sc_ctrl;

    globalData.cmdq_size = max_t(u32, IWL_CMD_QUEUE_SIZE,
			      trans->cfg->min_txq_size);

    globalData.prph_scratch = dma_alloc_coherent(trans->dev, sizeof(*globalData.prph_scratch),
					  &trans_pcie->prph_scratch_dma_addr,
					  GFP_KERNEL);
    if (!globalData.prph_scratch)
		return -ENOMEM;

    prph_sc_ctrl = &globalData.prph_scratch->ctrl_cfg;

    prph_sc_ctrl->version.version = 0;
    prph_sc_ctrl->version.mac_id =
		cpu_to_le16((u16)iwl_read32(trans, CSR_HW_REV));
    prph_sc_ctrl->version.size = cpu_to_le16(sizeof(*globalData.prph_scratch) / 4);

    globalData.control_flags = IWL_PRPH_SCRATCH_RB_SIZE_4K |
			IWL_PRPH_SCRATCH_MTR_MODE |
			(IWL_PRPH_MTR_FORMAT_256B &
			 IWL_PRPH_SCRATCH_MTR_FORMAT) |
			IWL_PRPH_SCRATCH_EARLY_DEBUG_EN |
			IWL_PRPH_SCRATCH_EDBG_DEST_DRAM;
    prph_sc_ctrl->control.control_flags = cpu_to_le32(globalData.control_flags);

    prph_sc_ctrl->rbd_cfg.free_rbd_addr =
		cpu_to_le64(trans_pcie->rxq->bd_dma);

    if (!iwl_trans_dbg_ini_valid(trans))
		iwl_pcie_alloc_fw_monitor(trans, 0);
    if (trans->dbg.num_blocks) {
		prph_sc_ctrl->hwm_cfg.hwm_base_addr =
			cpu_to_le64(trans->dbg.fw_mon[0].physical);
		prph_sc_ctrl->hwm_cfg.hwm_size =
			cpu_to_le32(trans->dbg.fw_mon[0].size);
	}

    globalData.ret = iwl_pcie_init_fw_sec(trans, fw, &globalData.prph_scratch->dram);
    if (globalData.ret) {
		dma_free_coherent(trans->dev,
				  sizeof(*globalData.prph_scratch),
				  globalData.prph_scratch,
				  trans_pcie->prph_scratch_dma_addr);
		return globalData.ret;
	}

    globalData.prph_info = dma_alloc_coherent(trans->dev, sizeof(*globalData.prph_info),
				       &trans_pcie->prph_info_dma_addr,
				       GFP_KERNEL);
    if (!globalData.prph_info)
		return -ENOMEM;

    globalData.ctxt_info_gen3 = dma_alloc_coherent(trans->dev,
					    sizeof(*globalData.ctxt_info_gen3),
					    &trans_pcie->ctxt_info_dma_addr,
					    GFP_KERNEL);
    if (!globalData.ctxt_info_gen3)
		return -ENOMEM;

    globalData.ctxt_info_gen3->prph_info_base_addr =
		cpu_to_le64(trans_pcie->prph_info_dma_addr);
    globalData.ctxt_info_gen3->prph_scratch_base_addr =
		cpu_to_le64(trans_pcie->prph_scratch_dma_addr);
    globalData.ctxt_info_gen3->prph_scratch_size =
		cpu_to_le32(sizeof(*globalData.prph_scratch));
    globalData.ctxt_info_gen3->cr_head_idx_arr_base_addr =
		cpu_to_le64(trans_pcie->rxq->rb_stts_dma);
    globalData.ctxt_info_gen3->tr_tail_idx_arr_base_addr =
		cpu_to_le64(trans_pcie->rxq->tr_tail_dma);
    globalData.ctxt_info_gen3->cr_tail_idx_arr_base_addr =
		cpu_to_le64(trans_pcie->rxq->cr_tail_dma);
    globalData.ctxt_info_gen3->cr_idx_arr_size =
		cpu_to_le16(IWL_NUM_OF_COMPLETION_RINGS);
    globalData.ctxt_info_gen3->tr_idx_arr_size =
		cpu_to_le16(IWL_NUM_OF_TRANSFER_RINGS);
    globalData.ctxt_info_gen3->mtr_base_addr =
		cpu_to_le64(trans_pcie->txq[trans_pcie->cmd_queue]->dma_addr);
    globalData.ctxt_info_gen3->mcr_base_addr =
		cpu_to_le64(trans_pcie->rxq->used_bd_dma);
    globalData.ctxt_info_gen3->mtr_size =
		cpu_to_le16(TFD_QUEUE_CB_SIZE(globalData.cmdq_size));
    globalData.ctxt_info_gen3->mcr_size =
		cpu_to_le16(RX_QUEUE_CB_SIZE(MQ_RX_TABLE_SIZE));

    trans_pcie->ctxt_info_gen3 = globalData.ctxt_info_gen3;
    trans_pcie->prph_info = globalData.prph_info;
    trans_pcie->prph_scratch = globalData.prph_scratch;

    globalData.iml_img = dma_alloc_coherent(trans->dev, trans->iml_len,
				     &trans_pcie->iml_dma_addr, GFP_KERNEL);
    if (!globalData.iml_img)
		return -ENOMEM;

    memcpy(globalData.iml_img, trans->iml, trans->iml_len);

    iwl_enable_fw_load_int_ctx_info(trans);

    iwl_write64(trans, CSR_CTXT_INFO_ADDR,
		    trans_pcie->ctxt_info_dma_addr);
    iwl_write64(trans, CSR_IML_DATA_ADDR,
		    trans_pcie->iml_dma_addr);
    iwl_write32(trans, CSR_IML_SIZE_ADDR, trans->iml_len);

    iwl_set_bit(trans, CSR_CTXT_INFO_BOOT_CTRL,
		    CSR_AUTO_FUNC_BOOT_ENA);
    if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_AX210)
		iwl_write_umac_prph(trans, UREG_CPU_INIT_RUN, 1);
    else
		iwl_set_bit(trans, CSR_GP_CNTRL, CSR_AUTO_FUNC_INIT);

    return 0;
}

void iwl_pcie_ctxt_info_gen3_free(struct iwl_trans *trans)
{
    struct iwl_trans_pcie *trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans);

    if (!trans_pcie->ctxt_info_gen3)
		return;

    dma_free_coherent(trans->dev, sizeof(*trans_pcie->ctxt_info_gen3),
			  trans_pcie->ctxt_info_gen3,
			  trans_pcie->ctxt_info_dma_addr);
    trans_pcie->ctxt_info_dma_addr = 0;
    trans_pcie->ctxt_info_gen3 = NULL;

    iwl_pcie_ctxt_info_free_fw_img(trans);

    dma_free_coherent(trans->dev, sizeof(*trans_pcie->prph_scratch),
			  trans_pcie->prph_scratch,
			  trans_pcie->prph_scratch_dma_addr);
    trans_pcie->prph_scratch_dma_addr = 0;
    trans_pcie->prph_scratch = NULL;

    dma_free_coherent(trans->dev, sizeof(*trans_pcie->prph_info),
			  trans_pcie->prph_info,
			  trans_pcie->prph_info_dma_addr);
    trans_pcie->prph_info_dma_addr = 0;
    trans_pcie->prph_info = NULL;
}