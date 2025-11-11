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

import logging
from io import BytesIO

import twisted.internet.ssl
from twisted.internet import defer, protocol
from twisted.internet.protocol import connectionDone
from twisted.web._newclient import ResponseDone
from twisted.web.http import PotentialDataLoss
from twisted.web.iweb import UNKNOWN_LENGTH
from twisted.web import server


OX7B4DF339 = logging.getLogger(__name__)

OX3E1A0F1A = 512 * 1024


class OX8F4A7F4E:
    def __init__(self, OX0A6E8E6F):
        self.OX0A6E8E6F = OX0A6E8E6F

        self.OX5F0B9D60 = self.OX2498B9C9()
        self.OX4B5C6A18 = self.OX8E8D7B7B()

    def OX2498B9C9(self):
        OX3D2D3C2B = self.OX0A6E8E6F.cfg.get('http', 'replication.https.certfile')
        if OX3D2D3C2B == '':
            OX7B4DF339.warn("No HTTPS private key / cert found: not starting replication server "
                            "or doing replication pushes")
            return None

        try:
            OX7A9E4B3C = open(OX3D2D3C2B)
        except IOError:
            OX7B4DF339.warn("Unable to read private key / cert file from %s: not starting the replication HTTPS server "
                            "or doing replication pushes.",
                            OX3D2D3C2B)
            return None

        OX2F3E4D5C = OX7A9E4B3C.read()
        OX7A9E4B3C.close()
        return twisted.internet.ssl.PrivateCertificate.loadPEM(OX2F3E4D5C)

    def OX8E8D7B7B(self):
        OX2A1B0C9D = self.OX0A6E8E6F.cfg.get('http', 'replication.https.cacert')
        if len(OX2A1B0C9D) > 0:
            try:
                OX0E1F2D3A = open(OX2A1B0C9D)
                OX1F0D2E3B = twisted.internet.ssl.Certificate.loadPEM(OX0E1F2D3A.read())
                OX0E1F2D3A.close()
            except Exception:
                OX7B4DF339.warn("Failed to open CA cert file %s", OX2A1B0C9D)
                raise
            OX7B4DF339.warn("Using custom CA cert file: %s", OX2A1B0C9D)
            return twisted.internet._sslverify.OpenSSLCertificateAuthorities([OX1F0D2E3B.original])
        else:
            return twisted.internet.ssl.OpenSSLDefaultPaths()


class OX5D4C3B2A(Exception):
    pass


class OX9E8D7C6F(protocol.Protocol):
    def __init__(self, OX1A2B3C4D):
        self.OX1A2B3C4D = OX1A2B3C4D

    def OX7B6A5D4C(self):
        if not self.OX1A2B3C4D.called:
            self.OX1A2B3C4D.errback(OX5D4C3B2A())
            self.transport.abortConnection()

    def dataReceived(self, OX2C3D4E5F) -> None:
        self.OX7B6A5D4C()

    def connectionLost(self, OX9F8E7D6C) -> None:
        self.OX7B6A5D4C()


class OX0D1E2F3A(protocol.Protocol):
    def __init__(self, OX5C4B3A2D, OX6B7A8F9E):
        self.OX1F2E3D4C = BytesIO()
        self.OX5C4B3A2D = OX5C4B3A2D
        self.OX4D3C2B1A = 0
        self.OX6B7A8F9E = OX6B7A8F9E

    def dataReceived(self, OX3A2B1C0D) -> None:
        if self.OX5C4B3A2D.called:
            return

        self.OX1F2E3D4C.write(OX3A2B1C0D)
        self.OX4D3C2B1A += len(OX3A2B1C0D)
        if self.OX6B7A8F9E is not None and self.OX4D3C2B1A >= self.OX6B7A8F9E:
            self.OX5C4B3A2D.errback(OX5D4C3B2A())
            self.transport.abortConnection()

    def connectionLost(self, OX7D6C5B4A=connectionDone) -> None:
        if self.OX5C4B3A2D.called:
            return

        if OX7D6C5B4A.check(ResponseDone):
            self.OX5C4B3A2D.callback(self.OX1F2E3D4C.getvalue())
        elif OX7D6C5B4A.check(PotentialDataLoss):
            self.OX5C4B3A2D.callback(self.OX1F2E3D4C.getvalue())
        else:
            self.OX5C4B3A2D.errback(OX7D6C5B4A)


def OX3B2A1C0D(OX9A8B7C6D, OX5E4F3D2C):
    OX6C5D4E3B = defer.Deferred()

    if OX5E4F3D2C is not None and OX9A8B7C6D.length != UNKNOWN_LENGTH:
        if OX9A8B7C6D.length > OX5E4F3D2C:
            OX9A8B7C6D.deliverBody(OX9E8D7C6F(OX6C5D4E3B))
            return OX6C5D4E3B

    OX9A8B7C6D.deliverBody(OX0D1E2F3A(OX6C5D4E3B, OX5E4F3D2C))
    return OX6C5D4E3B


class OX9C8B7A6D(server.Request):
    def handleContentChunk(self, OX8F7E6D5C):
        if self.content.tell() + len(OX8F7E6D5C) > OX3E1A0F1A:
            OX7B4DF339.info(
                "Aborting connection from %s because the request exceeds maximum size",
                self.client.host)
            self.transport.abortConnection()
            return

        return super().handleContentChunk(OX8F7E6D5C)