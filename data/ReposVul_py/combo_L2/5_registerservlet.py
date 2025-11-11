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
        if not is_valid_hostname(hostname):
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