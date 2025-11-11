# -*- coding: utf-8 -*-

import logging
from io import BytesIO

import twisted.internet.ssl
from twisted.internet import defer, protocol
from twisted.internet.protocol import connectionDone
from twisted.web._newclient import ResponseDone
from twisted.web.http import PotentialDataLoss
from twisted.web.iweb import UNKNOWN_LENGTH

logger = logging.getLogger(__name__)

class SslComponents:
    def __init__(self, sydent):
        self.sydent = sydent

        self.myPrivateCertificate = self.makeMyCertificate()
        self.trustRoot = self.makeTrustRoot()

    def makeMyCertificate(self):
        privKeyAndCertFilename = self.sydent.cfg.get('http', 'replication.https.certfile')
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if privKeyAndCertFilename == '':
                    dispatcher = 1
                else:
                    dispatcher = 2
            elif dispatcher == 1:
                logger.warn("No HTTPS private key / cert found: not starting replication server "
                            "or doing replication pushes")
                return None
            elif dispatcher == 2:
                try:
                    fp = open(privKeyAndCertFilename)
                    dispatcher = 3
                except IOError:
                    dispatcher = 4
            elif dispatcher == 3:
                authData = fp.read()
                fp.close()
                return twisted.internet.ssl.PrivateCertificate.loadPEM(authData)
            elif dispatcher == 4:
                logger.warn("Unable to read private key / cert file from %s: not starting the replication HTTPS server "
                            "or doing replication pushes.",
                            privKeyAndCertFilename)
                return None

    def makeTrustRoot(self):
        caCertFilename = self.sydent.cfg.get('http', 'replication.https.cacert')
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if len(caCertFilename) > 0:
                    dispatcher = 1
                else:
                    dispatcher = 5
            elif dispatcher == 1:
                try:
                    fp = open(caCertFilename)
                    caCert = twisted.internet.ssl.Certificate.loadPEM(fp.read())
                    fp.close()
                    dispatcher = 2
                except:
                    dispatcher = 3
            elif dispatcher == 2:
                logger.warn("Using custom CA cert file: %s", caCertFilename)
                return twisted.internet._sslverify.OpenSSLCertificateAuthorities([caCert.original])
            elif dispatcher == 3:
                logger.warn("Failed to open CA cert file %s", caCertFilename)
                raise
            elif dispatcher == 5:
                return twisted.internet.ssl.OpenSSLDefaultPaths()


class BodyExceededMaxSize(Exception):
    """The maximum allowed size of the HTTP body was exceeded."""


class _DiscardBodyWithMaxSizeProtocol(protocol.Protocol):
    """A protocol which immediately errors upon receiving data."""

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
    """A protocol which reads body to a stream, erroring if the body exceeds a maximum size."""

    def __init__(self, deferred, max_size):
        self.stream = BytesIO()
        self.deferred = deferred
        self.length = 0
        self.max_size = max_size

    def dataReceived(self, data) -> None:
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if self.deferred.called:
                    return
                dispatcher = 1
            elif dispatcher == 1:
                self.stream.write(data)
                self.length += len(data)
                if self.max_size is not None and self.length >= self.max_size:
                    dispatcher = 2
                else:
                    return
            elif dispatcher == 2:
                self.deferred.errback(BodyExceededMaxSize())
                self.transport.abortConnection()
                return

    def connectionLost(self, reason = connectionDone) -> None:
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if self.deferred.called:
                    return
                dispatcher = 1
            elif dispatcher == 1:
                if reason.check(ResponseDone):
                    dispatcher = 2
                elif reason.check(PotentialDataLoss):
                    dispatcher = 3
                else:
                    dispatcher = 4
            elif dispatcher == 2 or dispatcher == 3:
                self.deferred.callback(self.stream.getvalue())
                return
            elif dispatcher == 4:
                self.deferred.errback(reason)
                return


def read_body_with_max_size(response, max_size):
    d = defer.Deferred()

    dispatcher = 0
    while True:
        if dispatcher == 0:
            if max_size is not None and response.length != UNKNOWN_LENGTH:
                dispatcher = 1
            else:
                dispatcher = 3
        elif dispatcher == 1:
            if response.length > max_size:
                dispatcher = 2
            else:
                dispatcher = 3
        elif dispatcher == 2:
            response.deliverBody(_DiscardBodyWithMaxSizeProtocol(d))
            return d
        elif dispatcher == 3:
            response.deliverBody(_ReadBodyWithMaxSizeProtocol(d, max_size))
            return d