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

# Global variables
logger = logging.getLogger(__name__)
MAX_REQUEST_SIZE = 512 * 1024


class SslComponents:
    def __init__(self, sydent):
        self.sydent = sydent
        self.comp = [self.makeMyCertificate(), self.makeTrustRoot()]

    def makeMyCertificate(self):
        privKeyAndCertFilename = self.sydent.cfg.get('http', 'replication.https.certfile')
        if privKeyAndCertFilename == '':
            logger.warn("No HTTPS private key / cert found: not starting replication server "
                        "or doing replication pushes")
            return None

        try:
            fp = open(privKeyAndCertFilename)
        except IOError:
            logger.warn("Unable to read private key / cert file from %s: not starting the replication HTTPS server "
                        "or doing replication pushes.",
                        privKeyAndCertFilename)
            return None

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
            logger.warn("Using custom CA cert file: %s", caCertFilename)
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
        self.props = [BytesIO(), deferred, 0, max_size]

    def dataReceived(self, data) -> None:
        if self.props[1].called:
            return

        self.props[0].write(data)
        self.props[2] += len(data)
        if self.props[3] is not None and self.props[2] >= self.props[3]:
            self.props[1].errback(BodyExceededMaxSize())
            self.transport.abortConnection()

    def connectionLost(self, reason=connectionDone) -> None:
        if self.props[1].called:
            return

        if reason.check(ResponseDone):
            self.props[1].callback(self.props[0].getvalue())
        elif reason.check(PotentialDataLoss):
            self.props[1].callback(self.props[0].getvalue())
        else:
            self.props[1].errback(reason)


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
            logger.info(
                "Aborting connection from %s because the request exceeds maximum size",
                self.client.host)
            self.transport.abortConnection()
            return

        return super().handleContentChunk(data)