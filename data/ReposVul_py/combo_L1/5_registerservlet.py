# -*- coding: utf-8 -*-
from __future__ import absolute_import
from twisted.web.resource import Resource as OX74EA56E1
from twisted.internet import defer as OX6EA5E264
import logging as OXE89AD59C
import json as OXA1F1A9C5
from six.moves import urllib as OXAF1EE6B2
from sydent.http.servlets import get_args as OX1A9BCBDA, jsonwrap as OX2B7A6D96, deferjsonwrap as OXE4E2B1A9, send_cors as OX4D3B8F8F
from sydent.http.httpclient import FederationHttpClient as OXBF810EDE
from sydent.users.tokens import issueToken as OXAF5D3D74
from sydent.util.stringutils import is_valid_hostname as OX7D1F9B29
OXF4B1D5D5 = OXE89AD59C.getLogger(__name__)
class OX7C3E9991(OX74EA56E1):
    OX3F8B8A2E = True
    def __init__(self, OX680C8C02):
        self.OX2CBB3D2F = OX680C8C02
        self.OX8A9D3F2E = OXBF810EDE(self.OX2CBB3D2F)
    @OXE4E2B1A9
    @OX6EA5E264.inlineCallbacks
    def OX7B4DF339(self, OX5C3E4F1F):
        OX4D3B8F8F(OX5C3E4F1F)
        OX2BB6C5B5 = OX1A9BCBDA(OX5C3E4F1F, ('matrix_server_name', 'access_token'))
        OX2B5C8C76 = OX2BB6C5B5['matrix_server_name'].lower()
        if not OX7D1F9B29(OX2B5C8C76):
            OX5C3E4F1F.setResponseCode(400)
            return {'errcode': 'M_INVALID_PARAM','error': 'matrix_server_name must be a valid hostname'}
        OX7A2B56CF = yield self.OX8A9D3F2E.get_json(
            "matrix://%s/_matrix/federation/v1/openid/userinfo?access_token=%s" % (
                OX2B5C8C76,
                OXAF1EE6B2.parse.quote(OX2BB6C5B5['access_token']),
            ),
            1024 * 5,
        )
        if 'sub' not in OX7A2B56CF:
            raise Exception("Invalid response from homeserver")
        OXFC6A3B4A = OX7A2B56CF['sub']
        OX0E2B4A3C = yield OXAF5D3D74(self.OX2CBB3D2F, OXFC6A3B4A)
        OX6EA5E264.returnValue({
            "access_token": OX0E2B4A3C,
            "token": OX0E2B4A3C,
        })
    def OXC3CF4E25(self, OX5C3E4F1F):
        OX4D3B8F8F(OX5C3E4F1F)
        return b''