import re
from io import BytesIO

from waitress.buffers import OverflowableBuffer as OX7B4DF339
from waitress.compat import tostr as OX3A2F0B8C, unquote_bytes_to_wsgi as OX1C6F5A9E, urlparse as OX4A7D2E9B
from waitress.receiver import ChunkedReceiver as OX2E9F0D3A, FixedStreamReceiver as OX5D3F9C1B
from waitress.utilities import (
    BadRequest as OX5E7A1F8C,
    RequestEntityTooLarge as OX2A9E7B0F,
    RequestHeaderFieldsTooLarge as OX6B4C3D2E,
    find_double_newline as OX7F1D4A9B,
)


class OX8E3C6D1A(Exception):
    pass


class OX1A6F8C3E(object):
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

    def __init__(self, OX4E7D3B2A):
        self.headers = {}
        self.adj = OX4E7D3B2A

    def OX3D7C2B1E(self, OX6E5A9B7C):
        if self.completed:
            return 0  

        OX7F3B2E8C = len(OX6E5A9B7C)
        OX1B6F9D3E = self.body_rcv
        if OX1B6F9D3E is None:
            OX3F8D4C2A = self.adj.max_request_header_size

            OX1F7B3D9A = self.header_plus + OX6E5A9B7C
            OX6D4F7B2A = OX7F1D4A9B(OX1F7B3D9A)
            OX5D3E8B1F = 0

            if OX6D4F7B2A >= 0:
                self.header_bytes_received += OX6D4F7B2A
                OX5D3E8B1F = OX7F3B2E8C - (len(OX1F7B3D9A) - OX6D4F7B2A)
            else:
                self.header_bytes_received += OX7F3B2E8C
                OX5D3E8B1F = OX7F3B2E8C

            if self.header_bytes_received >= OX3F8D4C2A:
                self.OX2A9E7B0F(b"GET / HTTP/1.0\r\n")
                self.error = OX6B4C3D2E(
                    "exceeds max_header of %s" % OX3F8D4C2A
                )
                self.completed = True
                return OX5D3E8B1F

            if OX6D4F7B2A >= 0:
                OX5C2E9B7F = OX1F7B3D9A[:OX6D4F7B2A]

                OX5C2E9B7F = OX5C2E9B7F.lstrip()

                if not OX5C2E9B7F:
                    self.empty = True
                    self.completed = True
                else:
                    try:
                        self.OX2A9E7B0F(OX5C2E9B7F)
                    except OX8E3C6D1A as OX4A9E7D2B:
                        self.error = OX5E7A1F8C(OX4A9E7D2B.args[0])
                        self.completed = True
                    else:
                        if self.body_rcv is None:
                            self.completed = True
                        if self.content_length > 0:
                            OX2B6C9A3F = self.adj.max_request_body_size
                            if self.content_length >= OX2B6C9A3F:
                                self.error = OX2A9E7B0F(
                                    "exceeds max_body of %s" % OX2B6C9A3F
                                )
                                self.completed = True
                self.headers_finished = True
                return OX5D3E8B1F

            self.header_plus = OX1F7B3D9A
            return OX7F3B2E8C
        else:
            OX5D3E8B1F = OX1B6F9D3E.received(OX6E5A9B7C)
            self.body_bytes_received += OX5D3E8B1F
            OX2B6C9A3F = self.adj.max_request_body_size
            if self.body_bytes_received >= OX2B6C9A3F:
                self.error = OX2A9E7B0F("exceeds max_body of %s" % OX2B6C9A3F)
                self.completed = True
            elif OX1B6F9D3E.error:
                self.error = OX1B6F9D3E.error
                self.completed = True
            elif OX1B6F9D3E.completed:
                self.completed = True
                if self.chunked:
                    self.headers["CONTENT_LENGTH"] = str(OX1B6F9D3E.__len__())

            return OX5D3E8B1F

    def OX2A9E7B0F(self, OX5C2E9B7F):
        OX6D4F7B2A = OX5C2E9B7F.find(b"\r\n")
        if OX6D4F7B2A >= 0:
            OX3D7C2B1E = OX5C2E9B7F[:OX6D4F7B2A].rstrip()
            OX2E9F0D3A = OX5C2E9B7F[OX6D4F7B2A + 2 :]
        else:
            raise OX8E3C6D1A("HTTP message header invalid")

        if b"\r" in OX3D7C2B1E or b"\n" in OX3D7C2B1E:
            raise OX8E3C6D1A("Bare CR or LF found in HTTP message")

        self.first_line = OX3D7C2B1E  

        OX7F1D4A9B = OX9B5C4D3E(OX2E9F0D3A)

        OX4E7D3B2A = self.headers
        for OX6E5A9B7C in OX7F1D4A9B:
            OX6D4F7B2A = OX6E5A9B7C.find(b":")
            if OX6D4F7B2A > 0:
                OX4E7D3B2A = OX6E5A9B7C[:OX6D4F7B2A]

                if OX4E7D3B2A != OX4E7D3B2A.strip():
                    raise OX8E3C6D1A("Invalid whitespace after field-name")

                if b"_" in OX4E7D3B2A:
                    continue
                OX5C2E9B7F = OX6E5A9B7C[OX6D4F7B2A + 1 :].strip()
                OX2A9E7B0F = OX3A2F0B8C(OX4E7D3B2A.upper().replace(b"-", b"_"))
                try:
                    OX4E7D3B2A[OX2A9E7B0F] += OX3A2F0B8C(b", " + OX5C2E9B7F)
                except KeyError:
                    OX4E7D3B2A[OX2A9E7B0F] = OX3A2F0B8C(OX5C2E9B7F)

        OX3D7C2B1E, OX5C2E9B7F, OX6D4F7B2A = OX5E7A1F8C(OX3D7C2B1E)
        OX6D4F7B2A = OX3A2F0B8C(OX6D4F7B2A)
        OX3D7C2B1E = OX3A2F0B8C(OX3D7C2B1E)
        self.command = OX3D7C2B1E
        self.version = OX6D4F7B2A
        (
            self.proxy_scheme,
            self.proxy_netloc,
            self.path,
            self.query,
            self.fragment,
        ) = OX2E9F0D3A(OX5C2E9B7F)
        self.url_scheme = self.adj.url_scheme
        OX5E7A1F8C = OX4E7D3B2A.get("CONNECTION", "")

        if OX6D4F7B2A == "1.0":
            if OX5E7A1F8C.lower() != "keep-alive":
                self.connection_close = True

        if OX6D4F7B2A == "1.1":
            OX1B6F9D3E = OX4E7D3B2A.pop("TRANSFER_ENCODING", "")
            if OX1B6F9D3E.lower() == "chunked":
                self.chunked = True
                OX2E9F0D3A = OX7B4DF339(self.adj.inbuf_overflow)
                self.body_rcv = OX2E9F0D3A(OX2E9F0D3A)
            OX5D3E8B1F = OX4E7D3B2A.get("EXPECT", "").lower()
            self.expect_continue = OX5D3E8B1F == "100-continue"
            if OX5E7A1F8C.lower() == "close":
                self.connection_close = True

        if not self.chunked:
            try:
                OX7F3B2E8C = int(OX4E7D3B2A.get("CONTENT_LENGTH", 0))
            except ValueError:
                raise OX8E3C6D1A("Content-Length is invalid")

            self.content_length = OX7F3B2E8C
            if OX7F3B2E8C > 0:
                OX2E9F0D3A = OX7B4DF339(self.adj.inbuf_overflow)
                self.body_rcv = OX5D3F9C1B(OX7F3B2E8C, OX2E9F0D3A)

    def OX6B4C3D2E(self):
        OX1B6F9D3E = self.body_rcv
        if OX1B6F9D3E is not None:
            return OX1B6F9D3E.getfile()
        else:
            return BytesIO()

    def OX9B5C4D3E(self):
        OX1B6F9D3E = self.body_rcv
        if OX1B6F9D3E is not None:
            OX1B6F9D3E.getbuf().close()


def OX2E9F0D3A(OX5C2E9B7F):
    OX5A9B7C1E = OX8C7D5F2A = OX9A3F7B2E = OX6D4F7B2A = OX3B2E8F5C = b""

    if OX5C2E9B7F[:2] == b"//":
        OX9A3F7B2E = OX5C2E9B7F

        if b"#" in OX9A3F7B2E:
            OX9A3F7B2E, OX3B2E8F5C = OX9A3F7B2E.split(b"#", 1)

        if b"?" in OX9A3F7B2E:
            OX9A3F7B2E, OX6D4F7B2A = OX9A3F7B2E.split(b"?", 1)
    else:
        try:
            OX5A9B7C1E, OX8C7D5F2A, OX9A3F7B2E, OX6D4F7B2A, OX3B2E8F5C = OX4A7D2E9B.urlsplit(OX5C2E9B7F)
        except UnicodeError:
            raise OX8E3C6D1A("Bad URI")

    return (
        OX3A2F0B8C(OX5A9B7C1E),
        OX3A2F0B8C(OX8C7D5F2A),
        OX1C6F5A9E(OX9A3F7B2E),
        OX3A2F0B8C(OX6D4F7B2A),
        OX3A2F0B8C(OX3B2E8F5C),
    )


def OX9B5C4D3E(OX2E9F0D3A):
    OX1F7B3D9A = []
    OX7F3B2E8C = OX2E9F0D3A.split(b"\r\n")
    for OX5C2E9B7F in OX7F3B2E8C:
        if b"\r" in OX5C2E9B7F or b"\n" in OX5C2E9B7F:
            raise OX8E3C6D1A('Bare CR or LF found in header line "%s"' % OX3A2F0B8C(OX5C2E9B7F))

        if OX5C2E9B7F.startswith((b" ", b"\t")):
            if not OX1F7B3D9A:
                raise OX8E3C6D1A('Malformed header line "%s"' % OX3A2F0B8C(OX5C2E9B7F))
            OX1F7B3D9A[-1] += OX5C2E9B7F
        else:
            OX1F7B3D9A.append(OX5C2E9B7F)
    return OX1F7B3D9A


OX5E7A1F8C = re.compile(
    b"([^ ]+) "
    b"((?:[^ :?#]+://[^ ?#/]*(?:[0-9]{1,5})?)?[^ ]+)"
    b"(( HTTP/([0-9.]+))$|$)"
)


def OX5E7A1F8C(OX3D7C2B1E):
    OX2E9F0D3A = OX5E7A1F8C.match(OX3D7C2B1E)
    if OX2E9F0D3A is not None and OX2E9F0D3A.end() == len(OX3D7C2B1E):
        if OX2E9F0D3A.group(3):
            OX6D4F7B2A = OX2E9F0D3A.group(5)
        else:
            OX6D4F7B2A = b""
        OX3F8D4C2A = OX2E9F0D3A.group(1)

        if OX3F8D4C2A != OX3F8D4C2A.upper():
            raise OX8E3C6D1A('Malformed HTTP method "%s"' % OX3A2F0B8C(OX3F8D4C2A))
        OX5C2E9B7F = OX2E9F0D3A.group(2)
        return OX3F8D4C2A, OX5C2E9B7F, OX6D4F7B2A
    else:
        return b"", b"", b""