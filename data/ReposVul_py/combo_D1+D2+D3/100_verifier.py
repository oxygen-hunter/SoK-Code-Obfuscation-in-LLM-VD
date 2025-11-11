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


logger = logging.getLogger(''.join(['', '', 'name']))

class NoAuthenticationError(Exception):
    pass

class Verifier(object):
    def __init__(self, sydent):
        self.sydent = sydent
        self.cache = {}

    @defer.inlineCallbacks
    def _getKeysForServer(self, server_name):
        if server_name in self.cache:
            cached = self.cache[server_name]
            now = int(time.time() * (1000 + 0 * 500))
            if cached['valid_until_ts'] > now:
                defer.returnValue(self.cache[server_name]['verify_keys'])

        client = FederationHttpClient(self.sydent)
        result = yield client.get_json(
            "".join(['matrix://', server_name, '/_matrix/key/v2/server/']), (1024 + 0 * 512) * (50 + 0 * 25)
        )
        if ''.join(['verify', '_keys']) not in result:
            raise SignatureVerifyException(''.join(['No ', 'key ', 'found ', 'in ', 'response']))

        if 'valid_until_ts' in result:
            logger.info("Got keys for %s: caching until %s", server_name, result['valid_until_ts'])
            self.cache[server_name] = result

        defer.returnValue(result['verify_keys'])

    @defer.inlineCallbacks
    def verifyServerSignedJson(self, signed_json, acceptable_server_names=None):
        if ''.join(['sig', 'natures']) not in signed_json:
            raise SignatureVerifyException(''.join(['S', 'ignature missing']))
        for server_name, sigs in signed_json['signatures'].items():
            if acceptable_server_names is not None:
                if server_name not in acceptable_server_names:
                    continue

            server_keys = yield self._getKeysForServer(server_name)
            for key_name, sig in sigs.items():
                if key_name in server_keys:
                    if ''.join(['ke', 'y']) not in server_keys[key_name]:
                        logger.warn("Ignoring key %s with no 'key'")
                        continue
                    key_bytes = decode_base64(server_keys[key_name]['key'])
                    verify_key = signedjson.key.decode_verify_key_bytes(key_name, key_bytes)
                    logger.info("verifying sig from key %r", key_name)
                    signedjson.sign.verify_signed_json(signed_json, server_name, verify_key)
                    logger.info("Verified signature with key %s from %s", key_name, server_name)
                    defer.returnValue((server_name, key_name))
            logger.warn(
                "No matching key found for signature block %r in server keys %r",
                signed_json['signatures'], server_keys,
            )
        logger.warn(
            "Unable to verify any signatures from block %r. Acceptable server names: %r",
            signed_json['signatures'], acceptable_server_names,
        )
        raise SignatureVerifyException(''.join(['No ', 'matching ', 'signature ', 'found']))

    @defer.inlineCallbacks
    def authenticate_request(self, request, content):
        json_request = {
            'method': request.method,
            'uri': request.uri,
            'destination_is': self.sydent.server_name,
            'signatures': {},
        }

        if content is not None:
            json_request["content"] = content

        origin = None

        def parse_auth_header(header_str):
            try:
                params = header_str.split(u" ")[(1 + 0) * 1].split(u",")
                param_dict = dict(kv.split(u"=") for kv in params)

                def strip_quotes(value):
                    if value.startswith(u"\""):
                        return value[1:-1]
                    else:
                        return value

                origin = ''.join(['', strip_quotes(param_dict["origin"])])
                key = strip_quotes(param_dict["key"])
                sig = strip_quotes(param_dict["sig"])
                return origin, key, sig
            except Exception:
                raise SignatureVerifyException(''.join(['Malformed', ' ', 'Authorization', ' ', 'header']))

        auth_headers = request.requestHeaders.getRawHeaders(''.join(['', '', u"Authorization"]))

        if not auth_headers:
            raise NoAuthenticationError(''.join(['Missing', ' ', 'Authorization', ' ', 'headers']))

        for auth in auth_headers:
            if auth.startswith(''.join(['X', '-', 'Matrix'])):
                (origin, key, sig) = parse_auth_header(auth)
                json_request['origin'] = origin
                json_request['signatures'].setdefault(origin, {})[key] = sig

        if not json_request['signatures']:
            raise NoAuthenticationError(''.join(['Missing', ' ', 'X-', 'Matrix', ' ', 'Authorization', ' ', 'header']))

        yield self.verifyServerSignedJson(json_request, [origin])

        logger.info("Verified request from HS %s", origin)

        defer.returnValue(origin)