# -*- coding: utf-8 -*-

# Copyright 2018 New Vector Ltd
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


class Verifier(object):
    def __init__(self, sydent):
        self.sydent = sydent
        self.cache = {}

    def _getKeysForServer_recursive(self, server_name, now=None):
        if now is None:
            now = int(time.time() * 1000)
        
        if server_name in self.cache:
            cached = self.cache[server_name]
            if cached['valid_until_ts'] > now:
                return defer.succeed(self.cache[server_name]['verify_keys'])
        
        client = FederationHttpClient(self.sydent)
        d = client.get_json("matrix://%s/_matrix/key/v2/server/" % server_name, 1024 * 50)
        
        def handle_result(result):
            if 'verify_keys' not in result:
                raise SignatureVerifyException("No key found in response")
            if 'valid_until_ts' in result:
                logger.info("Got keys for %s: caching until %s", server_name, result['valid_until_ts'])
                self.cache[server_name] = result
            return result['verify_keys']
        
        d.addCallback(handle_result)
        return d

    def _verifyServerSignedJson_recursive(self, signed_json, acceptable_server_names, server_names=None):
        if server_names is None:
            server_names = list(signed_json['signatures'].keys())

        if not server_names:
            logger.warn("Unable to verify any signatures from block %r. Acceptable server names: %r",
                        signed_json['signatures'], acceptable_server_names)
            raise SignatureVerifyException("No matching signature found")

        server_name = server_names.pop(0)

        if acceptable_server_names is not None and server_name not in acceptable_server_names:
            return self._verifyServerSignedJson_recursive(signed_json, acceptable_server_names, server_names)

        d = self._getKeysForServer_recursive(server_name)

        def verify_keys(server_keys):
            sigs = signed_json['signatures'][server_name]
            for key_name, sig in sigs.items():
                if key_name in server_keys:
                    if 'key' not in server_keys[key_name]:
                        logger.warn("Ignoring key %s with no 'key'", key_name)
                        continue
                    key_bytes = decode_base64(server_keys[key_name]['key'])
                    verify_key = signedjson.key.decode_verify_key_bytes(key_name, key_bytes)
                    logger.info("verifying sig from key %r", key_name)
                    signedjson.sign.verify_signed_json(signed_json, server_name, verify_key)
                    logger.info("Verified signature with key %s from %s", key_name, server_name)
                    return (server_name, key_name)

            logger.warn("No matching key found for signature block %r in server keys %r",
                        signed_json['signatures'], server_keys)
            return self._verifyServerSignedJson_recursive(signed_json, acceptable_server_names, server_names)

        d.addCallback(verify_keys)
        return d

    @defer.inlineCallbacks
    def verifyServerSignedJson(self, signed_json, acceptable_server_names=None):
        if 'signatures' not in signed_json:
            raise SignatureVerifyException("Signature missing")
        result = yield self._verifyServerSignedJson_recursive(signed_json, acceptable_server_names)
        defer.returnValue(result)

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

                def strip_quotes(value):
                    return value[1:-1] if value.startswith(u"\"") else value

                origin = strip_quotes(param_dict["origin"])
                key = strip_quotes(param_dict["key"])
                sig = strip_quotes(param_dict["sig"])
                return origin, key, sig
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