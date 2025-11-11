import re
from io import BytesIO
from waitress.buffers import OverflowableBuffer
from waitress.compat import tostr, unquote_bytes_to_wsgi, urlparse
from waitress.receiver import ChunkedReceiver, FixedStreamReceiver
from waitress.utilities import (
    BadRequest,
    RequestEntityTooLarge,
    RequestHeaderFieldsTooLarge,
    find_double_newline,
)

class ParsingError(Exception):
    pass

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.context = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            op = instr[0]
            args = instr[1:]
            getattr(self, f"op_{op}")(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, addr):
        self.pc = addr - 1

    def op_JZ(self, addr):
        if self.stack.pop() == 0:
            self.pc = addr - 1

    def op_LOAD(self, name):
        self.stack.append(self.context.get(name, 0))

    def op_STORE(self, name):
        self.context[name] = self.stack.pop()


class HTTPRequestParser(object):
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
        self.vm = VM()

    def received(self, data):
        self.vm.load_program([
            ('PUSH', self.completed),
            ('JZ', 4),  # If completed is False, jump to instruction 4
            ('PUSH', 0),
            ('JMP', 28),  # Jump to end of function
            ('PUSH', len(data)),
            ('STORE', 'datalen'),
            ('LOAD', 'body_rcv'),
            ('PUSH', None),
            ('JZ', 13),  # If br is None, jump to instruction 13
            ('LOAD', 'body_rcv'),
            ('LOAD', 'data'),
            ('CALL_METHOD', 'received', 1),  # br.received(data)
            ('STORE', 'consumed'),
            ('LOAD', 'body_bytes_received'),
            ('LOAD', 'consumed'),
            ('ADD',),
            ('STORE', 'body_bytes_received'),
            # ... More VM instructions to handle the body...
            ('JMP', 28),  # Jump to end of function
            ('LOAD', 'header_plus'),
            ('LOAD', 'data'),
            ('ADD',),
            ('STORE', 's'),
            ('LOAD', 's'),
            ('CALL_FUNC', 'find_double_newline', 1),  # find_double_newline(s)
            ('STORE', 'index'),
            ('LOAD', 'index'),
            ('PUSH', -1),
            ('JZ', 28),  # If index is -1, jump to end of function
            # ... More VM instructions to handle the header...
        ])
        self.vm.run()
        return self.vm.context.get('consumed', 0)

    def parse_header(self, header_plus):
        self.vm.load_program([
            ('LOAD', 'header_plus'),
            ('CALL_METHOD', 'find', b'\r\n'),  # header_plus.find(b"\r\n")
            ('STORE', 'index'),
            ('LOAD', 'index'),
            ('PUSH', -1),
            ('JZ', 3),  # If index is -1, raise ParsingError
            ('LOAD', 'header_plus'),
            ('PUSH', 0),
            ('LOAD', 'index'),
            ('SLICE',),
            ('CALL_METHOD', 'rstrip'),  # header_plus[:index].rstrip()
            ('STORE', 'first_line'),
            ('LOAD', 'header_plus'),
            ('LOAD', 'index'),
            ('PUSH', 2),
            ('ADD',),
            ('SLICE',),
            ('STORE', 'header'),
            # ... More VM instructions for parsing headers...
        ])
        self.vm.run()

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
    return (
        tostr(scheme),
        tostr(netloc),
        unquote_bytes_to_wsgi(path),
        tostr(query),
        tostr(fragment),
    )

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

first_line_re = re.compile(
    b"([^ ]+) "
    b"((?:[^ :?#]+://[^ ?#/]*(?:[0-9]{1,5})?)?[^ ]+)"
    b"(( HTTP/([0-9.]+))$|$)"
)

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