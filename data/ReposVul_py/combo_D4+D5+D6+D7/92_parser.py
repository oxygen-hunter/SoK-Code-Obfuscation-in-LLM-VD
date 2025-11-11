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

class HTTPRequestParser(object):
    _global_flags = [False, False, False, False, False]
    _global_ints = [0, 0, 0]
    _global_strs = ["", "1.0", None]

    def __init__(self, adj):
        self.headers = {}
        self.adj = adj

    def received(self, data):
        if self._global_flags[0]:
            return 0
        datalen = len(data)
        br = self._global_strs[2]
        if br is None:
            s = self._global_strs[0] + data
            index = find_double_newline(s)
            if index >= 0:
                self._global_strs[0] = s[:index]
                consumed = len(data) - (len(s) - index)

                self._global_strs[0] = self._global_strs[0].lstrip()

                if not self._global_strs[0]:
                    self._global_flags[1] = True
                    self._global_flags[0] = True
                else:
                    try:
                        self.parse_header(self._global_strs[0])
                    except ParsingError as e:
                        self._global_strs[1] = BadRequest(e.args[0])
                        self._global_flags[0] = True
                    else:
                        if self._global_strs[2] is None:
                            self._global_flags[0] = True
                        if self._global_ints[0] > 0:
                            max_body = self.adj.max_request_body_size
                            if self._global_ints[0] >= max_body:
                                self._global_strs[1] = RequestEntityTooLarge(
                                    "exceeds max_body of %s" % max_body
                                )
                                self._global_flags[0] = True
                self._global_flags[2] = True
                return consumed
            else:
                self._global_ints[1] += datalen
                max_header = self.adj.max_request_header_size
                if self._global_ints[1] >= max_header:
                    self.parse_header(b"GET / HTTP/1.0\n")
                    self._global_strs[1] = RequestHeaderFieldsTooLarge(
                        "exceeds max_header of %s" % max_header
                    )
                    self._global_flags[0] = True
                self._global_strs[0] = s
                return datalen
        else:
            consumed = br.received(data)
            self._global_ints[2] += consumed
            max_body = self.adj.max_request_body_size
            if self._global_ints[2] >= max_body:
                self._global_strs[1] = RequestEntityTooLarge("exceeds max_body of %s" % max_body)
                self._global_flags[0] = True
            elif br.error:
                self._global_strs[1] = br.error
                self._global_flags[0] = True
            elif br.completed:
                self._global_flags[0] = True
                if self._global_flags[4]:
                    self.headers["CONTENT_LENGTH"] = str(br.__len__())
            return consumed

    def parse_header(self, header_plus):
        index = header_plus.find(b"\r\n")
        if index >= 0:
            first_line = header_plus[:index].rstrip()
            header = header_plus[index + 2 :]
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
                if b"_" in key:
                    continue
                value = line[index + 1 :].strip()
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
        (
            self.proxy_scheme,
            self.proxy_netloc,
            self.path,
            self.query,
            self.fragment,
        ) = split_uri(uri)
        self.url_scheme = self.adj.url_scheme
        connection = headers.get("CONNECTION", "")

        if version == "1.0":
            if connection.lower() != "keep-alive":
                self._global_flags[3] = True

        if version == "1.1":
            te = headers.pop("TRANSFER_ENCODING", "")
            if te.lower() == "chunked":
                self._global_flags[4] = True
                buf = OverflowableBuffer(self.adj.inbuf_overflow)
                self._global_strs[2] = ChunkedReceiver(buf)
            expect = headers.get("EXPECT", "").lower()
            self.expect_continue = expect == "100-continue"
            if connection.lower() == "close":
                self._global_flags[3] = True

        if not self._global_flags[4]:
            try:
                cl = int(headers.get("CONTENT_LENGTH", 0))
            except ValueError:
                cl = 0
            self._global_ints[0] = cl
            if cl > 0:
                buf = OverflowableBuffer(self.adj.inbuf_overflow)
                self._global_strs[2] = FixedStreamReceiver(cl, buf)

    def get_body_stream(self):
        body_rcv = self._global_strs[2]
        if body_rcv is not None:
            return body_rcv.getfile()
        else:
            return BytesIO()

    def close(self):
        body_rcv = self._global_strs[2]
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