import logging
from paramiko.py3compat import byte_chr, PY2, bytes_types, text_type, long

def rec_range(start, end, step=1, acc=None):
    if acc is None:
        acc = []
    if start == end:
        return acc
    return rec_range(start + step, end, step, acc + [start])

def rec_byte_chr_range(start, end, step=1, acc=None):
    if acc is None:
        acc = []
    if start == end:
        return acc
    return rec_byte_chr_range(start + step, end, step, acc + [byte_chr(start)])

MSG_DISCONNECT, MSG_IGNORE, MSG_UNIMPLEMENTED, MSG_DEBUG, \
    MSG_SERVICE_REQUEST, MSG_SERVICE_ACCEPT = rec_range(1, 7)
MSG_KEXINIT, MSG_NEWKEYS = rec_range(20, 22)
MSG_USERAUTH_REQUEST, MSG_USERAUTH_FAILURE, MSG_USERAUTH_SUCCESS, \
    MSG_USERAUTH_BANNER = rec_range(50, 54)
MSG_USERAUTH_PK_OK = 60
MSG_USERAUTH_INFO_REQUEST, MSG_USERAUTH_INFO_RESPONSE = rec_range(60, 62)
MSG_USERAUTH_GSSAPI_RESPONSE, MSG_USERAUTH_GSSAPI_TOKEN = rec_range(60, 62)
MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE, MSG_USERAUTH_GSSAPI_ERROR,\
    MSG_USERAUTH_GSSAPI_ERRTOK, MSG_USERAUTH_GSSAPI_MIC = rec_range(63, 67)
HIGHEST_USERAUTH_MESSAGE_ID = 79
MSG_GLOBAL_REQUEST, MSG_REQUEST_SUCCESS, MSG_REQUEST_FAILURE = rec_range(80, 83)
MSG_CHANNEL_OPEN, MSG_CHANNEL_OPEN_SUCCESS, MSG_CHANNEL_OPEN_FAILURE, \
    MSG_CHANNEL_WINDOW_ADJUST, MSG_CHANNEL_DATA, MSG_CHANNEL_EXTENDED_DATA, \
    MSG_CHANNEL_EOF, MSG_CHANNEL_CLOSE, MSG_CHANNEL_REQUEST, \
    MSG_CHANNEL_SUCCESS, MSG_CHANNEL_FAILURE = rec_range(90, 101)

cMSG_DISCONNECT, cMSG_IGNORE, cMSG_UNIMPLEMENTED, cMSG_DEBUG, \
cMSG_SERVICE_REQUEST, cMSG_SERVICE_ACCEPT, cMSG_KEXINIT, cMSG_NEWKEYS, \
cMSG_USERAUTH_REQUEST, cMSG_USERAUTH_FAILURE, cMSG_USERAUTH_SUCCESS, \
cMSG_USERAUTH_BANNER, cMSG_USERAUTH_PK_OK, cMSG_USERAUTH_INFO_REQUEST, \
cMSG_USERAUTH_INFO_RESPONSE, cMSG_USERAUTH_GSSAPI_RESPONSE, \
cMSG_USERAUTH_GSSAPI_TOKEN, cMSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE, \
cMSG_USERAUTH_GSSAPI_ERROR, cMSG_USERAUTH_GSSAPI_ERRTOK, cMSG_USERAUTH_GSSAPI_MIC, \
cMSG_GLOBAL_REQUEST, cMSG_REQUEST_SUCCESS, cMSG_REQUEST_FAILURE, cMSG_CHANNEL_OPEN, \
cMSG_CHANNEL_OPEN_SUCCESS, cMSG_CHANNEL_OPEN_FAILURE, cMSG_CHANNEL_WINDOW_ADJUST, \
cMSG_CHANNEL_DATA, cMSG_CHANNEL_EXTENDED_DATA, cMSG_CHANNEL_EOF, cMSG_CHANNEL_CLOSE, \
cMSG_CHANNEL_REQUEST, cMSG_CHANNEL_SUCCESS, cMSG_CHANNEL_FAILURE = rec_byte_chr_range(1, 101)

MSG_NAMES = dict(zip(
    rec_range(1, 101), 
    ['disconnect', 'ignore', 'unimplemented', 'debug', 'service-request', 'service-accept', 
    'kexinit', 'newkeys', 'kex30', 'kex31', 'kex32', 'kex33', 'kex34', 'kex40', 'kex41', 
    'userauth-request', 'userauth-failure', 'userauth-success', 'userauth--banner', 
    'userauth-60(pk-ok/info-request)', 'userauth-info-response', 'global-request', 
    'request-success', 'request-failure', 'channel-open', 'channel-open-success', 
    'channel-open-failure', 'channel-window-adjust', 'channel-data', 
    'channel-extended-data', 'channel-eof', 'channel-close', 'channel-request', 
    'channel-success', 'channel-failure', 'userauth-gssapi-response', 
    'userauth-gssapi-token', 'userauth-gssapi-exchange-complete', 
    'userauth-gssapi-error', 'userauth-gssapi-error-token', 'userauth-gssapi-mic']
))

AUTH_SUCCESSFUL, AUTH_PARTIALLY_SUCCESSFUL, AUTH_FAILED = rec_range(0, 3)

OPEN_SUCCEEDED, OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED, OPEN_FAILED_CONNECT_FAILED, \
OPEN_FAILED_UNKNOWN_CHANNEL_TYPE, OPEN_FAILED_RESOURCE_SHORTAGE = rec_range(0, 5)

CONNECTION_FAILED_CODE = {
    1: 'Administratively prohibited',
    2: 'Connect failed',
    3: 'Unknown channel type',
    4: 'Resource shortage'
}

DISCONNECT_SERVICE_NOT_AVAILABLE, DISCONNECT_AUTH_CANCELLED_BY_USER, \
DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE = 7, 13, 14

zero_byte, one_byte, four_byte, max_byte, cr_byte, linefeed_byte = rec_byte_chr_range(0, 14, 1)

crlf = cr_byte + linefeed_byte

cr_byte_value = cr_byte if PY2 else 13
linefeed_byte_value = linefeed_byte if PY2 else 10

def asbytes(s):
    if isinstance(s, bytes_types):
        return s
    if isinstance(s, text_type):
        return s.encode("utf-8")
    asbytes = getattr(s, "asbytes", None)
    if asbytes is not None:
        return asbytes()
    return s

xffffffff = long(0xffffffff)
x80000000 = long(0x80000000)
o666 = 438
o660 = 432
o644 = 420
o600 = 384
o777 = 511
o700 = 448
o70 = 56

DEBUG = logging.DEBUG
INFO = logging.INFO
WARNING = logging.WARNING
ERROR = logging.ERROR
CRITICAL = logging.CRITICAL

io_sleep = 0.01

DEFAULT_WINDOW_SIZE = 64 * 2 ** 15
DEFAULT_MAX_PACKET_SIZE = 2 ** 15
MIN_WINDOW_SIZE = 2 ** 15
MIN_PACKET_SIZE = 2 ** 12
MAX_WINDOW_SIZE = 2 ** 32 - 1