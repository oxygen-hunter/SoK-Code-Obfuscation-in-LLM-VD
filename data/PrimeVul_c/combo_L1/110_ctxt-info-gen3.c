#include "iwl-trans.h"
#include "iwl-fh.h"
#include "iwl-context-info-gen3.h"
#include "internal.h"
#include "iwl-prph.h"

int OX3A4B6F90(struct OX1C2D3E4F *OX5E6F7A8B,
               const struct OX9C8D7E6F *OX4B5C6D7E)
{
    struct OX2D3E4F5A *OX6B7C8D9E = IWL_TRANS_GET_PCIE_TRANS(OX5E6F7A8B);
    struct OX7A8B9C0D *OX1B2C3D4E;
    struct OX3E4F5A6B *OX5D6E7F8A;
    struct OX8B9C0D1E *OX2A3B4C5D;
    void *OX7F8A9B0C;
    u32 OX9D0E1F2A = 0;
    int OX4C5D6E7F;
    int OX0A1B2C3D = max_t(u32, IWL_CMD_QUEUE_SIZE, OX5E6F7A8B->cfg->min_txq_size);

    OX5D6E7F8A = dma_alloc_coherent(OX5E6F7A8B->dev, sizeof(*OX5D6E7F8A),
                                    &OX6B7C8D9E->prph_scratch_dma_addr,
                                    GFP_KERNEL);
    if (!OX5D6E7F8A)
        return -ENOMEM;

    OX5D6E7F8A->ctrl_cfg.version.version = 0;
    OX5D6E7F8A->ctrl_cfg.version.mac_id =
        cpu_to_le16((u16)iwl_read32(OX5E6F7A8B, CSR_HW_REV));
    OX5D6E7F8A->ctrl_cfg.version.size = cpu_to_le16(sizeof(*OX5D6E7F8A) / 4);

    OX9D0E1F2A = IWL_PRPH_SCRATCH_RB_SIZE_4K |
                 IWL_PRPH_SCRATCH_MTR_MODE |
                 (IWL_PRPH_MTR_FORMAT_256B & IWL_PRPH_SCRATCH_MTR_FORMAT) |
                 IWL_PRPH_SCRATCH_EARLY_DEBUG_EN |
                 IWL_PRPH_SCRATCH_EDBG_DEST_DRAM;
    OX5D6E7F8A->ctrl_cfg.control.control_flags = cpu_to_le32(OX9D0E1F2A);

    OX5D6E7F8A->ctrl_cfg.rbd_cfg.free_rbd_addr =
        cpu_to_le64(OX6B7C8D9E->rxq->bd_dma);

    if (!iwl_trans_dbg_ini_valid(OX5E6F7A8B))
        iwl_pcie_alloc_fw_monitor(OX5E6F7A8B, 0);
    if (OX5E6F7A8B->dbg.num_blocks) {
        OX5D6E7F8A->ctrl_cfg.hwm_cfg.hwm_base_addr =
            cpu_to_le64(OX5E6F7A8B->dbg.fw_mon[0].physical);
        OX5D6E7F8A->ctrl_cfg.hwm_cfg.hwm_size =
            cpu_to_le32(OX5E6F7A8B->dbg.fw_mon[0].size);
    }

    OX4C5D6E7F = iwl_pcie_init_fw_sec(OX5E6F7A8B, OX4B5C6D7E, &OX5D6E7F8A->dram);
    if (OX4C5D6E7F) {
        dma_free_coherent(OX5E6F7A8B->dev,
                          sizeof(*OX5D6E7F8A),
                          OX5D6E7F8A,
                          OX6B7C8D9E->prph_scratch_dma_addr);
        return OX4C5D6E7F;
    }

    OX2A3B4C5D = dma_alloc_coherent(OX5E6F7A8B->dev, sizeof(*OX2A3B4C5D),
                                    &OX6B7C8D9E->prph_info_dma_addr,
                                    GFP_KERNEL);
    if (!OX2A3B4C5D)
        return -ENOMEM;

    OX1B2C3D4E = dma_alloc_coherent(OX5E6F7A8B->dev,
                                    sizeof(*OX1B2C3D4E),
                                    &OX6B7C8D9E->ctxt_info_dma_addr,
                                    GFP_KERNEL);
    if (!OX1B2C3D4E)
        return -ENOMEM;

    OX1B2C3D4E->prph_info_base_addr =
        cpu_to_le64(OX6B7C8D9E->prph_info_dma_addr);
    OX1B2C3D4E->prph_scratch_base_addr =
        cpu_to_le64(OX6B7C8D9E->prph_scratch_dma_addr);
    OX1B2C3D4E->prph_scratch_size =
        cpu_to_le32(sizeof(*OX5D6E7F8A));
    OX1B2C3D4E->cr_head_idx_arr_base_addr =
        cpu_to_le64(OX6B7C8D9E->rxq->rb_stts_dma);
    OX1B2C3D4E->tr_tail_idx_arr_base_addr =
        cpu_to_le64(OX6B7C8D9E->rxq->tr_tail_dma);
    OX1B2C3D4E->cr_tail_idx_arr_base_addr =
        cpu_to_le64(OX6B7C8D9E->rxq->cr_tail_dma);
    OX1B2C3D4E->cr_idx_arr_size =
        cpu_to_le16(IWL_NUM_OF_COMPLETION_RINGS);
    OX1B2C3D4E->tr_idx_arr_size =
        cpu_to_le16(IWL_NUM_OF_TRANSFER_RINGS);
    OX1B2C3D4E->mtr_base_addr =
        cpu_to_le64(OX6B7C8D9E->txq[OX6B7C8D9E->cmd_queue]->dma_addr);
    OX1B2C3D4E->mcr_base_addr =
        cpu_to_le64(OX6B7C8D9E->rxq->used_bd_dma);
    OX1B2C3D4E->mtr_size =
        cpu_to_le16(TFD_QUEUE_CB_SIZE(OX0A1B2C3D));
    OX1B2C3D4E->mcr_size =
        cpu_to_le16(RX_QUEUE_CB_SIZE(MQ_RX_TABLE_SIZE));

    OX6B7C8D9E->ctxt_info_gen3 = OX1B2C3D4E;
    OX6B7C8D9E->prph_info = OX2A3B4C5D;
    OX6B7C8D9E->prph_scratch = OX5D6E7F8A;

    OX7F8A9B0C = dma_alloc_coherent(OX5E6F7A8B->dev, OX5E6F7A8B->iml_len,
                                    &OX6B7C8D9E->iml_dma_addr, GFP_KERNEL);
    if (!OX7F8A9B0C)
        return -ENOMEM;

    memcpy(OX7F8A9B0C, OX5E6F7A8B->iml, OX5E6F7A8B->iml_len);

    iwl_enable_fw_load_int_ctx_info(OX5E6F7A8B);

    iwl_write64(OX5E6F7A8B, CSR_CTXT_INFO_ADDR,
                OX6B7C8D9E->ctxt_info_dma_addr);
    iwl_write64(OX5E6F7A8B, CSR_IML_DATA_ADDR,
                OX6B7C8D9E->iml_dma_addr);
    iwl_write32(OX5E6F7A8B, CSR_IML_SIZE_ADDR, OX5E6F7A8B->iml_len);

    iwl_set_bit(OX5E6F7A8B, CSR_CTXT_INFO_BOOT_CTRL,
                CSR_AUTO_FUNC_BOOT_ENA);
    if (OX5E6F7A8B->trans_cfg->device_family >= IWL_DEVICE_FAMILY_AX210)
        iwl_write_umac_prph(OX5E6F7A8B, UREG_CPU_INIT_RUN, 1);
    else
        iwl_set_bit(OX5E6F7A8B, CSR_GP_CNTRL, CSR_AUTO_FUNC_INIT);

    return 0;
}

void OX6C7D8E9F(struct OX1C2D3E4F *OX5E6F7A8B)
{
    struct OX2D3E4F5A *OX6B7C8D9E = IWL_TRANS_GET_PCIE_TRANS(OX5E6F7A8B);

    if (!OX6B7C8D9E->ctxt_info_gen3)
        return;

    dma_free_coherent(OX5E6F7A8B->dev, sizeof(*OX6B7C8D9E->ctxt_info_gen3),
                      OX6B7C8D9E->ctxt_info_gen3,
                      OX6B7C8D9E->ctxt_info_dma_addr);
    OX6B7C8D9E->ctxt_info_dma_addr = 0;
    OX6B7C8D9E->ctxt_info_gen3 = NULL;

    iwl_pcie_ctxt_info_free_fw_img(OX5E6F7A8B);

    dma_free_coherent(OX5E6F7A8B->dev, sizeof(*OX6B7C8D9E->prph_scratch),
                      OX6B7C8D9E->prph_scratch,
                      OX6B7C8D9E->prph_scratch_dma_addr);
    OX6B7C8D9E->prph_scratch_dma_addr = 0;
    OX6B7C8D9E->prph_scratch = NULL;

    dma_free_coherent(OX5E6F7A8B->dev, sizeof(*OX6B7C8D9E->prph_info),
                      OX6B7C8D9E->prph_info,
                      OX6B7C8D9E->prph_info_dma_addr);
    OX6B7C8D9E->prph_info_dma_addr = 0;
    OX6B7C8D9E->prph_info = NULL;
}