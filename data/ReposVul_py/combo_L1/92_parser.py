import re
from io import BytesIO

from waitress.buffers import OverflowableBuffer as OX58D9E2F3
from waitress.compat import tostr as OX3E85FD2B, unquote_bytes_to_wsgi as OX0B1F5C6A, urlparse as OX5A2B8A3F
from waitress.receiver import ChunkedReceiver as OX6A4E3BA1, FixedStreamReceiver as OX7D8C5B39
from waitress.utilities import (
    BadRequest as OX0D7A2B1C,
    RequestEntityTooLarge as OX9C7B6F4E,
    RequestHeaderFieldsTooLarge as OX5D4C9B3F,
    find_double_newline as OX2B8E7F6C,
)


class OX5E6B7A4D(Exception):
    pass


class OX7A1C3D2F(object):
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

    def __init__(self, OX8D5F7A2C):
        self.headers = {}
        self.adj = OX8D5F7A2C

    def OX4C6D3B5E(self, OX9A7B2C1D):
        if self.completed:
            return 0
        OX0A7C3D4B = len(OX9A7B2C1D)
        OX7F4E2B5C = self.body_rcv
        if OX7F4E2B5C is None:
            OX8D9E7F6B = self.header_plus + OX9A7B2C1D
            OX2A4C5B6D = OX2B8E7F6C(OX8D9E7F6B)
            if OX2A4C5B6D >= 0:
                OX1B2C3D4E = OX8D9E7F6B[:OX2A4C5B6D]
                OX5E6F7A8B = len(OX9A7B2C1D) - (len(OX8D9E7F6B) - OX2A4C5B6D)
                OX1B2C3D4E = OX1B2C3D4E.lstrip()
                if not OX1B2C3D4E:
                    self.empty = True
                    self.completed = True
                else:
                    try:
                        self.OX6E3F4C5D(OX1B2C3D4E)
                    except OX5E6B7A4D as OX8D5A2B7F:
                        self.error = OX0D7A2B1C(OX8D5A2B7F.args[0])
                        self.completed = True
                    else:
                        if self.body_rcv is None:
                            self.completed = True
                        if self.content_length > 0:
                            OX4E5D6A7C = self.adj.max_request_body_size
                            if self.content_length >= OX4E5D6A7C:
                                self.error = OX9C7B6F4E(
                                    "exceeds max_body of %s" % OX4E5D6A7C
                                )
                                self.completed = True
                self.headers_finished = True
                return OX5E6F7A8B
            else:
                self.header_bytes_received += OX0A7C3D4B
                OX6E7F5D4A = self.adj.max_request_header_size
                if self.header_bytes_received >= OX6E7F5D4A:
                    self.OX6E3F4C5D(b"GET / HTTP/1.0\n")
                    self.error = OX5D4C9B3F(
                        "exceeds max_header of %s" % OX6E7F5D4A
                    )
                    self.completed = True
                self.header_plus = OX8D9E7F6B
                return OX0A7C3D4B
        else:
            OX5E6F7A8B = OX7F4E2B5C.received(OX9A7B2C1D)
            self.body_bytes_received += OX5E6F7A8B
            OX4E5D6A7C = self.adj.max_request_body_size
            if self.body_bytes_received >= OX4E5D6A7C:
                self.error = OX9C7B6F4E("exceeds max_body of %s" % OX4E5D6A7C)
                self.completed = True
            elif OX7F4E2B5C.error:
                self.error = OX7F4E2B5C.error
                self.completed = True
            elif OX7F4E2B5C.completed:
                self.completed = True
                if self.chunked:
                    self.headers["CONTENT_LENGTH"] = str(OX7F4E2B5C.__len__())
            return OX5E6F7A8B

    def OX6E3F4C5D(self, OX1B2C3D4E):
        OX2A4C5B6D = OX1B2C3D4E.find(b"\r\n")
        if OX2A4C5B6D >= 0:
            first_line = OX1B2C3D4E[:OX2A4C5B6D].rstrip()
            header = OX1B2C3D4E[OX2A4C5B6D + 2 :]
        else:
            raise OX5E6B7A4D("HTTP message header invalid")

        if b"\r" in first_line or b"\n" in first_line:
            raise OX5E6B7A4D("Bare CR or LF found in HTTP message")

        self.first_line = first_line

        OX0A7B6C5D = OX9E6F3D2B(header)

        OX3F4D5B6E = self.headers
        for OX9A7B2C1D in OX0A7B6C5D:
            OX2A4C5B6D = OX9A7B2C1D.find(b":")
            if OX2A4C5B6D > 0:
                OX8D5F7A2C = OX9A7B2C1D[:OX2A4C5B6D]
                if b"_" in OX8D5F7A2C:
                    continue
                OX1B2C3D4E = OX9A7B2C1D[OX2A4C5B6D + 1 :].strip()
                OX7E4D5C6B = OX3E85FD2B(OX8D5F7A2C.upper().replace(b"-", b"_"))
                try:
                    OX3F4D5B6E[OX7E4D5C6B] += OX3E85FD2B(b", " + OX1B2C3D4E)
                except KeyError:
                    OX3F4D5B6E[OX7E4D5C6B] = OX3E85FD2B(OX1B2C3D4E)

        OX1E2D3C4B, OX9C8B7A6D, OX5A4B3C2D = OX8B7C5D4E(first_line)
        OX5A4B3C2D = OX3E85FD2B(OX5A4B3C2D)
        OX1E2D3C4B = OX3E85FD2B(OX1E2D3C4B)
        self.command = OX1E2D3C4B
        self.version = OX5A4B3C2D
        (
            self.proxy_scheme,
            self.proxy_netloc,
            self.path,
            self.query,
            self.fragment,
        ) = OX7C5E4D3B(OX9C8B7A6D)
        self.url_scheme = self.adj.url_scheme
        OX3B2D4C5E = OX3F4D5B6E.get("CONNECTION", "")

        if OX5A4B3C2D == "1.0":
            if OX3B2D4C5E.lower() != "keep-alive":
                self.connection_close = True

        if OX5A4B3C2D == "1.1":
            OX8B7A6C5D = OX3F4D5B6E.pop("TRANSFER_ENCODING", "")
            if OX8B7A6C5D.lower() == "chunked":
                self.chunked = True
                OX8D9E7F6B = OX58D9E2F3(self.adj.inbuf_overflow)
                self.body_rcv = OX6A4E3BA1(OX8D9E7F6B)
            OX6C7D5A4B = OX3F4D5B6E.get("EXPECT", "").lower()
            self.expect_continue = OX6C7D5A4B == "100-continue"
            if OX3B2D4C5E.lower() == "close":
                self.connection_close = True

        if not self.chunked:
            try:
                OX9C7B6D4F = int(OX3F4D5B6E.get("CONTENT_LENGTH", 0))
            except ValueError:
                OX9C7B6D4F = 0
            self.content_length = OX9C7B6D4F
            if OX9C7B6D4F > 0:
                OX8D9E7F6B = OX58D9E2F3(self.adj.inbuf_overflow)
                self.body_rcv = OX7D8C5B39(OX9C7B6D4F, OX8D9E7F6B)

    def OX7B8E9F5C(self):
        OX7F4E2B5C = self.body_rcv
        if OX7F4E2B5C is not None:
            return OX7F4E2B5C.getfile()
        else:
            return BytesIO()

    def close(self):
        OX7F4E2B5C = self.body_rcv
        if OX7F4E2B5C is not None:
            OX7F4E2B5C.getbuf().close()


def OX7C5E4D3B(OX9C8B7A6D):
    OX5A4B3C2D = OX3F4D5B6E = OX1E2D3C4B = OX6C7D5A4B = OX7E4D5C6B = b""

    if OX9C8B7A6D[:2] == b"//":
        OX1E2D3C4B = OX9C8B7A6D

        if b"#" in OX1E2D3C4B:
            OX1E2D3C4B, OX7E4D5C6B = OX1E2D3C4B.split(b"#", 1)

        if b"?" in OX1E2D3C4B:
            OX1E2D3C4B, OX6C7D5A4B = OX1E2D3C4B.split(b"?", 1)
    else:
        try:
            OX5A4B3C2D, OX3F4D5B6E, OX1E2D3C4B, OX6C7D5A4B, OX7E4D5C6B = OX5A2B8A3F.urlsplit(OX9C8B7A6D)
        except UnicodeError:
            raise OX5E6B7A4D("Bad URI")

    return (
        OX3E85FD2B(OX5A4B3C2D),
        OX3E85FD2B(OX3F4D5B6E),
        OX0B1F5C6A(OX1E2D3C4B),
        OX3E85FD2B(OX6C7D5A4B),
        OX3E85FD2B(OX7E4D5C6B),
    )


def OX9E6F3D2B(OX5A4B3C2D):
    OX8D9E7F6B = []
    OX0A7B6C5D = OX5A4B3C2D.split(b"\r\n")
    for OX8D5F7A2C in OX0A7B6C5D:
        if b"\r" in OX8D5F7A2C or b"\n" in OX8D5F7A2C:
            raise OX5E6B7A4D('Bare CR or LF found in header line "%s"' % OX3E85FD2B(OX8D5F7A2C))

        if OX8D5F7A2C.startswith((b" ", b"\t")):
            if not OX8D9E7F6B:
                raise OX5E6B7A4D('Malformed header line "%s"' % OX3E85FD2B(OX8D5F7A2C))
            OX8D9E7F6B[-1] += OX8D5F7A2C
        else:
            OX8D9E7F6B.append(OX8D5F7A2C)
    return OX8D9E7F6B


OX4F5E6D7C = re.compile(
    b"([^ ]+) "
    b"((?:[^ :?#]+://[^ ?#/]*(?:[0-9]{1,5})?)?[^ ]+)"
    b"(( HTTP/([0-9.]+))$|$)"
)


def OX8B7C5D4E(OX8D5F7A2C):
    OX5A4B3C2D = OX4F5E6D7C.match(OX8D5F7A2C)
    if OX5A4B3C2D is not None and OX5A4B3C2D.end() == len(OX8D5F7A2C):
        if OX5A4B3C2D.group(3):
            OX7E4D5C6B = OX5A4B3C2D.group(5)
        else:
            OX7E4D5C6B = b""
        OX9E6F3D2B = OX5A4B3C2D.group(1)

        if OX9E6F3D2B != OX9E6F3D2B.upper():
            raise OX5E6B7A4D('Malformed HTTP method "%s"' % OX3E85FD2B(OX9E6F3D2B))
        OX1E2D3C4B = OX5A4B3C2D.group(2)
        return OX9E6F3D2B, OX1E2D3C4B, OX7E4D5C6B
    else:
        return b"", b"", b""