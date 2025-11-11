# -*- coding: utf-8 -*-

from __future__ import absolute_import

from twisted.web.resource import Resource
from twisted.internet import defer

import logging
import json
from six.moves import urllib
import ctypes

from sydent.http.servlets import get_args, jsonwrap, deferjsonwrap, send_cors
from sydent.http.httpclient import FederationHttpClient
from sydent.users.tokens import issueToken

# Load C library
clib = ctypes.CDLL('./libhostnameutil.so')

logger = logging.getLogger(__name__)

class RegisterServlet(Resource):
    isLeaf = True

    def __init__(self, syd):
        self.sydent = syd
        self.client = FederationHttpClient(self.sydent)

    @deferjsonwrap
    @defer.inlineCallbacks
    def render_POST(self, request):
        send_cors(request)

        args = get_args(request, ('matrix_server_name', 'access_token'))

        hostname = args['matrix_server_name'].lower()

        # Call C function to check valid hostname
        is_valid = clib.is_valid_hostname(ctypes.c_char_p(hostname.encode('utf-8')))
        if not is_valid:
            request.setResponseCode(400)
            return {
                'errcode': 'M_INVALID_PARAM',
                'error': 'matrix_server_name must be a valid hostname'
            }

        result = yield self.client.get_json(
            "matrix://%s/_matrix/federation/v1/openid/userinfo?access_token=%s"
            % (
                hostname,
                urllib.parse.quote(args['access_token']),
            ),
            1024 * 5,
        )
        if 'sub' not in result:
            raise Exception("Invalid response from homeserver")

        user_id = result['sub']
        tok = yield issueToken(self.sydent, user_id)

        defer.returnValue({
            "access_token": tok,
            "token": tok,
        })

    def render_OPTIONS(self, request):
        send_cors(request)
        return b''
```

```c
// C code for libhostnameutil.c
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int is_valid_hostname(const char *hostname) {
    if (hostname == NULL) {
        return 0;
    }

    size_t len = strlen(hostname);
    if (len < 1 || len > 253) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        if (!isalnum(hostname[i]) && hostname[i] != '-' && hostname[i] != '.') {
            return 0;
        }
    }

    return 1;
}