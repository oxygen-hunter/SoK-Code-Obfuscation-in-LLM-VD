# -*- coding: utf-8 -*-
import ctypes
from ctypes import c_void_p, c_char_p, c_bool, c_ulong, c_uint16, Structure, POINTER

# Load the shared library
lib = ctypes.CDLL('./wmi_lib.so')

# Define the C structures and functions
class WmiCmdHdr(Structure):
    _fields_ = [("command_id", c_uint16),
                ("seq_no", c_uint16)]

class WmiEventSwba(Structure):
    _fields_ = []

class Wmi(Structure):
    _fields_ = [("drv_priv", c_void_p),
                ("stopped", c_bool),
                ("wmi_event_queue", c_void_p),
                ("wmi_lock", c_void_p),
                ("event_lock", c_void_p),
                ("op_mutex", c_void_p),
                ("multi_write_mutex", c_void_p),
                ("multi_rmw_mutex", c_void_p),
                ("cmd_wait", c_void_p),
                ("pending_tx_events", c_void_p),
                ("wmi_event_tasklet", c_void_p),
                ("cmd_rsp_buf", c_void_p),
                ("cmd_rsp_len", c_ulong),
                ("htc", c_void_p),
                ("ctrl_epid", c_uint16),
                ("tx_seq_id", c_uint16),
                ("last_seq_id", c_uint16)]

lib.ath9k_init_wmi.restype = POINTER(Wmi)
lib.ath9k_init_wmi.argtypes = [c_void_p]

lib.ath9k_deinit_wmi.argtypes = [c_void_p]

lib.ath9k_wmi_event_drain.argtypes = [c_void_p]

lib.ath9k_wmi_event_tasklet.argtypes = [c_ulong]

lib.ath9k_fatal_work.argtypes = [c_void_p]

lib.ath9k_wmi_rsp_callback.argtypes = [POINTER(Wmi), c_void_p]

lib.ath9k_wmi_ctrl_rx.argtypes = [c_void_p, c_void_p, c_uint16]

lib.ath9k_wmi_ctrl_tx.argtypes = [c_void_p, c_void_p, c_uint16, c_bool]

lib.ath9k_wmi_connect.argtypes = [c_void_p, POINTER(Wmi), POINTER(c_uint16)]
lib.ath9k_wmi_connect.restype = c_int

lib.ath9k_wmi_cmd_issue.argtypes = [POINTER(Wmi), c_void_p, c_uint16, c_uint16]
lib.ath9k_wmi_cmd_issue.restype = c_int

lib.ath9k_wmi_cmd.argtypes = [POINTER(Wmi), c_uint16, c_void_p, c_ulong, c_void_p, c_ulong, c_ulong]
lib.ath9k_wmi_cmd.restype = c_int

def wmi_cmd_to_name(wmi_cmd):
    names = {
        1: "WMI_ECHO_CMDID",
        2: "WMI_ACCESS_MEMORY_CMDID",
        3: "WMI_GET_FW_VERSION",
        4: "WMI_DISABLE_INTR_CMDID",
        5: "WMI_ENABLE_INTR_CMDID",
        6: "WMI_ATH_INIT_CMDID",
        7: "WMI_ABORT_TXQ_CMDID",
        8: "WMI_STOP_TX_DMA_CMDID",
        9: "WMI_ABORT_TX_DMA_CMDID",
        10: "WMI_DRAIN_TXQ_CMDID",
        11: "WMI_DRAIN_TXQ_ALL_CMDID",
        12: "WMI_START_RECV_CMDID",
        13: "WMI_STOP_RECV_CMDID",
        14: "WMI_FLUSH_RECV_CMDID",
        15: "WMI_SET_MODE_CMDID",
        16: "WMI_NODE_CREATE_CMDID",
        17: "WMI_NODE_REMOVE_CMDID",
        18: "WMI_VAP_REMOVE_CMDID",
        19: "WMI_VAP_CREATE_CMDID",
        20: "WMI_REG_READ_CMDID",
        21: "WMI_REG_WRITE_CMDID",
        22: "WMI_REG_RMW_CMDID",
        23: "WMI_RC_STATE_CHANGE_CMDID",
        24: "WMI_RC_RATE_UPDATE_CMDID",
        25: "WMI_TARGET_IC_UPDATE_CMDID",
        26: "WMI_TX_AGGR_ENABLE_CMDID",
        27: "WMI_TGT_DETACH_CMDID",
        28: "WMI_NODE_UPDATE_CMDID",
        29: "WMI_INT_STATS_CMDID",
        30: "WMI_TX_STATS_CMDID",
        31: "WMI_RX_STATS_CMDID",
        32: "WMI_BITRATE_MASK_CMDID",
    }
    return names.get(wmi_cmd, "Bogus")

# Example usage:
# priv = c_void_p() # Initialize with actual data
# wmi_instance = lib.ath9k_init_wmi(priv)
# lib.ath9k_deinit_wmi(priv)