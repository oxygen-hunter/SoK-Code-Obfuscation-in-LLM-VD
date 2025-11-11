"""
TCP Illinois congestion control.
Home page:
	http://www.ews.uiuc.edu/~shaoliu/tcpillinois/index.html

The algorithm is described in:
"TCP-Illinois: A Loss and Delay-Based Congestion Control Algorithm
 for High-Speed Networks"
http://www.ifp.illinois.edu/~srikant/Papers/liubassri06perf.pdf

Implemented from description in paper and ns-2 simulation.
Copyright (C) 2007 Stephen Hemminger <shemminger@linux-foundation.org>
"""

import ctypes
from ctypes import c_uint32, c_uint64, c_uint16, c_uint8, c_int
from ctypes import Structure, CDLL, POINTER, byref

lib = CDLL('./tcp_illinois.so')  # Assume the C part is compiled into a shared library

ALPHA_SHIFT = 7
ALPHA_SCALE = 1 << ALPHA_SHIFT
ALPHA_MIN = (3 * ALPHA_SCALE) // 10  # ~0.3
ALPHA_MAX = 10 * ALPHA_SCALE  # 10.0
ALPHA_BASE = ALPHA_SCALE  # 1.0
U32_MAX = 0xFFFFFFFF
RTT_MAX = U32_MAX // ALPHA_MAX  # 3.3 secs

BETA_SHIFT = 6
BETA_SCALE = 1 << BETA_SHIFT
BETA_MIN = BETA_SCALE // 8  # 0.125
BETA_MAX = BETA_SCALE // 2  # 0.5
BETA_BASE = BETA_MAX

win_thresh = 15
theta = 5

class Illinois(Structure):
    _fields_ = [
        ('sum_rtt', c_uint64),
        ('cnt_rtt', c_uint16),
        ('base_rtt', c_uint32),
        ('max_rtt', c_uint32),
        ('end_seq', c_uint32),
        ('alpha', c_uint32),
        ('beta', c_uint32),
        ('acked', c_uint16),
        ('rtt_above', c_uint8),
        ('rtt_low', c_uint8),
    ]

def rtt_reset(sk):
    tp = lib.tcp_sk(sk)
    ca = lib.inet_csk_ca(sk)

    ca.end_seq = tp.snd_nxt
    ca.cnt_rtt = 0
    ca.sum_rtt = 0

def tcp_illinois_init(sk):
    ca = lib.inet_csk_ca(sk)

    ca.alpha = ALPHA_MAX
    ca.beta = BETA_BASE
    ca.base_rtt = 0x7fffffff
    ca.max_rtt = 0

    ca.acked = 0
    ca.rtt_low = 0
    ca.rtt_above = 0

    rtt_reset(sk)

def tcp_illinois_acked(sk, pkts_acked, rtt):
    ca = lib.inet_csk_ca(sk)

    ca.acked = pkts_acked

    if rtt < 0:
        return

    if rtt > RTT_MAX:
        rtt = RTT_MAX

    if ca.base_rtt > rtt:
        ca.base_rtt = rtt

    if ca.max_rtt < rtt:
        ca.max_rtt = rtt

    ca.cnt_rtt += 1
    ca.sum_rtt += rtt

def max_delay(ca):
    return ca.max_rtt - ca.base_rtt

def avg_delay(ca):
    t = ca.sum_rtt
    lib.do_div(byref(t), ca.cnt_rtt)
    return t.value - ca.base_rtt

def alpha(ca, da, dm):
    d1 = dm // 100

    if da <= d1:
        if not ca.rtt_above:
            return ALPHA_MAX

        if ca.rtt_low + 1 < theta:
            ca.rtt_low += 1
            return ca.alpha

        ca.rtt_low = 0
        ca.rtt_above = 0
        return ALPHA_MAX

    ca.rtt_above = 1

    dm -= d1
    da -= d1
    return (dm * ALPHA_MAX) // (dm + (da * (ALPHA_MAX - ALPHA_MIN)) // ALPHA_MIN)

def beta(da, dm):
    d2 = dm // 10
    if da <= d2:
        return BETA_MIN

    d3 = (8 * dm) // 10
    if da >= d3 or d3 <= d2:
        return BETA_MAX

    return (BETA_MIN * d3 - BETA_MAX * d2 + (BETA_MAX - BETA_MIN) * da) // (d3 - d2)

def update_params(sk):
    tp = lib.tcp_sk(sk)
    ca = lib.inet_csk_ca(sk)

    if tp.snd_cwnd < win_thresh:
        ca.alpha = ALPHA_BASE
        ca.beta = BETA_BASE
    elif ca.cnt_rtt > 0:
        dm = max_delay(ca)
        da = avg_delay(ca)

        ca.alpha = alpha(ca, da, dm)
        ca.beta = beta(da, dm)

    rtt_reset(sk)

def tcp_illinois_state(sk, new_state):
    ca = lib.inet_csk_ca(sk)

    if new_state == lib.TCP_CA_Loss:
        ca.alpha = ALPHA_BASE
        ca.beta = BETA_BASE
        ca.rtt_low = 0
        ca.rtt_above = 0
        rtt_reset(sk)

def tcp_illinois_cong_avoid(sk, ack, in_flight):
    tp = lib.tcp_sk(sk)
    ca = lib.inet_csk_ca(sk)

    if lib.after(ack, ca.end_seq):
        update_params(sk)

    if not lib.tcp_is_cwnd_limited(sk, in_flight):
        return

    if tp.snd_cwnd <= tp.snd_ssthresh:
        lib.tcp_slow_start(tp)
    else:
        tp.snd_cwnd_cnt += ca.acked
        ca.acked = 1

        delta = (tp.snd_cwnd_cnt * ca.alpha) >> ALPHA_SHIFT
        if delta >= tp.snd_cwnd:
            tp.snd_cwnd = min(tp.snd_cwnd + delta // tp.snd_cwnd,
                              tp.snd_cwnd_clamp)
            tp.snd_cwnd_cnt = 0

def tcp_illinois_ssthresh(sk):
    tp = lib.tcp_sk(sk)
    ca = lib.inet_csk_ca(sk)

    return max(tp.snd_cwnd - ((tp.snd_cwnd * ca.beta) >> BETA_SHIFT), 2)

def tcp_illinois_info(sk, ext, skb):
    ca = lib.inet_csk_ca(sk)

    if ext & (1 << (lib.INET_DIAG_VEGASINFO - 1)):
        info = lib.tcpvegas_info()
        info.tcpv_enabled = 1
        info.tcpv_rttcnt = ca.cnt_rtt
        info.tcpv_minrtt = ca.base_rtt
        t = ca.sum_rtt

        lib.do_div(byref(t), ca.cnt_rtt)
        info.tcpv_rtt = t

        lib.nla_put(skb, lib.INET_DIAG_VEGASINFO, ctypes.sizeof(info), byref(info))

tcp_illinois = lib.tcp_congestion_ops()
tcp_illinois.flags = lib.TCP_CONG_RTT_STAMP
tcp_illinois.init = lib.tcp_illinois_init
tcp_illinois.ssthresh = lib.tcp_illinois_ssthresh
tcp_illinois.min_cwnd = lib.tcp_reno_min_cwnd
tcp_illinois.cong_avoid = lib.tcp_illinois_cong_avoid
tcp_illinois.set_state = lib.tcp_illinois_state
tcp_illinois.get_info = lib.tcp_illinois_info
tcp_illinois.pkts_acked = lib.tcp_illinois_acked
tcp_illinois.owner = lib.THIS_MODULE
tcp_illinois.name = b"illinois"

def tcp_illinois_register():
    lib.BUILD_BUG_ON(ctypes.sizeof(Illinois) > lib.ICSK_CA_PRIV_SIZE)
    return lib.tcp_register_congestion_control(byref(tcp_illinois))

def tcp_illinois_unregister():
    lib.tcp_unregister_congestion_control(byref(tcp_illinois))

lib.module_init(tcp_illinois_register)
lib.module_exit(tcp_illinois_unregister)