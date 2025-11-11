# -*- coding: utf-8 -*-

import logging
from io import BytesIO

import twisted.internet.ssl
from twisted.internet import defer, protocol
from twisted.internet.protocol import connectionDone
from twisted.web._newclient import ResponseDone
from twisted.web.http import PotentialDataLoss
from twisted.web.iweb import UNKNOWN_LENGTH
from twisted.web import server

logger = logging.getLogger(__name__)

MAX_REQUEST_SIZE = 512 * 1024

class SslComponents:
    def __init__(self, sydent):
        self.sydent = sydent
        self.myPrivateCertificate = self.makeMyCertificate()
        self.trustRoot = self.makeTrustRoot()

    def makeMyCertificate(self):
        privKeyAndCertFilename = self.sydent.cfg.get('http', 'replication.https.certfile')
        dispatch = 0
        while True:
            if dispatch == 0:
                if privKeyAndCertFilename == '':
                    dispatch = 1
                else:
                    dispatch = 2
            elif dispatch == 1:
                logger.warn("No HTTPS private key / cert found: not starting replication server "
                            "or doing replication pushes")
                return None
            elif dispatch == 2:
                try:
                    dispatch = 3
                except IOError:
                    dispatch = 4
            elif dispatch == 3:
                fp = open(privKeyAndCertFilename)
                authData = fp.read()
                fp.close()
                return twisted.internet.ssl.PrivateCertificate.loadPEM(authData)
            elif dispatch == 4:
                logger.warn("Unable to read private key / cert file from %s: not starting the replication HTTPS server "
                            "or doing replication pushes.",
                            privKeyAndCertFilename)
                return None

    def makeTrustRoot(self):
        caCertFilename = self.sydent.cfg.get('http', 'replication.https.cacert')
        dispatch = 0
        while True:
            if dispatch == 0:
                if len(caCertFilename) > 0:
                    dispatch = 1
                else:
                    dispatch = 5
            elif dispatch == 1:
                try:
                    fp = open(caCertFilename)
                    caCert = twisted.internet.ssl.Certificate.loadPEM(fp.read())
                    fp.close()
                    dispatch = 2
                except Exception:
                    dispatch = 3
            elif dispatch == 2:
                logger.warn("Using custom CA cert file: %s", caCertFilename)
                return twisted.internet._sslverify.OpenSSLCertificateAuthorities([caCert.original])
            elif dispatch == 3:
                logger.warn("Failed to open CA cert file %s", caCertFilename)
                raise
            elif dispatch == 5:
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
        dispatch = 0
        while True:
            if dispatch == 0:
                if self.deferred.called:
                    return
                self.stream.write(data)
                self.length += len(data)
                if self.max_size is not None and self.length >= self.max_size:
                    dispatch = 1
                else:
                    return
            elif dispatch == 1:
                self.deferred.errback(BodyExceededMaxSize())
                self.transport.abortConnection()
                return

    def connectionLost(self, reason=connectionDone) -> None:
        dispatch = 0
        while True:
            if dispatch == 0:
                if self.deferred.called:
                    return
                if reason.check(ResponseDone):
                    dispatch = 1
                elif reason.check(PotentialDataLoss):
                    dispatch = 2
                else:
                    dispatch = 3
            elif dispatch == 1 or dispatch == 2:
                self.deferred.callback(self.stream.getvalue())
                return
            elif dispatch == 3:
                self.deferred.errback(reason)
                return

def read_body_with_max_size(response, max_size):
    d = defer.Deferred()
    dispatch = 0
    while True:
        if dispatch == 0:
            if max_size is not None and response.length != UNKNOWN_LENGTH:
                dispatch = 1
            else:
                dispatch = 2
        elif dispatch == 1:
            if response.length > max_size:
                response.deliverBody(_DiscardBodyWithMaxSizeProtocol(d))
                return d
            else:
                dispatch = 2
        elif dispatch == 2:
            response.deliverBody(_ReadBodyWithMaxSizeProtocol(d, max_size))
            return d

class SizeLimitingRequest(server.Request):
    def handleContentChunk(self, data):
        dispatch = 0
        while True:
            if dispatch == 0:
                if self.content.tell() + len(data) > MAX_REQUEST_SIZE:
                    dispatch = 1
                else:
                    dispatch = 2
            elif dispatch == 1:
                logger.info(
                    "Aborting connection from %s because the request exceeds maximum size",
                    self.client.host)
                self.transport.abortConnection()
                return
            elif dispatch == 2:
                return super().handleContentChunk(data)