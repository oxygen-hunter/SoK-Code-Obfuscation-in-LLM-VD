import logging
from paramiko.py3compat import byte_chr, PY2, bytes_types, text_type, long

msg_values = [1, 2, 3, 4, 5, 6, 20, 21, 50, 51, 52, 53, 60, 60, 61, 60, 61, 63, 64, 65, 66, 79, 80, 81, 82, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100]
MSG_DISCONNECT, MSG_IGNORE, MSG_UNIMPLEMENTED, MSG_DEBUG, MSG_SERVICE_REQUEST, MSG_SERVICE_ACCEPT, MSG_KEXINIT, MSG_NEWKEYS, MSG_USERAUTH_REQUEST, MSG_USERAUTH_FAILURE, MSG_USERAUTH_SUCCESS, MSG_USERAUTH_BANNER, MSG_USERAUTH_PK_OK, MSG_USERAUTH_INFO_REQUEST, MSG_USERAUTH_INFO_RESPONSE, MSG_USERAUTH_GSSAPI_RESPONSE, MSG_USERAUTH_GSSAPI_TOKEN, MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE, MSG_USERAUTH_GSSAPI_ERROR, MSG_USERAUTH_GSSAPI_ERRTOK, MSG_USERAUTH_GSSAPI_MIC, HIGHEST_USERAUTH_MESSAGE_ID, MSG_GLOBAL_REQUEST, MSG_REQUEST_SUCCESS, MSG_REQUEST_FAILURE, MSG_CHANNEL_OPEN, MSG_CHANNEL_OPEN_SUCCESS, MSG_CHANNEL_OPEN_FAILURE, MSG_CHANNEL_WINDOW_ADJUST, MSG_CHANNEL_DATA, MSG_CHANNEL_EXTENDED_DATA, MSG_CHANNEL_EOF, MSG_CHANNEL_CLOSE, MSG_CHANNEL_REQUEST, MSG_CHANNEL_SUCCESS, MSG_CHANNEL_FAILURE = msg_values

cMSG_vals = [byte_chr(x) for x in msg_values]
cMSG_DISCONNECT, cMSG_IGNORE, cMSG_UNIMPLEMENTED, cMSG_DEBUG, cMSG_SERVICE_REQUEST, cMSG_SERVICE_ACCEPT, cMSG_KEXINIT, cMSG_NEWKEYS, cMSG_USERAUTH_REQUEST, cMSG_USERAUTH_FAILURE, cMSG_USERAUTH_SUCCESS, cMSG_USERAUTH_BANNER, cMSG_USERAUTH_PK_OK, cMSG_USERAUTH_INFO_REQUEST, cMSG_USERAUTH_INFO_RESPONSE, cMSG_USERAUTH_GSSAPI_RESPONSE, cMSG_USERAUTH_GSSAPI_TOKEN, cMSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE, cMSG_USERAUTH_GSSAPI_ERROR, cMSG_USERAUTH_GSSAPI_ERRTOK, cMSG_USERAUTH_GSSAPI_MIC, _, cMSG_GLOBAL_REQUEST, cMSG_REQUEST_SUCCESS, cMSG_REQUEST_FAILURE, cMSG_CHANNEL_OPEN, cMSG_CHANNEL_OPEN_SUCCESS, cMSG_CHANNEL_OPEN_FAILURE, cMSG_CHANNEL_WINDOW_ADJUST, cMSG_CHANNEL_DATA, cMSG_CHANNEL_EXTENDED_DATA, cMSG_CHANNEL_EOF, cMSG_CHANNEL_CLOSE, cMSG_CHANNEL_REQUEST, cMSG_CHANNEL_SUCCESS, cMSG_CHANNEL_FAILURE = cMSG_vals

MSG_NAMES = {
    msg_values[0]: 'disconnect',
    msg_values[1]: 'ignore',
    msg_values[2]: 'unimplemented',
    msg_values[3]: 'debug',
    msg_values[4]: 'service-request',
    msg_values[5]: 'service-accept',
    msg_values[6]: 'kexinit',
    msg_values[7]: 'newkeys',
    30: 'kex30',
    31: 'kex31',
    32: 'kex32',
    33: 'kex33',
    34: 'kex34',
    40: 'kex40',
    41: 'kex41',
    msg_values[8]: 'userauth-request',
    msg_values[9]: 'userauth-failure',
    msg_values[10]: 'userauth-success',
    msg_values[11]: 'userauth--banner',
    msg_values[12]: 'userauth-60(pk-ok/info-request)',
    msg_values[13]: 'userauth-info-response',
    msg_values[22]: 'global-request',
    msg_values[23]: 'request-success',
    msg_values[24]: 'request-failure',
    msg_values[25]: 'channel-open',
    msg_values[26]: 'channel-open-success',
    msg_values[27]: 'channel-open-failure',
    msg_values[28]: 'channel-window-adjust',
    msg_values[29]: 'channel-data',
    msg_values[30]: 'channel-extended-data',
    msg_values[31]: 'channel-eof',
    msg_values[32]: 'channel-close',
    msg_values[33]: 'channel-request',
    msg_values[34]: 'channel-success',
    msg_values[35]: 'channel-failure',
    msg_values[14]: 'userauth-gssapi-response',
    msg_values[15]: 'userauth-gssapi-token',
    msg_values[16]: 'userauth-gssapi-exchange-complete',
    msg_values[17]: 'userauth-gssapi-error',
    msg_values[18]: 'userauth-gssapi-error-token',
    msg_values[19]: 'userauth-gssapi-mic'
}

auth_codes = [0, 1, 2]
AUTH_SUCCESSFUL, AUTH_PARTIALLY_SUCCESSFUL, AUTH_FAILED = auth_codes

open_fail_codes = [0, 1, 2, 3, 4]
(OPEN_SUCCEEDED, OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED, OPEN_FAILED_CONNECT_FAILED, OPEN_FAILED_UNKNOWN_CHANNEL_TYPE, OPEN_FAILED_RESOURCE_SHORTAGE) = open_fail_codes

CONNECTION_FAILED_CODE = {1: 'Administratively prohibited', 2: 'Connect failed', 3: 'Unknown channel type', 4: 'Resource shortage'}

disconnect_codes = [7, 13, 14]
DISCONNECT_SERVICE_NOT_AVAILABLE, DISCONNECT_AUTH_CANCELLED_BY_USER, DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE = disconnect_codes

byte_values = [0, 1, 4, 0xff, 13, 10]
zero_byte, one_byte, four_byte, max_byte, cr_byte, linefeed_byte = [byte_chr(x) for x in byte_values]
crlf = cr_byte + linefeed_byte

if PY2:
    cr_byte_value, linefeed_byte_value = cr_byte, linefeed_byte
else:
    cr_byte_value, linefeed_byte_value = 13, 10

def asbytes(s):
    if isinstance(s, bytes_types):
        return s
    if isinstance(s, text_type):
        return s.encode("utf-8")
    asbytes = getattr(s, "asbytes", None)
    if asbytes is not None:
        return asbytes()
    return s

long_values = [0xffffffff, 0x80000000]
ffffffff, x80000000 = [long(x) for x in long_values]
octal_values = [438, 432, 420, 384, 511, 448, 56]
o666, o660, o644, o600, o777, o700, o70 = octal_values

log_levels = [logging.DEBUG, logging.INFO, logging.WARNING, logging.ERROR, logging.CRITICAL]
DEBUG, INFO, WARNING, ERROR, CRITICAL = log_levels

io_sleep = 0.01

DEFAULT_WINDOW_SIZE = 64 * 2 ** 15
DEFAULT_MAX_PACKET_SIZE = 2 ** 15

MIN_WINDOW_SIZE = 2 ** 15
MIN_PACKET_SIZE = 2 ** 12
MAX_WINDOW_SIZE = 2 ** 32 - 1