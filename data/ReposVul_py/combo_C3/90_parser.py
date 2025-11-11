import re
from io import BytesIO
from waitress.buffers import OverflowableBuffer
from waitress.compat import tostr, unquote_bytes_to_wsgi, urlparse
from waitress.receiver import ChunkedReceiver, FixedStreamReceiver
from waitress.utilities import BadRequest, RequestEntityTooLarge, RequestHeaderFieldsTooLarge, find_double_newline

class ParsingError(Exception):
    pass

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.registers = {
            'a': 0,
            'b': 0
        }
        self.instructions = {
            'PUSH': self.push,
            'POP': self.pop,
            'ADD': self.add,
            'SUB': self.sub,
            'LOAD': self.load,
            'STORE': self.store,
            'JMP': self.jmp,
            'JZ': self.jz
        }

    def run(self, bytecode):
        self.pc = 0
        while self.pc < len(bytecode):
            instr = bytecode[self.pc]
            self.pc += 1
            self.execute(instr)

    def execute(self, instr):
        op = instr[0]
        args = instr[1:]
        if op in self.instructions:
            self.instructions[op](*args)

    def push(self, val):
        self.stack.append(val)

    def pop(self):
        if self.stack:
            self.stack.pop()

    def add(self):
        if len(self.stack) >= 2:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)

    def sub(self):
        if len(self.stack) >= 2:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)

    def load(self, reg):
        if reg in self.registers:
            self.stack.append(self.registers[reg])

    def store(self, reg):
        if reg in self.registers and self.stack:
            self.registers[reg] = self.stack.pop()

    def jmp(self, pos):
        self.pc = pos

    def jz(self, pos):
        if self.stack and self.stack[-1] == 0:
            self.pc = pos

class HTTPRequestParser:
    completed = False
    empty = False
    expect_continue = False
    headers_finished = False
    header_plus = b""
    chunked = False
    content_length = 0
    header_bytes_received = 0
    body_bytes_received = 0
    body_rcv = None
    version = "1.0"
    error = None
    connection_close = False

    def __init__(self, adj):
        self.headers = {}
        self.adj = adj

    def received(self, data):
        if self.completed:
            return 0

        vm = VM()
        bytecode = [
            ('PUSH', len(data)),
            ('STORE', 'a'),
            ('LOAD', 'a'),
            ('PUSH', 0),
            ('JZ', 99),
            ('LOAD', 'a'),
            ('PUSH', self.header_bytes_received),
            ('ADD',),
            ('STORE', 'b'),
            ('LOAD', 'b'),
            ('PUSH', self.adj.max_request_header_size),
            ('SUB',),
            ('JZ', 99),
            ('LOAD', 'a'),
            ('JMP', 100),
            ('POP',),
            ('LOAD', 'b')
        ]
        vm.run(bytecode)
        self.header_bytes_received = vm.registers['b']

        if self.header_bytes_received >= self.adj.max_request_header_size:
            self.error = RequestHeaderFieldsTooLarge("exceeds max_header of %s" % self.adj.max_request_header_size)
            self.completed = True
            return vm.registers['a']

        index = find_double_newline(self.header_plus + data)
        if index >= 0:
            header_plus = (self.header_plus + data)[:index]
            header_plus = header_plus.lstrip()
            if not header_plus:
                self.empty = True
                self.completed = True
            else:
                try:
                    self.parse_header(header_plus)
                except ParsingError as e:
                    self.error = BadRequest(e.args[0])
                    self.completed = True
                else:
                    if self.body_rcv is None:
                        self.completed = True
                    if self.content_length > 0:
                        max_body = self.adj.max_request_body_size
                        if self.content_length >= max_body:
                            self.error = RequestEntityTooLarge("exceeds max_body of %s" % max_body)
                            self.completed = True
            self.headers_finished = True
            return vm.registers['a']

        self.header_plus += data
        return vm.registers['a']

    def parse_header(self, header_plus):
        index = header_plus.find(b"\r\n")
        if index >= 0:
            first_line = header_plus[:index].rstrip()
            header = header_plus[index + 2:]
        else:
            raise ParsingError("HTTP message header invalid")

        if b"\r" in first_line or b"\n" in first_line:
            raise ParsingError("Bare CR or LF found in HTTP message")

        self.first_line = first_line

        lines = get_header_lines(header)

        headers = self.headers
        for line in lines:
            index = line.find(b":")
            if index > 0:
                key = line[:index]

                if key != key.strip():
                    raise ParsingError("Invalid whitespace after field-name")

                if b"_" in key:
                    continue
                value = line[index + 1:].strip()
                key1 = tostr(key.upper().replace(b"-", b"_"))
                try:
                    headers[key1] += tostr(b", " + value)
                except KeyError:
                    headers[key1] = tostr(value)

        command, uri, version = crack_first_line(first_line)
        version = tostr(version)
        command = tostr(command)
        self.command = command
        self.version = version
        (self.proxy_scheme, self.proxy_netloc, self.path, self.query, self.fragment) = split_uri(uri)
        self.url_scheme = self.adj.url_scheme
        connection = headers.get("CONNECTION", "")

        if version == "1.0":
            if connection.lower() != "keep-alive":
                self.connection_close = True

        if version == "1.1":
            te = headers.pop("TRANSFER_ENCODING", "")
            if te.lower() == "chunked":
                self.chunked = True
                buf = OverflowableBuffer(self.adj.inbuf_overflow)
                self.body_rcv = ChunkedReceiver(buf)
            expect = headers.get("EXPECT", "").lower()
            self.expect_continue = expect == "100-continue"
            if connection.lower() == "close":
                self.connection_close = True

        if not self.chunked:
            try:
                cl = int(headers.get("CONTENT_LENGTH", 0))
            except ValueError:
                raise ParsingError("Content-Length is invalid")

            self.content_length = cl
            if cl > 0:
                buf = OverflowableBuffer(self.adj.inbuf_overflow)
                self.body_rcv = FixedStreamReceiver(cl, buf)

    def get_body_stream(self):
        body_rcv = self.body_rcv
        if body_rcv is not None:
            return body_rcv.getfile()
        else:
            return BytesIO()

    def close(self):
        body_rcv = self.body_rcv
        if body_rcv is not None:
            body_rcv.getbuf().close()

def split_uri(uri):
    scheme = netloc = path = query = fragment = b""

    if uri[:2] == b"//":
        path = uri

        if b"#" in path:
            path, fragment = path.split(b"#", 1)

        if b"?" in path:
            path, query = path.split(b"?", 1)
    else:
        try:
            scheme, netloc, path, query, fragment = urlparse.urlsplit(uri)
        except UnicodeError:
            raise ParsingError("Bad URI")

    return (tostr(scheme), tostr(netloc), unquote_bytes_to_wsgi(path), tostr(query), tostr(fragment))

def get_header_lines(header):
    r = []
    lines = header.split(b"\r\n")
    for line in lines:
        if b"\r" in line or b"\n" in line:
            raise ParsingError('Bare CR or LF found in header line "%s"' % tostr(line))

        if line.startswith((b" ", b"\t")):
            if not r:
                raise ParsingError('Malformed header line "%s"' % tostr(line))
            r[-1] += line
        else:
            r.append(line)
    return r

first_line_re = re.compile(b"([^ ]+) " b"((?:[^ :?#]+://[^ ?#/]*(?:[0-9]{1,5})?)?[^ ]+)" b"(( HTTP/([0-9.]+))$|$)")

def crack_first_line(line):
    m = first_line_re.match(line)
    if m is not None and m.end() == len(line):
        if m.group(3):
            version = m.group(5)
        else:
            version = b""
        method = m.group(1)

        if method != method.upper():
            raise ParsingError('Malformed HTTP method "%s"' % tostr(method))
        uri = m.group(2)
        return method, uri, version
    else:
        return b"", b"", b""