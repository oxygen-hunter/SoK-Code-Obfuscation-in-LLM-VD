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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.running = True

    def run(self, instructions):
        while self.running:
            opcode, *args = instructions[self.pc]
            self.pc += 1
            getattr(self, f"op_{opcode.lower()}")(*args)

    def op_push(self, value):
        self.stack.append(value)

    def op_pop(self):
        return self.stack.pop()

    def op_add(self):
        self.stack.append(self.op_pop() + self.op_pop())

    def op_sub(self):
        b, a = self.op_pop(), self.op_pop()
        self.stack.append(a - b)

    def op_load(self, idx):
        self.stack.append(self.stack[idx])

    def op_store(self, idx):
        self.stack[idx] = self.op_pop()

    def op_jmp(self, addr):
        self.pc = addr

    def op_jz(self, addr):
        if self.op_pop() == 0:
            self.pc = addr

    def op_halt(self):
        self.running = False

class SslComponents:
    def __init__(self, sydent):
        self.sydent = sydent
        self.vm = VM()
        self.vm.run([
            ('PUSH', self.makeMyCertificate),
            ('PUSH', self.makeTrustRoot),
            ('PUSH', sydent),
            ('STORE', 0),
            ('STORE', 1),
            ('STORE', 2),
            ('PUSH', 0),
            ('LOAD', 0),
            ('LOAD', 1),
            ('HALT',)
        ])

    def makeMyCertificate(self):
        vm = VM()
        vm.run([
            ('PUSH', self.sydent.cfg.get('http', 'replication.https.certfile')),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', ''),
            ('SUB',),
            ('JZ', 5),
            ('HALT',)
        ])
        privKeyAndCertFilename = vm.op_pop()
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
        vm = VM()
        vm.run([
            ('PUSH', self.sydent.cfg.get('http', 'replication.https.cacert')),
            ('STORE', 0),
            ('LOAD', 0),
            ('PUSH', 0),
            ('SUB',),
            ('JZ', 4),
            ('HALT',)
        ])
        caCertFilename = vm.op_pop()
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
        self.vm = VM()
        self.vm.run([
            ('PUSH', deferred),
            ('STORE', 0),
        ])

    def _maybe_fail(self):
        vm = VM()
        vm.run([
            ('LOAD', 0),
            ('PUSH', BodyExceededMaxSize()),
            ('STORE', 1),
            ('LOAD', 0),
            ('PUSH', 'called'),
            ('LOAD', 1),
            ('JZ', 5),
            ('HALT',)
        ])
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
            logger.info(
                "Aborting connection from %s because the request exceeds maximum size",
                self.client.host)
            self.transport.abortConnection()
            return
        return super().handleContentChunk(data)