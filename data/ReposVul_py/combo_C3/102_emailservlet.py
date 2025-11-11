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

class StackVM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.instructions = []

    def load_program(self, instructions):
        self.instructions = instructions
        self.program_counter = 0

    def execute(self):
        while self.program_counter < len(self.instructions):
            ins = self.instructions[self.program_counter]
            op = ins[0]

            if op == "PUSH":
                self.stack.append(ins[1])
            elif op == "POP":
                self.stack.pop()
            elif op == "ADD":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif op == "SUB":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(b - a)
            elif op == "JMP":
                self.program_counter = ins[1] - 1
            elif op == "JZ":
                if self.stack.pop() == 0:
                    self.program_counter = ins[1] - 1
            elif op == "LOAD":
                self.stack.append(ins[1])
            elif op == "STORE":
                self.stack[ins[1]] = self.stack.pop()

            self.program_counter += 1

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

        vm = StackVM()
        vm.load_program([
            ("PUSH", len(email)),
            ("LOAD", MAX_EMAIL_ADDRESS_LENGTH),
            ("SUB",),
            ("JZ", 7),
            ("PUSH", 0),
            ("JMP", 9),
            ("PUSH", 1),
            ("JZ", 12),
            ("LOAD", "M_INVALID_PARAM"),
            ("LOAD", "Invalid email provided"),
            ("JMP", 20),
            ("PUSH", is_valid_client_secret(clientSecret)),
            ("JZ", 16),
            ("LOAD", "M_INVALID_PARAM"),
            ("LOAD", "Invalid client_secret provided"),
            ("JMP", 20),
            ("LOAD", self.sydent.ip_from_request(request)),
            ("LOAD", self.sydent.brand_from_request(request)),
            ("PUSH", None),
            ("JMP", 29),
            ("LOAD", args.get('next_link', None)),
            ("PUSH", "file:///"),
            ("JZ", 29),
            ("STORE", 2),
            ("LOAD", "try"),
            ("STORE", 3),
            ("LOAD", self.sydent.validators.email.requestToken),
            ("STORE", 4),
            ("LOAD", email),
            ("LOAD", clientSecret),
            ("LOAD", sendAttempt),
            ("LOAD", vm.stack[2]),
            ("LOAD", vm.stack[0]),
            ("LOAD", vm.stack[1]),
            ("CALL", 4),
            ("LOAD", "sid"),
            ("STORE", 5),
            ("LOAD", "str"),
            ("LOAD", vm.stack[5]),
            ("CALL", 1),
            ("STORE", 6),
            ("LOAD", "resp"),
            ("LOAD", {"sid": vm.stack[6]}),
            ("STORE", 7),
            ("LOAD", "except"),
            ("STORE", 8),
            ("LOAD", EmailAddressException),
            ("STORE", 9),
            ("LOAD", EmailSendException),
            ("STORE", 10),
            ("LOAD", "request"),
            ("LOAD", 400),
            ("CALL", 1),
            ("LOAD", "resp"),
            ("LOAD", {"errcode": "M_INVALID_EMAIL", "error": "Invalid email address"}),
            ("STORE", 11),
            ("LOAD", "request"),
            ("LOAD", 500),
            ("CALL", 1),
            ("LOAD", "resp"),
            ("LOAD", {"errcode": "M_EMAIL_SEND_ERROR", "error": "Failed to send email"}),
            ("STORE", 12),
            ("LOAD", vm.stack[7]),
            ("JZ", 88),
            ("LOAD", "return"),
            ("LOAD", vm.stack[11]),
            ("CALL", 1),
            ("LOAD", vm.stack[12]),
            ("JZ", 92),
            ("LOAD", "return"),
            ("LOAD", vm.stack[12]),
            ("CALL", 1),
            ("LOAD", vm.stack[7]),
            ("CALL", 0)
        ])
        vm.execute()

        if vm.stack[-1]:
            return vm.stack[-1]

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

        vm = StackVM()
        vm.load_program([
            ("LOAD", self.do_validate_request),
            ("LOAD", request),
            ("CALL", 1),
            ("STORE", 0),
            ("LOAD", vm.stack[0]),
            ("JZ", 13),
            ("LOAD", "success"),
            ("LOAD", vm.stack[0]),
            ("CALL", 1),
            ("JZ", 13),
            ("LOAD", "msg"),
            ("LOAD", "Verification successful! Please return to your Matrix client to continue."),
            ("STORE", 1),
            ("LOAD", "nextLink"),
            ("LOAD", args),
            ("CALL", 1),
            ("STORE", 2),
            ("LOAD", vm.stack[2]),
            ("JZ", 21),
            ("LOAD", "file:///"),
            ("LOAD", vm.stack[2]),
            ("CALL", 1),
            ("JZ", 26),
            ("LOAD", "request"),
            ("LOAD", 302),
            ("CALL", 1),
            ("LOAD", "request"),
            ("LOAD", "Location"),
            ("LOAD", vm.stack[2]),
            ("CALL", 2),
            ("JMP", 28),
            ("LOAD", "msg"),
            ("LOAD", "Verification failed: you may need to request another verification email"),
            ("STORE", 1),
            ("LOAD", "brand"),
            ("LOAD", self.sydent.brand_from_request),
            ("LOAD", request),
            ("CALL", 1),
            ("STORE", 3),
            ("LOAD", "templateFile"),
            ("LOAD", self.sydent.get_branded_template),
            ("LOAD", vm.stack[3]),
            ("LOAD", "verify_response_template.html"),
            ("LOAD", ["http", "verify_response_template"]),
            ("CALL", 3),
            ("STORE", 4),
            ("LOAD", "request"),
            ("LOAD", "Content-Type"),
            ("LOAD", "text/html"),
            ("CALL", 2),
            ("LOAD", "res"),
            ("LOAD", open),
            ("LOAD", vm.stack[4]),
            ("CALL", 1),
            ("LOAD", "read"),
            ("CALL", 0),
            ("LOAD", "%"),
            ("LOAD", {"message": vm.stack[1]}),
            ("CALL", 2),
            ("STORE", 5),
            ("LOAD", "return"),
            ("LOAD", vm.stack[5]),
            ("LOAD", "encode"),
            ("LOAD", "UTF-8"),
            ("CALL", 1),
            ("CALL", 1),
            ("CALL", 0)
        ])
        vm.execute()

        return vm.stack[-1]

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

        vm = StackVM()
        vm.load_program([
            ("PUSH", is_valid_client_secret(clientSecret)),
            ("JZ", 7),
            ("LOAD", self.sydent.validators.email.validateSessionWithToken),
            ("LOAD", sid),
            ("LOAD", clientSecret),
            ("LOAD", tokenString),
            ("CALL", 3),
            ("STORE", 0),
            ("LOAD", vm.stack[0]),
            ("JZ", 13),
            ("LOAD", "return"),
            ("LOAD", vm.stack[0]),
            ("CALL", 1),
            ("LOAD", "return"),
            ("LOAD", {"success": False, "errcode": "M_INVALID_PARAM",
                      "error": "Invalid client_secret provided"}),
            ("CALL", 1),
            ("LOAD", "except"),
            ("STORE", 1),
            ("LOAD", IncorrectClientSecretException),
            ("STORE", 2),
            ("LOAD", SessionExpiredException),
            ("STORE", 3),
            ("LOAD", InvalidSessionIdException),
            ("STORE", 4),
            ("LOAD", IncorrectSessionTokenException),
            ("STORE", 5),
            ("LOAD", "return"),
            ("LOAD", {"success": False, "errcode": "M_INVALID_PARAM",
                      "error": "Client secret does not match the one given when requesting the token"}),
            ("CALL", 1),
            ("LOAD", "return"),
            ("LOAD", {"success": False, "errcode": "M_SESSION_EXPIRED",
                      "error": "This validation session has expired: call requestToken again"}),
            ("CALL", 1),
            ("LOAD", "return"),
            ("LOAD", {"success": False, "errcode": "M_INVALID_PARAM",
                      "error": "The token doesn't match"}),
            ("CALL", 1),
            ("LOAD", "return"),
            ("LOAD", {"success": False, "errcode": "M_NO_VALID_SESSION",
                      "error": "No session could be found with this sid"}),
            ("CALL", 1)
        ])
        vm.execute()

        return vm.stack[-1]

    def render_OPTIONS(self, request):
        send_cors(request)
        return b''