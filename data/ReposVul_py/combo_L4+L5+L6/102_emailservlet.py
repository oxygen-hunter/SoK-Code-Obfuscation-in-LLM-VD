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

from sydent.util.stringutils import is_valid_client_secret, MAX_EMAIL_ADDRESS_LENGTH
from sydent.util.emailutils import EmailAddressException, EmailSendException
from sydent.validators import (
    IncorrectClientSecretException,
    InvalidSessionIdException,
    IncorrectSessionTokenException,
    SessionExpiredException,
)

from sydent.http.servlets import get_args, jsonwrap, send_cors
from sydent.http.auth import authV2


class EmailRequestCodeServlet(Resource):
    isLeaf = True

    def __init__(self, syd, require_auth=False):
        self.sydent = syd
        self.require_auth = require_auth

    @jsonwrap
    def render_POST(self, request):
        send_cors(request)

        if self.require_auth:
            authV2(self.sydent, request)

        args = get_args(request, ('email', 'client_secret', 'send_attempt'))

        email = args['email']
        sendAttempt = args['send_attempt']
        clientSecret = args['client_secret']

        def validate_email_and_secret():
            if not is_valid_client_secret(clientSecret):
                request.setResponseCode(400)
                return {
                    'errcode': 'M_INVALID_PARAM',
                    'error': 'Invalid client_secret provided'
                }
            if not (0 < len(email) <= MAX_EMAIL_ADDRESS_LENGTH):
                request.setResponseCode(400)
                return {
                    'errcode': 'M_INVALID_PARAM',
                    'error': 'Invalid email provided'
                }
            return None

        validation_error = validate_email_and_secret()
        if validation_error:
            return validation_error

        ipaddress = self.sydent.ip_from_request(request)
        brand = self.sydent.brand_from_request(request)

        nextLink = None
        if 'next_link' in args and not args['next_link'].startswith("file:///"):
            nextLink = args['next_link']

        def handle_email_exceptions():
            try:
                sid = self.sydent.validators.email.requestToken(
                    email, clientSecret, sendAttempt, nextLink, ipaddress=ipaddress, brand=brand,
                )
                return {'sid': str(sid)}
            except EmailAddressException:
                request.setResponseCode(400)
                return {'errcode': 'M_INVALID_EMAIL', 'error': 'Invalid email address'}
            except EmailSendException:
                request.setResponseCode(500)
                return {'errcode': 'M_EMAIL_SEND_ERROR', 'error': 'Failed to send email'}

        return handle_email_exceptions()

    def render_OPTIONS(self, request):
        send_cors(request)
        return b''


class EmailValidateCodeServlet(Resource):
    isLeaf = True

    def __init__(self, syd, require_auth=False):
        self.sydent = syd
        self.require_auth = require_auth

    def render_GET(self, request):
        args = get_args(request, ('nextLink',), required=False)

        resp = None
        def try_validation():
            nonlocal resp
            try:
                resp = self.do_validate_request(request)
            except:
                pass
        try_validation()

        def handle_verification_response():
            if resp and 'success' in resp and resp['success']:
                msg = "Verification successful! Please return to your Matrix client to continue."
                if 'nextLink' in args:
                    next_link = args['nextLink']
                    if not next_link.startswith("file:///"):
                        request.setResponseCode(302)
                        request.setHeader("Location", next_link)
            else:
                msg = "Verification failed: you may need to request another verification email"
            return msg
        
        msg = handle_verification_response()

        brand = self.sydent.brand_from_request(request)
        templateFile = self.sydent.get_branded_template(
            brand,
            "verify_response_template.html",
            ('http', 'verify_response_template'),
        )

        request.setHeader("Content-Type", "text/html")
        res = open(templateFile).read() % {'message': msg}
        return res.encode("UTF-8")

    @jsonwrap
    def render_POST(self, request):
        send_cors(request)

        if self.require_auth:
            authV2(self.sydent, request)

        return self.do_validate_request(request)

    def do_validate_request(self, request):
        args = get_args(request, ('token', 'sid', 'client_secret'))

        sid = args['sid']
        tokenString = args['token']
        clientSecret = args['client_secret']

        def validate_secret():
            if not is_valid_client_secret(clientSecret):
                request.setResponseCode(400)
                return {
                    'errcode': 'M_INVALID_PARAM',
                    'error': 'Invalid client_secret provided'
                }
            return None
        
        validation_error = validate_secret()
        if validation_error:
            return validation_error

        def handle_validation_exceptions():
            try:
                return self.sydent.validators.email.validateSessionWithToken(sid, clientSecret, tokenString)
            except IncorrectClientSecretException:
                return {'success': False, 'errcode': 'M_INVALID_PARAM',
                        'error': "Client secret does not match the one given when requesting the token"}
            except SessionExpiredException:
                return {'success': False, 'errcode': 'M_SESSION_EXPIRED',
                        'error': "This validation session has expired: call requestToken again"}
            except InvalidSessionIdException:
                return {'success': False, 'errcode': 'M_INVALID_PARAM',
                        'error': "The token doesn't match"}
            except IncorrectSessionTokenException:
                return {'success': False, 'errcode': 'M_NO_VALID_SESSION',
                        'error': "No session could be found with this sid"}

        return handle_validation_exceptions()

    def render_OPTIONS(self, request):
        send_cors(request)
        return b''