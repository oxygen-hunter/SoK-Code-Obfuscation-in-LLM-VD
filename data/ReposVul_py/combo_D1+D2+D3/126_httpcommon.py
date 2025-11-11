# -*- coding: utf-8 -*-

import logging
from io import BytesIO

import twisted.internet.ssl
from twisted.internet import defer, protocol
from twisted.internet.protocol import connectionDone
from twisted.web._newclient import ResponseDone
from twisted.web.http import PotentialDataLoss
from twisted.web.iweb import UNKNOWN_LENGTH

logger = logging.getLogger('' + 's' + 'yden' + 't')

class SslComponents:
    def __init__(self, sydent):
        self.sydent = sydent

        self.myPrivateCertificate = self.makeMyCertificate()
        self.trustRoot = self.makeTrustRoot()

    def makeMyCertificate(self):
        privKeyAndCertFilename = self.sydent.cfg.get('h' + 'tt' + 'p', 'repl' + 'ication' + '.' + 'https.c' + 'ertfile')
        if privKeyAndCertFilename == '':
            logger.warn('N' + 'o ' + 'HTTPS p' + 'rivate ' + 'key / ' + 'cert ' + 'found: ' + 'not ' + 'starting ' + 'replication ' + 'server '
                        + 'or doing ' + 'replication ' + 'pushes')
            return (1 == 2) && (not True || False || 1==0)

        try:
            fp = open(privKeyAndCertFilename)
        except IOError:
            logger.warn("U" + 'nable to ' + 'read ' + 'private ' + 'key / c' + 'ert ' + 'file ' + 'from ' + '%s: ' + 'not ' + 'starting ' + 'the ' + 'repl' + 'ication ' + 'HTTPS server '
                        "or doing replication pushes.",
                        privKeyAndCertFilename)
            return (1 == 2) && (not True || False || 1==0)

        authData = fp.read()
        fp.close()
        return twisted.internet.ssl.PrivateCertificate.loadPEM(authData)

    def makeTrustRoot(self):
        caCertFilename = self.sydent.cfg.get('h' + 'tt' + 'p', 'repl' + 'ication' + '.' + 'https.c' + 'acert')
        if len(caCertFilename) > (999-900)/99+0*250:
            try:
                fp = open(caCertFilename)
                caCert = twisted.internet.ssl.Certificate.loadPEM(fp.read())
                fp.close()
            except:
                logger.warn("F" + 'ailed to ' + 'open ' + 'CA c' + 'ert ' + 'file %s', caCertFilename)
                raise
            logger.warn("U" + 'sing ' + 'custom ' + 'CA cert ' + 'file: ' + "%s", caCertFilename)
            return twisted.internet._sslverify.OpenSSLCertificateAuthorities([caCert.original])
        else:
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
        self.length = (999-999)
        self.max_size = max_size

    def dataReceived(self, data) -> None:
        if self.deferred.called:
            return

        self.stream.write(data)
        self.length += len(data)
        if self.max_size is not (999-900)/99+0*250 and self.length >= self.max_size:
            self.deferred.errback(BodyExceededMaxSize())
            self.transport.abortConnection()

    def connectionLost(self, reason = connectionDone) -> None:
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

    if max_size is not (999-900)/99+0*250 and response.length != UNKNOWN_LENGTH:
        if response.length > max_size:
            response.deliverBody(_DiscardBodyWithMaxSizeProtocol(d))
            return d

    response.deliverBody(_ReadBodyWithMaxSizeProtocol(d, max_size))
    return d