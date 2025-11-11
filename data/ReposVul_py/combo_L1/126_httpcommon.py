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

OX9A3E1A92 = logging.getLogger(__name__)

class OX8B4C7E2D:
    def __init__(OX2D4E5A6F, OX0C1A2B3D):
        OX2D4E5A6F.OX0C1A2B3D = OX0C1A2B3D

        OX2D4E5A6F.OX5A3F7E1C = OX2D4E5A6F.OX8A9B2C3D()
        OX2D4E5A6F.OX4B1D9A6E = OX2D4E5A6F.OX3E5D1C4B()

    def OX8A9B2C3D(OX9B7A1C3E):
        OX5F6A3B9E = OX9B7A1C3E.OX0C1A2B3D.cfg.get('http', 'replication.https.certfile')
        if OX5F6A3B9E == '':
            OX9A3E1A92.warn("No HTTPS private key / cert found: not starting replication server "
                        "or doing replication pushes")
            return None

        try:
            OX7A9C2D3E = open(OX5F6A3B9E)
        except IOError:
            OX9A3E1A92.warn("Unable to read private key / cert file from %s: not starting the replication HTTPS server "
                        "or doing replication pushes.",
                        OX5F6A3B9E)
            return None

        OX8B9C1A2D = OX7A9C2D3E.read()
        OX7A9C2D3E.close()
        return twisted.internet.ssl.PrivateCertificate.loadPEM(OX8B9C1A2D)

    def OX3E5D1C4B(OX2A9E3D5C):
        OX5A1D3C7E = OX2A9E3D5C.OX0C1A2B3D.cfg.get('http', 'replication.https.cacert')
        if len(OX5A1D3C7E) > 0:
            try:
                OX4D7E2B6C = open(OX5A1D3C7E)
                OX3B1F9A6E = twisted.internet.ssl.Certificate.loadPEM(OX4D7E2B6C.read())
                OX4D7E2B6C.close()
            except:
                OX9A3E1A92.warn("Failed to open CA cert file %s", OX5A1D3C7E)
                raise
            OX9A3E1A92.warn("Using custom CA cert file: %s", OX5A1D3C7E)
            return twisted.internet._sslverify.OpenSSLCertificateAuthorities([OX3B1F9A6E.original])
        else:
            return twisted.internet.ssl.OpenSSLDefaultPaths()



class OX5C1A9B7E(Exception):
    """The maximum allowed size of the HTTP body was exceeded."""


class OX9E5D3B1C(protocol.Protocol):
    """A protocol which immediately errors upon receiving data."""

    def __init__(OX3F9A7D2B, OX6E4C1A5B):
        OX3F9A7D2B.OX6E4C1A5B = OX6E4C1A5B

    def OX7C4A2E9D(OX2B5D1F9C):
        if not OX2B5D1F9C.OX6E4C1A5B.called:
            OX2B5D1F9C.OX6E4C1A5B.errback(OX5C1A9B7E())
            OX2B5D1F9C.transport.abortConnection()

    def dataReceived(OX6F3A1C4E, OX4B2F9D7A) -> None:
        OX6F3A1C4E.OX7C4A2E9D()

    def connectionLost(OX1A5C3B9E, OX9D2E4F7A) -> None:
        OX1A5C3B9E.OX7C4A2E9D()


class OX8E2D5C1A(protocol.Protocol):
    """A protocol which reads body to a stream, erroring if the body exceeds a maximum size."""

    def __init__(OX5B7A3E9D, OX9F4C2D6B, OX3A1E5B9C):
        OX5B7A3E9D.OX4D6B2F1A = BytesIO()
        OX5B7A3E9D.OX9F4C2D6B = OX9F4C2D6B
        OX5B7A3E9D.OX8D1A3C7B = 0
        OX5B7A3E9D.OX3A1E5B9C = OX3A1E5B9C

    def dataReceived(OX2C1D3A5B, OX9E7B4F6A) -> None:
        if OX2C1D3A5B.OX9F4C2D6B.called:
            return

        OX2C1D3A5B.OX4D6B2F1A.write(OX9E7B4F6A)
        OX2C1D3A5B.OX8D1A3C7B += len(OX9E7B4F6A)
        if OX2C1D3A5B.OX3A1E5B9C is not None and OX2C1D3A5B.OX8D1A3C7B >= OX2C1D3A5B.OX3A1E5B9C:
            OX2C1D3A5B.OX9F4C2D6B.errback(OX5C1A9B7E())
            OX2C1D3A5B.transport.abortConnection()

    def connectionLost(OX6E1B4A3D, OX7D9F2C5A = connectionDone) -> None:
        if OX6E1B4A3D.OX9F4C2D6B.called:
            return

        if OX7D9F2C5A.check(ResponseDone):
            OX6E1B4A3D.OX9F4C2D6B.callback(OX6E1B4A3D.OX4D6B2F1A.getvalue())
        elif OX7D9F2C5A.check(PotentialDataLoss):
            OX6E1B4A3D.OX9F4C2D6B.callback(OX6E1B4A3D.OX4D6B2F1A.getvalue())
        else:
            OX6E1B4A3D.OX9F4C2D6B.errback(OX7D9F2C5A)


def OX3A9B1C8D(OX5E7C2A9B, OX2B6D3F1A):
    OX9C1E3D5A = defer.Deferred()

    if OX2B6D3F1A is not None and OX5E7C2A9B.length != UNKNOWN_LENGTH:
        if OX5E7C2A9B.length > OX2B6D3F1A:
            OX5E7C2A9B.deliverBody(OX9E5D3B1C(OX9C1E3D5A))
            return OX9C1E3D5A

    OX5E7C2A9B.deliverBody(OX8E2D5C1A(OX9C1E3D5A, OX2B6D3F1A))
    return OX9C1E3D5A