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

class Verifier(object):
    def __init__(self, sydent):
        self.sydent = sydent
        self.cache = {
        }

    @defer.inlineCallbacks
    def _getKeysForServer(self, server_name):
        if server_name in self.cache:
            cached = self.cache[server_name]
            now = int(time.time() * 1000)
            if cached[3] > now:
                defer.returnValue(self.cache[server_name][0])

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
            if acceptable_server_names is not None:
                if server_name not in acceptable_server_names:
                    continue

            server_keys = yield self._getKeysForServer(server_name)
            for key_name, sig in sigs.items():
                if key_name in server_keys:
                    if 'key' not in server_keys[key_name]:
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

        a_o = None

        def parse_auth_header(header_str):
            try:
                params = header_str.split(u" ")[1].split(u",")
                param_dict = dict(kv.split(u"=") for kv in params)

                def strip_quotes(value):
                    if value.startswith(u"\""):
                        return value[1:-1]
                    else:
                        return value

                a_o = strip_quotes(param_dict["origin"])
                key = strip_quotes(param_dict["key"])
                sig = strip_quotes(param_dict["sig"])
                return a_o, key, sig
            except Exception:
                raise SignatureVerifyException("Malformed Authorization header")

        auth_headers = request.requestHeaders.getRawHeaders(u"Authorization")

        if not auth_headers:
            raise NoAuthenticationError("Missing Authorization headers")

        for auth in auth_headers:
            if auth.startswith(u"X-Matrix"):
                (a_o, key, sig) = parse_auth_header(auth)
                json_request["origin"] = a_o
                json_request["signatures"].setdefault(a_o, {})[key] = sig

        if not json_request["signatures"]:
            raise NoAuthenticationError("Missing X-Matrix Authorization header")

        yield self.verifyServerSignedJson(json_request, [a_o])

        logger.info("Verified request from HS %s", a_o)

        defer.returnValue(a_o)