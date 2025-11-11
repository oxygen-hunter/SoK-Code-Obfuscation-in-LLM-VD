# -*- coding: utf-8 -*-

from __future__ import absolute_import

import logging
import time

from twisted.internet import defer
from unpaddedbase64 import decode_base64
import signedjson.sign
import signedjson.key
from signedjson.sign import SignatureVerifyException

from sydent.http.httpclient import FederationHttpClient

logger = logging.getLogger(__name__)

class NoAuthenticationError(Exception):
    pass

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            op = instr[0]
            if op == "PUSH":
                self.stack.append(instr[1])
            elif op == "POP":
                self.stack.pop()
            elif op == "ADD":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif op == "SUB":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif op == "JMP":
                self.pc = instr[1]
                continue
            elif op == "JZ":
                if self.stack.pop() == 0:
                    self.pc = instr[1]
                    continue
            elif op == "LOAD":
                self.stack.append(instr[1])
            elif op == "STORE":
                val = self.stack.pop()
                self.stack[instr[1]] = val
            self.pc += 1

class Verifier(object):
    def __init__(self, sydent):
        self.sydent = sydent
        self.cache = {}
        self.vm = VM()

    @defer.inlineCallbacks
    def _getKeysForServer(self, server_name):
        if server_name in self.cache:
            cached = self.cache[server_name]
            now = int(time.time() * 1000)
            if cached['valid_until_ts'] > now:
                defer.returnValue(self.cache[server_name]['verify_keys'])

        client = FederationHttpClient(self.sydent)
        result = yield client.get_json("matrix://%s/_matrix/key/v2/server/" % server_name, 1024 * 50)
        if 'verify_keys' not in result:
            raise SignatureVerifyException("No key found in response")

        if 'valid_until_ts' in result:
            logger.info("Got keys for %s: caching until %s", server_name, result['valid_until_ts'])
            self.cache[server_name] = result

        defer.returnValue(result['verify_keys'])

    @defer.inlineCallbacks
    def verifyServerSignedJson(self, signed_json, acceptable_server_names=None):
        if 'signatures' not in signed_json:
            raise SignatureVerifyException("Signature missing")

        for server_name, sigs in signed_json['signatures'].items():
            if acceptable_server_names is not None and server_name not in acceptable_server_names:
                continue

            server_keys = yield self._getKeysForServer(server_name)
            for key_name, sig in sigs.items():
                if key_name in server_keys and 'key' in server_keys[key_name]:
                    key_bytes = decode_base64(server_keys[key_name]['key'])
                    verify_key = signedjson.key.decode_verify_key_bytes(key_name, key_bytes)
                    signedjson.sign.verify_signed_json(signed_json, server_name, verify_key)
                    defer.returnValue((server_name, key_name))

        raise SignatureVerifyException("No matching signature found")

    @defer.inlineCallbacks
    def authenticate_request(self, request, content):
        json_request = {
            "method": request.method,
            "uri": request.uri,
            "destination_is": self.sydent.server_name,
            "signatures": {},
        }

        if content is not None:
            json_request["content"] = content

        origin = None

        def parse_auth_header(header_str):
            try:
                params = header_str.split(u" ")[1].split(u",")
                param_dict = dict(kv.split(u"=") for kv in params)
                return (param_dict["origin"].strip('"'), param_dict["key"].strip('"'), param_dict["sig"].strip('"'))
            except Exception:
                raise SignatureVerifyException("Malformed Authorization header")

        auth_headers = request.requestHeaders.getRawHeaders(u"Authorization")

        if not auth_headers:
            raise NoAuthenticationError("Missing Authorization headers")

        for auth in auth_headers:
            if auth.startswith(u"X-Matrix"):
                (origin, key, sig) = parse_auth_header(auth)
                json_request["origin"] = origin
                json_request["signatures"].setdefault(origin, {})[key] = sig

        if not json_request["signatures"]:
            raise NoAuthenticationError("Missing X-Matrix Authorization header")

        yield self.verifyServerSignedJson(json_request, [origin])
        logger.info("Verified request from HS %s", origin)
        defer.returnValue(origin)