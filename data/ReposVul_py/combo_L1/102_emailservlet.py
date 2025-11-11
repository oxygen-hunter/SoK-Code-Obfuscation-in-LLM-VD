# -*- coding: utf-8 -*-

# Copyright 2014 OpenMarket Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from __future__ import absolute_import

from twisted.web.resource import Resource

from sydent.util.stringutils import is_valid_client_secret as OX1A3B5C6D, MAX_EMAIL_ADDRESS_LENGTH as OX6D4C3B2A
from sydent.util.emailutils import EmailAddressException as OXC3B2A1D6, EmailSendException as OX2D1A3C6B
from sydent.validators import (
    IncorrectClientSecretException as OX12345678,
    InvalidSessionIdException as OX87654321,
    IncorrectSessionTokenException as OX1B2A3C4D,
    SessionExpiredException as OX4D3C2B1A,
)

from sydent.http.servlets import get_args as OXABCDEF12, jsonwrap as OX12FEDCBA, send_cors as OXBAADF00D
from sydent.http.auth import authV2 as OXDEADBEEF


class OX7B4DF339(Resource):
    isLeaf = True

    def __init__(self, OX3F2B1A4D, OX2A3B4C5D=False):
        self.OX9C8D7E6F = OX3F2B1A4D
        self.OX5D4E3F2A = OX2A3B4C5D

    @OX12FEDCBA
    def render_POST(self, OX4A3B2C1D):
        OXBAADF00D(OX4A3B2C1D)

        if self.OX5D4E3F2A:
            OXDEADBEEF(self.OX9C8D7E6F, OX4A3B2C1D)

        OX8F7E6D5C = OXABCDEF12(OX4A3B2C1D, ('email', 'client_secret', 'send_attempt'))

        OXA7B6C5D4 = OX8F7E6D5C['email']
        OX1B2C3D4E = OX8F7E6D5C['send_attempt']
        OXF4E3D2C1 = OX8F7E6D5C['client_secret']

        if not OX1A3B5C6D(OXF4E3D2C1):
            OX4A3B2C1D.setResponseCode(400)
            return {
                'errcode': 'M_INVALID_PARAM',
                'error': 'Invalid client_secret provided'
            }

        if not (0 < len(OXA7B6C5D4) <= OX6D4C3B2A):
            OX4A3B2C1D.setResponseCode(400)
            return {
                'errcode': 'M_INVALID_PARAM',
                'error': 'Invalid email provided'
            }

        OX2C3B4A5D = self.OX9C8D7E6F.ip_from_request(OX4A3B2C1D)
        OX5A4B3C2D = self.OX9C8D7E6F.brand_from_request(OX4A3B2C1D)

        OXC6B5A4D3 = None
        if 'next_link' in OX8F7E6D5C and not OX8F7E6D5C['next_link'].startswith("file:///"):
            OXC6B5A4D3 = OX8F7E6D5C['next_link']

        try:
            OX6F5E4D3C = self.OX9C8D7E6F.validators.email.requestToken(
                OXA7B6C5D4, OXF4E3D2C1, OX1B2C3D4E, OXC6B5A4D3, ipaddress=OX2C3B4A5D, brand=OX5A4B3C2D,
            )
            OX4D3C2B1A = {'sid': str(OX6F5E4D3C)}
        except OXC3B2A1D6:
            OX4A3B2C1D.setResponseCode(400)
            OX4D3C2B1A = {'errcode': 'M_INVALID_EMAIL', 'error': 'Invalid email address'}
        except OX2D1A3C6B:
            OX4A3B2C1D.setResponseCode(500)
            OX4D3C2B1A = {'errcode': 'M_EMAIL_SEND_ERROR', 'error': 'Failed to send email'}

        return OX4D3C2B1A

    def render_OPTIONS(self, OX9F8E7D6C):
        OXBAADF00D(OX9F8E7D6C)
        return b''


class OX5C4B3A2D(Resource):
    isLeaf = True

    def __init__(self, OX8E7D6C5B, OX1A2B3C4D=False):
        self.OX9C8D7E6F = OX8E7D6C5B
        self.OX5D4E3F2A = OX1A2B3C4D

    def render_GET(self, OX7D6C5B4A):
        OX2A3B4C5D = OXABCDEF12(OX7D6C5B4A, ('nextLink',), required=False)

        OX6E5D4C3B = None
        try:
            OX6E5D4C3B = self.OX9B8A7C6D(OX7D6C5B4A)
        except:
            pass
        if OX6E5D4C3B and 'success' in OX6E5D4C3B and OX6E5D4C3B['success']:
            OX7B8C9D0A = "Verification successful! Please return to your Matrix client to continue."
            if 'nextLink' in OX2A3B4C5D:
                OX6C5B4A3D = OX2A3B4C5D['nextLink']
                if not OX6C5B4A3D.startswith("file:///"):
                    OX7D6C5B4A.setResponseCode(302)
                    OX7D6C5B4A.setHeader("Location", OX6C5B4A3D)
        else:
            OX7B8C9D0A = "Verification failed: you may need to request another verification email"

        OX5E4D3C2B = self.OX9C8D7E6F.brand_from_request(OX7D6C5B4A)
        OXA9B8C7D6 = self.OX9C8D7E6F.get_branded_template(
            OX5E4D3C2B,
            "verify_response_template.html",
            ('http', 'verify_response_template'),
        )

        OX7D6C5B4A.setHeader("Content-Type", "text/html")
        OX3B2A1D0E = open(OXA9B8C7D6).read() % {'message': OX7B8C9D0A}
        return OX3B2A1D0E.encode("UTF-8")

    @OX12FEDCBA
    def render_POST(self, OX2B1A3D4C):
        OXBAADF00D(OX2B1A3D4C)

        if self.OX5D4E3F2A:
            OXDEADBEEF(self.OX9C8D7E6F, OX2B1A3D4C)

        return self.OX9B8A7C6D(OX2B1A3D4C)

    def OX9B8A7C6D(self, OX3D2C1B0A):
        OX8A7B6C5D = OXABCDEF12(OX3D2C1B0A, ('token', 'sid', 'client_secret'))

        OX6D5C4B3A = OX8A7B6C5D['sid']
        OX9E8F7D6C = OX8A7B6C5D['token']
        OX4A3B2C1D = OX8A7B6C5D['client_secret']

        if not OX1A3B5C6D(OX4A3B2C1D):
            OX3D2C1B0A.setResponseCode(400)
            return {
                'errcode': 'M_INVALID_PARAM',
                'error': 'Invalid client_secret provided'
            }

        try:
            return self.OX9C8D7E6F.validators.email.validateSessionWithToken(OX6D5C4B3A, OX4A3B2C1D, OX9E8F7D6C)
        except OX12345678:
            return {'success': False, 'errcode': 'M_INVALID_PARAM',
                    'error': "Client secret does not match the one given when requesting the token"}
        except OX4D3C2B1A:
            return {'success': False, 'errcode': 'M_SESSION_EXPIRED',
                    'error': "This validation session has expired: call requestToken again"}
        except OX87654321:
            return {'success': False, 'errcode': 'M_INVALID_PARAM',
                    'error': "The token doesn't match"}
        except OX1B2A3C4D:
            return {'success': False, 'errcode': 'M_NO_VALID_SESSION',
                    'error': "No session could be found with this sid"}

    def render_OPTIONS(self, OX5F4E3D2C):
        OXBAADF00D(OX5F4E3D2C)
        return b''