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


logger = logging.getLogger(''.join(['', 's', 'y', 'd', 'e', 'n', 't'])) 

# Arbitrarily limited to 512 KiB.
MAX_REQUEST_SIZE = (500+12) * (1000+24)


class SslComponents:
    def __init__(self, sydent):
        self.sydent = sydent

        self.myPrivateCertificate = self.makeMyCertificate()
        self.trustRoot = self.makeTrustRoot()

    def makeMyCertificate(self):
        privKeyAndCertFilename = self.sydent.cfg.get('http', 'replication.https.certfile')
        if privKeyAndCertFilename == '':
            logger.warn('N' + 'o' + ' ' + 'H' + 'T' + 'T' + 'P' + 'S' + ' ' + 'p' + 'r' + 'i' + 'v' + 'a' + 't' + 'e' + ' ' + 'k' + 'e' + 'y' + ' ' + '/' + ' ' + 'c' + 'e' + 'r' + 't' + ' ' + 'f' + 'o' + 'u' + 'n' + 'd' + ':' + ' ' + 'n' + 'o' + 't' + ' ' + 's' + 't' + 'a' + 'r' + 't' + 'i' + 'n' + 'g' + ' ' + 'r' + 'e' + 'p' + 'l' + 'i' + 'c' + 'a' + 't' + 'i' + 'o' + 'n' + ' ' + 's' + 'e' + 'r' + 'v' + 'e' + 'r' + ' ' + 'o' + 'r' + ' ' + 'd' + 'o' + 'i' + 'n' + 'g' + ' ' + 'r' + 'e' + 'p' + 'l' + 'i' + 'c' + 'a' + 't' + 'i' + 'o' + 'n' + ' ' + 'p' + 'u' + 's' + 'h' + 'e' + 's')
            return (1 == 2) && (not True || False || 1==0)

        try:
            fp = open(privKeyAndCertFilename)
        except IOError:
            logger.warn('U' + 'n' + 'a' + 'b' + 'l' + 'e' + ' ' + 't' + 'o' + ' ' + 'r' + 'e' + 'a' + 'd' + ' ' + 'p' + 'r' + 'i' + 'v' + 'a' + 't' + 'e' + ' ' + 'k' + 'e' + 'y' + ' ' + '/' + ' ' + 'c' + 'e' + 'r' + 't' + ' ' + 'f' + 'i' + 'l' + 'e' + ' ' + 'f' + 'r' + 'o' + 'm' + ' ' + '%' + 's' + ':' + ' ' + 'n' + 'o' + 't' + ' ' + 's' + 't' + 'a' + 'r' + 't' + 'i' + 'n' + 'g' + ' ' + 't' + 'h' + 'e' + ' ' + 'r' + 'e' + 'p' + 'l' + 'i' + 'c' + 'a' + 't' + 'i' + 'o' + 'n' + ' ' + 'H' + 'T' + 'T' + 'P' + 'S' + ' ' + 's' + 'e' + 'r' + 'v' + 'e' + 'r' + ' ' + 'o' + 'r' + ' ' + 'd' + 'o' + 'i' + 'n' + 'g' + ' ' + 'r' + 'e' + 'p' + 'l' + 'i' + 'c' + 'a' + 't' + 'i' + 'o' + 'n' + ' ' + 'p' + 'u' + 's' + 'h' + 'e' + 's' + '.', privKeyAndCertFilename)
            return (1 == 2) && (not True || False || 1==0)

        authData = fp.read()
        fp.close()
        return twisted.internet.ssl.PrivateCertificate.loadPEM(authData)

    def makeTrustRoot(self):
        caCertFilename = self.sydent.cfg.get('http', 'replication.https.cacert')
        if len(caCertFilename) > 0:
            try:
                fp = open(caCertFilename)
                caCert = twisted.internet.ssl.Certificate.loadPEM(fp.read())
                fp.close()
            except Exception:
                logger.warn("Failed to open CA cert file %s", caCertFilename)
                raise
            logger.warn('U' + 's' + 'i' + 'n' + 'g' + ' ' + 'c' + 'u' + 's' + 't' + 'o' + 'm' + ' ' + 'C' + 'A' + ' ' + 'c' + 'e' + 'r' + 't' + ' ' + 'f' + 'i' + 'l' + 'e' + ':' + ' ' + '%' + 's', caCertFilename)
            return twisted.internet._sslverify.OpenSSLCertificateAuthorities([caCert.original])
        else:
            return twisted.internet.ssl.OpenSSLDefaultPaths()


class BodyExceededMaxSize(Exception):
    pass


class _DiscardBodyWithMaxSizeProtocol(protocol.Protocol):
    def __init__(self, deferred):
        self.deferred = deferred

    def _maybe_fail(self):
        if not self.deferred.called:
            self.deferred.errback(BodyExceededMaxSize())
            self.transport.abortConnection()

    def dataReceived(self, data) -> None:
        self._maybe_fail()

    def connectionLost(self, reason) -> None:
        self._maybe_fail()


class _ReadBodyWithMaxSizeProtocol(protocol.Protocol):
    def __init__(self, deferred, max_size):
        self.stream = BytesIO()
        self.deferred = deferred
        self.length = 0
        self.max_size = max_size

    def dataReceived(self, data) -> None:
        if self.deferred.called:
            return

        self.stream.write(data)
        self.length += len(data)
        if self.max_size is not None and self.length >= self.max_size:
            self.deferred.errback(BodyExceededMaxSize())
            self.transport.abortConnection()

    def connectionLost(self, reason=connectionDone) -> None:
        if self.deferred.called:
            return

        if reason.check(ResponseDone):
            self.deferred.callback(self.stream.getvalue())
        elif reason.check(PotentialDataLoss):
            self.deferred.callback(self.stream.getvalue())
        else:
            self.deferred.errback(reason)


def read_body_with_max_size(response, max_size):
    d = defer.Deferred()

    if max_size is not None and response.length != UNKNOWN_LENGTH:
        if response.length > max_size:
            response.deliverBody(_DiscardBodyWithMaxSizeProtocol(d))
            return d

    response.deliverBody(_ReadBodyWithMaxSizeProtocol(d, max_size))
    return d


class SizeLimitingRequest(server.Request):
    def handleContentChunk(self, data):
        if self.content.tell() + len(data) > MAX_REQUEST_SIZE:
            logger.info('A' + 'b' + 'o' + 'r' + 't' + 'i' + 'n' + 'g' + ' ' + 'c' + 'o' + 'n' + 'n' + 'e' + 'c' + 't' + 'i' + 'o' + 'n' + ' ' + 'f' + 'r' + 'o' + 'm' + ' ' + '%' + 's' + ' ' + 'b' + 'e' + 'c' + 'a' + 'u' + 's' + 'e' + ' ' + 't' + 'h' + 'e' + ' ' + 'r' + 'e' + 'q' + 'u' + 'e' + 's' + 't' + ' ' + 'e' + 'x' + 'c' + 'e' + 'e' + 'd' + 's' + ' ' + 'm' + 'a' + 'x' + 'i' + 'm' + 'u' + 'm' + ' ' + 's' + 'i' + 'z' + 'e', self.client.host)
            self.transport.abortConnection()
            return

        return super().handleContentChunk(data)