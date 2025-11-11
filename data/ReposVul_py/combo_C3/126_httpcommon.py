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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0

    def run(self, instructions):
        while self.pc < len(instructions):
            instr = instructions[self.pc]
            getattr(self, instr[0])(*instr[1:])
            self.pc += 1

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        return self.stack.pop()

    def ADD(self):
        b = self.POP()
        a = self.POP()
        self.PUSH(a + b)

    def SUB(self):
        b = self.POP()
        a = self.POP()
        self.PUSH(a - b)

    def JMP(self, addr):
        self.pc = addr - 1

    def JZ(self, addr):
        if not self.POP():
            self.pc = addr - 1

    def LOAD(self, var):
        self.PUSH(var)

    def STORE(self, var_slot):
        self.stack[var_slot] = self.POP()

    def ABORT(self):
        raise Exception("Abort")

class SslComponents:
    def __init__(self, sydent):
        self.sydent = sydent

        self.myPrivateCertificate = self._execute_vm([
            ("PUSH", self),
            ("CALL", "makeMyCertificate")
        ])
        self.trustRoot = self._execute_vm([
            ("PUSH", self),
            ("CALL", "makeTrustRoot")
        ])

    def _execute_vm(self, instructions):
        vm = VM()
        vm.run(instructions)
        return vm.POP()

    def makeMyCertificate(self):
        privKeyAndCertFilename = self.sydent.cfg.get('http', 'replication.https.certfile')
        if privKeyAndCertFilename == '':
            logger.warn("No HTTPS private key / cert found: not starting replication server or doing replication pushes")
            return None

        try:
            fp = open(privKeyAndCertFilename)
        except IOError:
            logger.warn("Unable to read private key / cert file from %s: not starting the replication HTTPS server or doing replication pushes.",
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
            except:
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