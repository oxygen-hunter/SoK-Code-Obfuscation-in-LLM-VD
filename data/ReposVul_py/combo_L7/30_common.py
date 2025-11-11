// SPDX-License-Identifier: GPL-2.0-or-later
#include <stdio.h>
#include <stdint.h>

#define MSG_DISCONNECT 1
#define MSG_IGNORE 2
#define MSG_UNIMPLEMENTED 3
#define MSG_DEBUG 4
#define MSG_SERVICE_REQUEST 5
#define MSG_SERVICE_ACCEPT 6
#define MSG_KEXINIT 20
#define MSG_NEWKEYS 21
#define MSG_USERAUTH_REQUEST 50
#define MSG_USERAUTH_FAILURE 51
#define MSG_USERAUTH_SUCCESS 52
#define MSG_USERAUTH_BANNER 53
#define MSG_USERAUTH_PK_OK 60
#define MSG_USERAUTH_INFO_REQUEST 60
#define MSG_USERAUTH_INFO_RESPONSE 61
#define MSG_USERAUTH_GSSAPI_RESPONSE 60
#define MSG_USERAUTH_GSSAPI_TOKEN 61
#define MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE 63
#define MSG_USERAUTH_GSSAPI_ERROR 64
#define MSG_USERAUTH_GSSAPI_ERRTOK 65
#define MSG_USERAUTH_GSSAPI_MIC 66
#define HIGHEST_USERAUTH_MESSAGE_ID 79
#define MSG_GLOBAL_REQUEST 80
#define MSG_REQUEST_SUCCESS 81
#define MSG_REQUEST_FAILURE 82
#define MSG_CHANNEL_OPEN 90
#define MSG_CHANNEL_OPEN_SUCCESS 91
#define MSG_CHANNEL_OPEN_FAILURE 92
#define MSG_CHANNEL_WINDOW_ADJUST 93
#define MSG_CHANNEL_DATA 94
#define MSG_CHANNEL_EXTENDED_DATA 95
#define MSG_CHANNEL_EOF 96
#define MSG_CHANNEL_CLOSE 97
#define MSG_CHANNEL_REQUEST 98
#define MSG_CHANNEL_SUCCESS 99
#define MSG_CHANNEL_FAILURE 100

inline uint8_t byte_chr(int msg) {
    return (uint8_t) msg;
}

#define cMSG_DISCONNECT byte_chr(MSG_DISCONNECT)
#define cMSG_IGNORE byte_chr(MSG_IGNORE)
#define cMSG_UNIMPLEMENTED byte_chr(MSG_UNIMPLEMENTED)
#define cMSG_DEBUG byte_chr(MSG_DEBUG)
#define cMSG_SERVICE_REQUEST byte_chr(MSG_SERVICE_REQUEST)
#define cMSG_SERVICE_ACCEPT byte_chr(MSG_SERVICE_ACCEPT)
#define cMSG_KEXINIT byte_chr(MSG_KEXINIT)
#define cMSG_NEWKEYS byte_chr(MSG_NEWKEYS)
#define cMSG_USERAUTH_REQUEST byte_chr(MSG_USERAUTH_REQUEST)
#define cMSG_USERAUTH_FAILURE byte_chr(MSG_USERAUTH_FAILURE)
#define cMSG_USERAUTH_SUCCESS byte_chr(MSG_USERAUTH_SUCCESS)
#define cMSG_USERAUTH_BANNER byte_chr(MSG_USERAUTH_BANNER)
#define cMSG_USERAUTH_PK_OK byte_chr(MSG_USERAUTH_PK_OK)
#define cMSG_USERAUTH_INFO_REQUEST byte_chr(MSG_USERAUTH_INFO_REQUEST)
#define cMSG_USERAUTH_INFO_RESPONSE byte_chr(MSG_USERAUTH_INFO_RESPONSE)
#define cMSG_USERAUTH_GSSAPI_RESPONSE byte_chr(MSG_USERAUTH_GSSAPI_RESPONSE)
#define cMSG_USERAUTH_GSSAPI_TOKEN byte_chr(MSG_USERAUTH_GSSAPI_TOKEN)
#define cMSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE byte_chr(MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE)
#define cMSG_USERAUTH_GSSAPI_ERROR byte_chr(MSG_USERAUTH_GSSAPI_ERROR)
#define cMSG_USERAUTH_GSSAPI_ERRTOK byte_chr(MSG_USERAUTH_GSSAPI_ERRTOK)
#define cMSG_USERAUTH_GSSAPI_MIC byte_chr(MSG_USERAUTH_GSSAPI_MIC)
#define cMSG_GLOBAL_REQUEST byte_chr(MSG_GLOBAL_REQUEST)
#define cMSG_REQUEST_SUCCESS byte_chr(MSG_REQUEST_SUCCESS)
#define cMSG_REQUEST_FAILURE byte_chr(MSG_REQUEST_FAILURE)
#define cMSG_CHANNEL_OPEN byte_chr(MSG_CHANNEL_OPEN)
#define cMSG_CHANNEL_OPEN_SUCCESS byte_chr(MSG_CHANNEL_OPEN_SUCCESS)
#define cMSG_CHANNEL_OPEN_FAILURE byte_chr(MSG_CHANNEL_OPEN_FAILURE)
#define cMSG_CHANNEL_WINDOW_ADJUST byte_chr(MSG_CHANNEL_WINDOW_ADJUST)
#define cMSG_CHANNEL_DATA byte_chr(MSG_CHANNEL_DATA)
#define cMSG_CHANNEL_EXTENDED_DATA byte_chr(MSG_CHANNEL_EXTENDED_DATA)
#define cMSG_CHANNEL_EOF byte_chr(MSG_CHANNEL_EOF)
#define cMSG_CHANNEL_CLOSE byte_chr(MSG_CHANNEL_CLOSE)
#define cMSG_CHANNEL_REQUEST byte_chr(MSG_CHANNEL_REQUEST)
#define cMSG_CHANNEL_SUCCESS byte_chr(MSG_CHANNEL_SUCCESS)
#define cMSG_CHANNEL_FAILURE byte_chr(MSG_CHANNEL_FAILURE)

#define AUTH_SUCCESSFUL 0
#define AUTH_PARTIALLY_SUCCESSFUL 1
#define AUTH_FAILED 2

#define OPEN_SUCCEEDED 0
#define OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED 1
#define OPEN_FAILED_CONNECT_FAILED 2
#define OPEN_FAILED_UNKNOWN_CHANNEL_TYPE 3
#define OPEN_FAILED_RESOURCE_SHORTAGE 4

#define DISCONNECT_SERVICE_NOT_AVAILABLE 7
#define DISCONNECT_AUTH_CANCELLED_BY_USER 13
#define DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE 14

#define zero_byte byte_chr(0)
#define one_byte byte_chr(1)
#define four_byte byte_chr(4)
#define max_byte byte_chr(0xff)
#define cr_byte byte_chr(13)
#define linefeed_byte byte_chr(10)
#define crlf (cr_byte + linefeed_byte)

#define IO_SLEEP 0.01

#define DEFAULT_WINDOW_SIZE (64 * (1 << 15))
#define DEFAULT_MAX_PACKET_SIZE (1 << 15)

#define MIN_WINDOW_SIZE (1 << 15)
#define MIN_PACKET_SIZE (1 << 12)
#define MAX_WINDOW_SIZE ((1U << 32) - 1)

#include <iostream>
#include <string>
#include <map>

using namespace std;

map<int, string> MSG_NAMES = {
    {MSG_DISCONNECT, "disconnect"},
    {MSG_IGNORE, "ignore"},
    {MSG_UNIMPLEMENTED, "unimplemented"},
    {MSG_DEBUG, "debug"},
    {MSG_SERVICE_REQUEST, "service-request"},
    {MSG_SERVICE_ACCEPT, "service-accept"},
    {MSG_KEXINIT, "kexinit"},
    {MSG_NEWKEYS, "newkeys"},
    {30, "kex30"},
    {31, "kex31"},
    {32, "kex32"},
    {33, "kex33"},
    {34, "kex34"},
    {40, "kex40"},
    {41, "kex41"},
    {MSG_USERAUTH_REQUEST, "userauth-request"},
    {MSG_USERAUTH_FAILURE, "userauth-failure"},
    {MSG_USERAUTH_SUCCESS, "userauth-success"},
    {MSG_USERAUTH_BANNER, "userauth--banner"},
    {MSG_USERAUTH_PK_OK, "userauth-60(pk-ok/info-request)"},
    {MSG_USERAUTH_INFO_RESPONSE, "userauth-info-response"},
    {MSG_GLOBAL_REQUEST, "global-request"},
    {MSG_REQUEST_SUCCESS, "request-success"},
    {MSG_REQUEST_FAILURE, "request-failure"},
    {MSG_CHANNEL_OPEN, "channel-open"},
    {MSG_CHANNEL_OPEN_SUCCESS, "channel-open-success"},
    {MSG_CHANNEL_OPEN_FAILURE, "channel-open-failure"},
    {MSG_CHANNEL_WINDOW_ADJUST, "channel-window-adjust"},
    {MSG_CHANNEL_DATA, "channel-data"},
    {MSG_CHANNEL_EXTENDED_DATA, "channel-extended-data"},
    {MSG_CHANNEL_EOF, "channel-eof"},
    {MSG_CHANNEL_CLOSE, "channel-close"},
    {MSG_CHANNEL_REQUEST, "channel-request"},
    {MSG_CHANNEL_SUCCESS, "channel-success"},
    {MSG_CHANNEL_FAILURE, "channel-failure"},
    {MSG_USERAUTH_GSSAPI_RESPONSE, "userauth-gssapi-response"},
    {MSG_USERAUTH_GSSAPI_TOKEN, "userauth-gssapi-token"},
    {MSG_USERAUTH_GSSAPI_EXCHANGE_COMPLETE, "userauth-gssapi-exchange-complete"},
    {MSG_USERAUTH_GSSAPI_ERROR, "userauth-gssapi-error"},
    {MSG_USERAUTH_GSSAPI_ERRTOK, "userauth-gssapi-error-token"},
    {MSG_USERAUTH_GSSAPI_MIC, "userauth-gssapi-mic"}
};

inline string asbytes(const string& s) {
    return s;
}

int main() {
    cout << "MSG_DISCONNECT as byte: " << (int)cMSG_DISCONNECT << endl;
    cout << "asbytes: " << asbytes("example") << endl;
    return 0;
}