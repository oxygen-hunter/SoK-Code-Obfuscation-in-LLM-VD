import logging
from paramiko.py3compat import byte_chr, PY2, bytes_types, text_type, long

def getValues():
    return {
        'msg_disconn': range(1, 7),
        'msg_kex': range(20, 22),
        'msg_userauth': range(50, 54),
        'msg_userauth_gssapi': range(63, 67),
        'highest_userauth_id': 79,
        'msg_global': range(80, 83),
        'msg_channel': range(90, 101),
        'auth_codes': range(3),
        'open_failed': range(0, 5),
        'disconnect_codes': (7, 13, 14),
        'zero': 0,
        'one': 1,
        'four': 4,
        'max_val': 0xff,
        'cr': 13,
        'lf': 10,
        'file_modes': {
            'o666': 438,
            'o660': 432,
            'o644': 420,
            'o600': 384,
            'o777': 511,
            'o700': 448,
            'o70': 56
        },
        'logging_levels': {
            'debug': logging.DEBUG,
            'info': logging.INFO,
            'warning': logging.WARNING,
            'error': logging.ERROR,
            'critical': logging.CRITICAL
        },
        'io_sleep': 0.01,
        'default_window_size': 64 * 2 ** 15,
        'default_max_packet_size': 2 ** 15,
        'min_window_size': 2 ** 15,
        'min_packet_size': 2 ** 12,
        'max_window_size': 2 ** 32 - 1
    }

def byteValues():
    values = getValues()
    return {
        'c_msg_disconn': byte_chr(next(values['msg_disconn'])),
        'c_msg_ignore': byte_chr(next(values['msg_disconn'])),
        'c_msg_unimplemented': byte_chr(next(values['msg_disconn'])),
        'c_msg_debug': byte_chr(next(values['msg_disconn'])),
        'c_msg_service_request': byte_chr(next(values['msg_disconn'])),
        'c_msg_service_accept': byte_chr(next(values['msg_disconn'])),
        'c_msg_kexinit': byte_chr(next(values['msg_kex'])),
        'c_msg_newkeys': byte_chr(next(values['msg_kex'])),
        'c_msg_userauth_request': byte_chr(next(values['msg_userauth'])),
        'c_msg_userauth_failure': byte_chr(next(values['msg_userauth'])),
        'c_msg_userauth_success': byte_chr(next(values['msg_userauth'])),
        'c_msg_userauth_banner': byte_chr(next(values['msg_userauth'])),
        'c_msg_userauth_pk_ok': byte_chr(values['msg_userauth'][3]),
        'c_msg_userauth_info_request': byte_chr(values['msg_userauth'][3]),
        'c_msg_userauth_info_response': byte_chr(values['msg_userauth'][3] + 1),
        'c_msg_userauth_gssapi_response': byte_chr(values['msg_userauth'][3]),
        'c_msg_userauth_gssapi_token': byte_chr(values['msg_userauth'][3] + 1),
        'c_msg_userauth_gssapi_exchange_complete': byte_chr(next(values['msg_userauth_gssapi'])),
        'c_msg_userauth_gssapi_error': byte_chr(next(values['msg_userauth_gssapi'])),
        'c_msg_userauth_gssapi_ertok': byte_chr(next(values['msg_userauth_gssapi'])),
        'c_msg_userauth_gssapi_mic': byte_chr(next(values['msg_userauth_gssapi'])),
        'c_msg_global_request': byte_chr(next(values['msg_global'])),
        'c_msg_request_success': byte_chr(next(values['msg_global'])),
        'c_msg_request_failure': byte_chr(next(values['msg_global'])),
        'c_msg_channel_open': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_open_success': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_open_failure': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_window_adjust': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_data': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_extended_data': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_eof': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_close': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_request': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_success': byte_chr(next(values['msg_channel'])),
        'c_msg_channel_failure': byte_chr(next(values['msg_channel']))
    }

def asbytes(s):
    if isinstance(s, bytes_types):
        return s
    if isinstance(s, text_type):
        return s.encode("utf-8")
    asbytes = getattr(s, "asbytes", None)
    if asbytes is not None:
        return asbytes()
    return s

byte_values = byteValues()
cr_byte = byte_chr(getValues()['cr'])
linefeed_byte = byte_chr(getValues()['lf'])
crlf = cr_byte + linefeed_byte

if PY2:
    cr_byte_value = cr_byte
    linefeed_byte_value = linefeed_byte
else:
    cr_byte_value = getValues()['cr']
    linefeed_byte_value = getValues()['lf']