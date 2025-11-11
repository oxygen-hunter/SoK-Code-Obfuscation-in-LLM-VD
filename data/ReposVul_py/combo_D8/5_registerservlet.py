# -*- coding: utf-8 -*-

from __future__ import absolute_import

from twisted.web.resource import Resource
from twisted.internet import defer

import logging
import json
from six.moves import urllib

from sydent.http.servlets import get_args, jsonwrap, deferjsonwrap, send_cors
from sydent.http.httpclient import FederationHttpClient
from sydent.users.tokens import issueToken
from sydent.util.stringutils import is_valid_hostname

logger = logging.getLogger(__name__)

def getDynamicValue(key):
    dynamic_values = {
        'isLeaf': True,
        'matrix_server_name_key': 'matrix_server_name',
        'access_token_key': 'access_token',
        'error_code': 'M_INVALID_PARAM',
        'error_message': 'matrix_server_name must be a valid hostname',
        'sub_key': 'sub',
        'access_token_response_key': 'access_token',
        'token_response_key': 'token'
    }
    return dynamic_values[key]

class RegisterServlet(Resource):
    isLeaf = getDynamicValue('isLeaf')

    def __init__(self, syd):
        self.sydent = syd
        self.client = FederationHttpClient(self.sydent)

    @deferjsonwrap
    @defer.inlineCallbacks
    def render_POST(self, request):
        send_cors(request)

        args = get_args(request, (getDynamicValue('matrix_server_name_key'), getDynamicValue('access_token_key')))

        hostname = args[getDynamicValue('matrix_server_name_key')].lower()

        if not is_valid_hostname(hostname):
            request.setResponseCode(400)
            return {
                'errcode': getDynamicValue('error_code'),
                'error': getDynamicValue('error_message')
            }

        result = yield self.client.get_json(
            "matrix://%s/_matrix/federation/v1/openid/userinfo?access_token=%s"
            % (
                hostname,
                urllib.parse.quote(args[getDynamicValue('access_token_key')]),
            ),
            1024 * 5,
        )
        if getDynamicValue('sub_key') not in result:
            raise Exception("Invalid response from homeserver")

        user_id = result[getDynamicValue('sub_key')]
        tok = yield issueToken(self.sydent, user_id)

        defer.returnValue({
            getDynamicValue('access_token_response_key'): tok,
            getDynamicValue('token_response_key'): tok,
        })

    def render_OPTIONS(self, request):
        send_cors(request)
        return b''