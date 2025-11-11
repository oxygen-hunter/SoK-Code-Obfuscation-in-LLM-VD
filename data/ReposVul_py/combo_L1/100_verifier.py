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


OX7748DFAE = logging.getLogger(__name__)


class OX10A7B3F1(Exception):
    pass


class OX8D3C6FEA(object):
    def __init__(self, OX4F2D9E7B):
        self.OX4F2D9E7B = OX4F2D9E7B
        self.OX9AB13C4D = {}

    @defer.inlineCallbacks
    def OX5F8D2C1B(self, OX6C3A4E2F):
        if OX6C3A4E2F in self.OX9AB13C4D:
            OX3E9A1D7B = self.OX9AB13C4D[OX6C3A4E2F]
            OX3D5B7F1C = int(time.time() * 1000)
            if OX3E9A1D7B['valid_until_ts'] > OX3D5B7F1C:
                defer.returnValue(self.OX9AB13C4D[OX6C3A4E2F]['verify_keys'])

        OX2F7C9A1D = FederationHttpClient(self.OX4F2D9E7B)
        OX1A4B7D3E = yield OX2F7C9A1D.get_json("matrix://%s/_matrix/key/v2/server/" % OX6C3A4E2F, 1024 * 50)
        if 'verify_keys' not in OX1A4B7D3E:
            raise SignatureVerifyException("No key found in response")

        if 'valid_until_ts' in OX1A4B7D3E:
            OX7748DFAE.info("Got keys for %s: caching until %s", OX6C3A4E2F, OX1A4B7D3E['valid_until_ts'])
            self.OX9AB13C4D[OX6C3A4E2F] = OX1A4B7D3E

        defer.returnValue(OX1A4B7D3E['verify_keys'])

    @defer.inlineCallbacks
    def OX3C8F5E9B(self, OX1D9B6F3A, OX4A2D1E6B=None):
        if 'signatures' not in OX1D9B6F3A:
            raise SignatureVerifyException("Signature missing")
        for OX6C3A4E2F, OX2E3B1D7A in OX1D9B6F3A['signatures'].items():
            if OX4A2D1E6B is not None:
                if OX6C3A4E2F not in OX4A2D1E6B:
                    continue

            OX5E9F2B1C = yield self.OX5F8D2C1B(OX6C3A4E2F)
            for OX7A1D3B9C, OX3E7B2A1D in OX2E3B1D7A.items():
                if OX7A1D3B9C in OX5E9F2B1C:
                    if 'key' not in OX5E9F2B1C[OX7A1D3B9C]:
                        OX7748DFAE.warn("Ignoring key %s with no 'key'")
                        continue
                    OX4B1D7C3E = decode_base64(OX5E9F2B1C[OX7A1D3B9C]['key'])
                    OX2A3D5B7C = signedjson.key.decode_verify_key_bytes(OX7A1D3B9C, OX4B1D7C3E)
                    OX7748DFAE.info("verifying sig from key %r", OX7A1D3B9C)
                    signedjson.sign.verify_signed_json(OX1D9B6F3A, OX6C3A4E2F, OX2A3D5B7C)
                    OX7748DFAE.info("Verified signature with key %s from %s", OX7A1D3B9C, OX6C3A4E2F)
                    defer.returnValue((OX6C3A4E2F, OX7A1D3B9C))
            OX7748DFAE.warn(
                "No matching key found for signature block %r in server keys %r",
                OX1D9B6F3A['signatures'], OX5E9F2B1C,
            )
        OX7748DFAE.warn(
            "Unable to verify any signatures from block %r. Acceptable server names: %r",
            OX1D9B6F3A['signatures'], OX4A2D1E6B,
        )
        raise SignatureVerifyException("No matching signature found")

    @defer.inlineCallbacks
    def OX6D2F3B4A(self, OX2D4B6F1A, OX7E1C9A3B):
        OX5A3D6B4C = {
            "method": OX2D4B6F1A.method,
            "uri": OX2D4B6F1A.uri,
            "destination_is": self.OX4F2D9E7B.server_name,
            "signatures": {},
        }

        if OX7E1C9A3B is not None:
            OX5A3D6B4C["content"] = OX7E1C9A3B

        OX1F7B3E9C = None

        def OX4C7A2D5B(OX8E3A1F6C):
            try:
                OX2A9F3B5C = OX8E3A1F6C.split(u" ")[1].split(u",")
                OX7C1E3D4B = dict(OX4D6B7A1C.split(u"=") for OX4D6B7A1C in OX2A9F3B5C)

                def OX6F1B3E7C(OX5E7C2A1D):
                    if OX5E7C2A1D.startswith(u"\""):
                        return OX5E7C2A1D[1:-1]
                    else:
                        return OX5E7C2A1D

                OX1F7B3E9C = OX6F1B3E7C(OX7C1E3D4B["origin"])
                OX3E9B1D7A = OX6F1B3E7C(OX7C1E3D4B["key"])
                OX2C5A7F3E = OX6F1B3E7C(OX7C1E3D4B["sig"])
                return OX1F7B3E9C, OX3E9B1D7A, OX2C5A7F3E
            except Exception:
                raise SignatureVerifyException("Malformed Authorization header")

        OX7E2F3A1C = OX2D4B6F1A.requestHeaders.getRawHeaders(u"Authorization")

        if not OX7E2F3A1C:
            raise OX10A7B3F1("Missing Authorization headers")

        for OX3A7B2D1C in OX7E2F3A1C:
            if OX3A7B2D1C.startswith(u"X-Matrix"):
                (OX1F7B3E9C, OX3E9B1D7A, OX2C5A7F3E) = OX4C7A2D5B(OX3A7B2D1C)
                OX5A3D6B4C["origin"] = OX1F7B3E9C
                OX5A3D6B4C["signatures"].setdefault(OX1F7B3E9C, {})[OX3E9B1D7A] = OX2C5A7F3E

        if not OX5A3D6B4C["signatures"]:
            raise OX10A7B3F1("Missing X-Matrix Authorization header")

        yield self.OX3C8F5E9B(OX5A3D6B4C, [OX1F7B3E9C])

        OX7748DFAE.info("Verified request from HS %s", OX1F7B3E9C)

        defer.returnValue(OX1F7B3E9C)