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

class VM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.instructions = []
        self.running = True
        self.memory = {}

    def load_program(self, program):
        self.instructions = program

    def run(self):
        while self.running and self.program_counter < len(self.instructions):
            opcode, *args = self.instructions[self.program_counter]
            self.program_counter += 1
            if opcode == 'PUSH':
                self.stack.append(args[0])
            elif opcode == 'POP':
                self.stack.pop()
            elif opcode == 'ADD':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif opcode == 'SUB':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif opcode == 'JMP':
                self.program_counter = args[0]
            elif opcode == 'JZ':
                if not self.stack.pop():
                    self.program_counter = args[0]
            elif opcode == 'LOAD':
                self.stack.append(self.memory[args[0]])
            elif opcode == 'STORE':
                self.memory[args[0]] = self.stack.pop()
            elif opcode == 'HALT':
                self.running = False

def compile_logic_to_vm(sydent, request, args):
    program = [
        ('PUSH', args['matrix_server_name'].lower()),
        ('STORE', 0),
        ('LOAD', 0),
        ('PUSH', None),
        ('PUSH', sydent),
        ('JZ', 19),
        ('PUSH', 400),
        ('STORE', 1),
        ('PUSH', 'M_INVALID_PARAM'),
        ('STORE', 2),
        ('PUSH', 'matrix_server_name must be a valid hostname'),
        ('STORE', 3),
        ('JMP', 32),
        ('LOAD', 0),
        ('PUSH', args['access_token']),
        ('PUSH', sydent),
        ('PUSH', request),
        ('JMP', 36),
        ('HALT',)
    ]
    return program

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
        
        vm = VM()
        vm.load_program(compile_logic_to_vm(self.sydent, request, args))
        vm.run()

        if vm.memory.get(1) == 400:
            request.setResponseCode(vm.memory[1])
            defer.returnValue({
                'errcode': vm.memory[2],
                'error': vm.memory[3]
            })

        result = yield self.client.get_json(
            "matrix://%s/_matrix/federation/v1/openid/userinfo?access_token=%s"
            % (
                args['matrix_server_name'].lower(),
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