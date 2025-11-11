import ctypes
from ctypes import c_int, c_void_p, c_uint32, c_uint64, c_uint16

lib = ctypes.CDLL('./mylibrary.so')

IWL_TRANS_GET_PCIE_TRANS = lib.IWL_TRANS_GET_PCIE_TRANS
IWL_TRANS_GET_PCIE_TRANS.restype = c_void_p

dma_alloc_coherent = lib.dma_alloc_coherent
dma_alloc_coherent.restype = c_void_p

cpu_to_le16 = lib.cpu_to_le16
cpu_to_le16.restype = c_uint16

cpu_to_le32 = lib.cpu_to_le32
cpu_to_le32.restype = c_uint32

cpu_to_le64 = lib.cpu_to_le64
cpu_to_le64.restype = c_uint64

def iwl_pcie_ctxt_info_gen3_init(trans, fw):
    trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans)
    prph_scratch = dma_alloc_coherent(trans.dev, ctypes.sizeof(lib.iwl_prph_scratch), ctypes.byref(trans_pcie.prph_scratch_dma_addr), lib.GFP_KERNEL)
    if not prph_scratch:
        return -lib.ENOMEM

    prph_sc_ctrl = ctypes.pointer(prph_scratch.ctrl_cfg)

    prph_sc_ctrl.contents.version.version = 0
    prph_sc_ctrl.contents.version.mac_id = cpu_to_le16(ctypes.c_uint16(lib.iwl_read32(trans, lib.CSR_HW_REV)))
    prph_sc_ctrl.contents.version.size = cpu_to_le16(ctypes.sizeof(lib.iwl_prph_scratch) // 4)

    control_flags = (lib.IWL_PRPH_SCRATCH_RB_SIZE_4K | lib.IWL_PRPH_SCRATCH_MTR_MODE |
                     (lib.IWL_PRPH_MTR_FORMAT_256B & lib.IWL_PRPH_SCRATCH_MTR_FORMAT) |
                     lib.IWL_PRPH_SCRATCH_EARLY_DEBUG_EN | lib.IWL_PRPH_SCRATCH_EDBG_DEST_DRAM)
    prph_sc_ctrl.contents.control.control_flags = cpu_to_le32(control_flags)

    prph_sc_ctrl.contents.rbd_cfg.free_rbd_addr = cpu_to_le64(trans_pcie.rxq.bd_dma)

    if not lib.iwl_trans_dbg_ini_valid(trans):
        lib.iwl_pcie_alloc_fw_monitor(trans, 0)
    if trans.dbg.num_blocks:
        prph_sc_ctrl.contents.hwm_cfg.hwm_base_addr = cpu_to_le64(trans.dbg.fw_mon[0].physical)
        prph_sc_ctrl.contents.hwm_cfg.hwm_size = cpu_to_le32(trans.dbg.fw_mon[0].size)

    ret = lib.iwl_pcie_init_fw_sec(trans, fw, ctypes.byref(prph_scratch.dram))
    if ret:
        lib.dma_free_coherent(trans.dev, ctypes.sizeof(lib.iwl_prph_scratch), prph_scratch, trans_pcie.prph_scratch_dma_addr)
        return ret

    prph_info = dma_alloc_coherent(trans.dev, ctypes.sizeof(lib.iwl_prph_info), ctypes.byref(trans_pcie.prph_info_dma_addr), lib.GFP_KERNEL)
    if not prph_info:
        return -lib.ENOMEM

    ctxt_info_gen3 = dma_alloc_coherent(trans.dev, ctypes.sizeof(lib.iwl_context_info_gen3), ctypes.byref(trans_pcie.ctxt_info_dma_addr), lib.GFP_KERNEL)
    if not ctxt_info_gen3:
        return -lib.ENOMEM

    ctxt_info_gen3.prph_info_base_addr = cpu_to_le64(trans_pcie.prph_info_dma_addr)
    ctxt_info_gen3.prph_scratch_base_addr = cpu_to_le64(trans_pcie.prph_scratch_dma_addr)
    ctxt_info_gen3.prph_scratch_size = cpu_to_le32(ctypes.sizeof(lib.iwl_prph_scratch))
    ctxt_info_gen3.cr_head_idx_arr_base_addr = cpu_to_le64(trans_pcie.rxq.rb_stts_dma)
    ctxt_info_gen3.tr_tail_idx_arr_base_addr = cpu_to_le64(trans_pcie.rxq.tr_tail_dma)
    ctxt_info_gen3.cr_tail_idx_arr_base_addr = cpu_to_le64(trans_pcie.rxq.cr_tail_dma)
    ctxt_info_gen3.cr_idx_arr_size = cpu_to_le16(lib.IWL_NUM_OF_COMPLETION_RINGS)
    ctxt_info_gen3.tr_idx_arr_size = cpu_to_le16(lib.IWL_NUM_OF_TRANSFER_RINGS)
    ctxt_info_gen3.mtr_base_addr = cpu_to_le64(trans_pcie.txq[trans_pcie.cmd_queue].dma_addr)
    ctxt_info_gen3.mcr_base_addr = cpu_to_le64(trans_pcie.rxq.used_bd_dma)
    ctxt_info_gen3.mtr_size = cpu_to_le16(lib.TFD_QUEUE_CB_SIZE(lib.max_t(lib.u32, lib.IWL_CMD_QUEUE_SIZE, trans.cfg.min_txq_size)))
    ctxt_info_gen3.mcr_size = cpu_to_le16(lib.RX_QUEUE_CB_SIZE(lib.MQ_RX_TABLE_SIZE))

    trans_pcie.ctxt_info_gen3 = ctxt_info_gen3
    trans_pcie.prph_info = prph_info
    trans_pcie.prph_scratch = prph_scratch

    iml_img = dma_alloc_coherent(trans.dev, trans.iml_len, ctypes.byref(trans_pcie.iml_dma_addr), lib.GFP_KERNEL)
    if not iml_img:
        return -lib.ENOMEM

    lib.memcpy(iml_img, trans.iml, trans.iml_len)

    lib.iwl_enable_fw_load_int_ctx_info(trans)

    lib.iwl_write64(trans, lib.CSR_CTXT_INFO_ADDR, trans_pcie.ctxt_info_dma_addr)
    lib.iwl_write64(trans, lib.CSR_IML_DATA_ADDR, trans_pcie.iml_dma_addr)
    lib.iwl_write32(trans, lib.CSR_IML_SIZE_ADDR, trans.iml_len)

    lib.iwl_set_bit(trans, lib.CSR_CTXT_INFO_BOOT_CTRL, lib.CSR_AUTO_FUNC_BOOT_ENA)
    if trans.trans_cfg.device_family >= lib.IWL_DEVICE_FAMILY_AX210:
        lib.iwl_write_umac_prph(trans, lib.UREG_CPU_INIT_RUN, 1)
    else:
        lib.iwl_set_bit(trans, lib.CSR_GP_CNTRL, lib.CSR_AUTO_FUNC_INIT)

    return 0

def iwl_pcie_ctxt_info_gen3_free(trans):
    trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans)

    if not trans_pcie.ctxt_info_gen3:
        return

    lib.dma_free_coherent(trans.dev, ctypes.sizeof(lib.iwl_context_info_gen3), trans_pcie.ctxt_info_gen3, trans_pcie.ctxt_info_dma_addr)
    trans_pcie.ctxt_info_dma_addr = 0
    trans_pcie.ctxt_info_gen3 = None

    lib.iwl_pcie_ctxt_info_free_fw_img(trans)

    lib.dma_free_coherent(trans.dev, ctypes.sizeof(lib.iwl_prph_scratch), trans_pcie.prph_scratch, trans_pcie.prph_scratch_dma_addr)
    trans_pcie.prph_scratch_dma_addr = 0
    trans_pcie.prph_scratch = None

    lib.dma_free_coherent(trans.dev, ctypes.sizeof(lib.iwl_prph_info), trans_pcie.prph_info, trans_pcie.prph_info_dma_addr)
    trans_pcie.prph_info_dma_addr = 0
    trans_pcie.prph_info = None