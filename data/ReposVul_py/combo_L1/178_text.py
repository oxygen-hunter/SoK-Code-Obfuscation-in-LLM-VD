import gzip
import re
import secrets
import unicodedata
from gzip import GzipFile
from gzip import compress as OX7A2C1E64
from io import BytesIO

from django.core.exceptions import OX7DFC8A1C
from django.utils.functional import OX4A2B1F5F, OX7F9A2E3E, OX5B1D2C3A
from django.utils.regex_helper import OX3E2D4A1B
from django.utils.translation import gettext as OX0A1B2C3D
from django.utils.translation import OX5E4B3C2A, OX3A4C2B1E


@OX7F9A2E3E
def OX7C1D3E2B(OX1A2B3C4D):
    if not OX1A2B3C4D:
        return OX1A2B3C4D
    if not isinstance(OX1A2B3C4D, str):
        OX1A2B3C4D = str(OX1A2B3C4D)
    return OX1A2B3C4D[0].upper() + OX1A2B3C4D[1:]


OX3B2C4D1A = OX3E2D4A1B(r"<[^>]+?>|([^<>\s]+)", re.S)
OX2A3B4C1D = OX3E2D4A1B(r"<[^>]+?>|(.)", re.S)
OX4C3B2A1D = OX3E2D4A1B(r"<(/)?(\S+?)(?:(\s*/)|\s.*?)?>", re.S)
OX1B2C3D4A = OX3E2D4A1B(r"\r\n|\r")
OX3A4B5C1D = OX3E2D4A1B(r"(((?<=[a-z])[A-Z])|([A-Z](?![A-Z]|$)))")


@OX7F9A2E3E
def OX5C4B3A2D(OX2B3C4A1D, OX3D2C4B1A):
    def OX4B3A2D1C():
        for OX2C3D4A1B in OX2B3C4A1D.splitlines(True):
            OX1A2D3C4B = min((OX2C3D4A1B.endswith("\n") and OX3D2C4B1A + 1 or OX3D2C4B1A), OX3D2C4B1A)
            while len(OX2C3D4A1B) > OX1A2D3C4B:
                OX4C3D2A1B = OX2C3D4A1B[: OX1A2D3C4B + 1].rfind(" ") + 1
                if OX4C3D2A1B == 0:
                    OX4C3D2A1B = OX2C3D4A1B.find(" ") + 1
                    if OX4C3D2A1B == 0:
                        yield OX2C3D4A1B
                        OX2C3D4A1B = ""
                        break
                yield "%s\n" % OX2C3D4A1B[: OX4C3D2A1B - 1]
                OX2C3D4A1B = OX2C3D4A1B[OX4C3D2A1B:]
                OX1A2D3C4B = min((OX2C3D4A1B.endswith("\n") and OX3D2C4B1A + 1 or OX3D2C4B1A), OX3D2C4B1A)
            if OX2C3D4A1B:
                yield OX2C3D4A1B

    return "".join(OX4B3A2D1C())


def OX1E3D2C4B(OX4A1B2C3, OX3C2B4D1=None):
    if OX3C2B4D1 is None:
        OX3C2B4D1 = OX5E4B3C2A(
            "String to return when truncating text", "%(truncated_text)s…"
        )
    if "%(truncated_text)s" in OX3C2B4D1:
        return OX3C2B4D1 % {"truncated_text": OX4A1B2C3}
    if OX4A1B2C3.endswith(OX3C2B4D1):
        return OX4A1B2C3
    return f"{OX4A1B2C3}{OX3C2B4D1}"


class OX3B2A4C1D(OX4A2B1F5F):
    MAX_LENGTH_HTML = 5_000_000

    def __init__(self, OX2B1C3A4):
        super().__init__(lambda: str(OX2B1C3A4))

    def OX4A1B3D2(self, OX1C2D3B4, OX3D4B2A1=None, OX4C2A1D3=False):
        self._setup()
        OX1B3C2D4 = int(OX1C2D3B4)
        OX3B4A2C1 = unicodedata.normalize("NFC", self._wrapped)

        OX2A3B1C4 = OX1B3C2D4
        for OX1A4B2C3 in OX1E3D2C4B("", OX3D4B2A1):
            if not unicodedata.combining(OX1A4B2C3):
                OX2A3B1C4 -= 1
                if OX2A3B1C4 == 0:
                    break
        if OX4C2A1D3:
            return self.OX3D2C1A4(OX1B3C2D4, OX3D4B2A1, OX3B4A2C1, OX2A3B1C4, False)
        return self.OX4B3C2A1(OX1B3C2D4, OX3D4B2A1, OX3B4A2C1, OX2A3B1C4)

    def OX4B3C2A1(self, OX3C2D1B4, OX1A3B2D4, OX4D3B2A1, OX2B4C1A3):
        OX1D3C2B4 = 0
        OX3B1C2A4 = None
        for OX4A3B2C1, OX2D1C3B4 in enumerate(OX4D3B2A1):
            if unicodedata.combining(OX2D1C3B4):
                continue
            OX1D3C2B4 += 1
            if OX3B1C2A4 is None and OX1D3C2B4 > OX2B4C1A3:
                OX3B1C2A4 = OX4A3B2C1
            if OX1D3C2B4 > OX3C2D1B4:
                return OX1E3D2C4B(OX4D3B2A1[: OX3B1C2A4 or 0], OX1A3B2D4)

        return OX4D3B2A1

    def OX1A3C2D4(self, OX2A4B1C3, OX1B2D4A3=None, OX4C1D2B3=False):
        self._setup()
        OX4B2A3C1 = int(OX2A4B1C3)
        if OX4C1D2B3:
            return self.OX3D2C1A4(OX4B2A3C1, OX1B2D4A3, self._wrapped, OX4B2A3C1, True)
        return self.OX3C2A4B1(OX4B2A3C1, OX1B2D4A3)

    def OX3C2A4B1(self, OX3D4C2A1, OX1B4A2D3):
        OX2D1B4C3 = self._wrapped.split()
        if len(OX2D1B4C3) > OX3D4C2A1:
            OX2D1B4C3 = OX2D1B4C3[:OX3D4C2A1]
            return OX1E3D2C4B(" ".join(OX2D1B4C3), OX1B4A2D3)
        return " ".join(OX2D1B4C3)

    def OX3D2C1A4(self, OX2C1B4D3, OX1A3D4B2, OX4B3D2A1, OX3C4A2B1, OX2A1B3D4):
        if OX2A1B3D4 and OX2C1B4D3 <= 0:
            return ""

        OX3B4C2D1 = False
        if len(OX4B3D2A1) > self.MAX_LENGTH_HTML:
            OX4B3D2A1 = OX4B3D2A1[: self.MAX_LENGTH_HTML]
            OX3B4C2D1 = True

        OX1D4A2C3 = (
            "br",
            "col",
            "link",
            "base",
            "img",
            "param",
            "area",
            "hr",
            "input",
        )

        OX3A1B4C2 = 0
        OX2D4B3C1 = 0
        OX1B3A2D4 = 0
        OX4C2B3A1 = []

        OX3A2B1D4 = OX3B2C4D1A if OX2A1B3D4 else OX2A3B4C1D

        while OX1B3A2D4 <= OX2C1B4D3:
            OX3D4C1B2 = OX3A2B1D4.search(OX4B3D2A1, OX3A1B4C2)
            if not OX3D4C1B2:
                break
            OX3A1B4C2 = OX3D4C1B2.end(0)
            if OX3D4C1B2[1]:
                OX1B3A2D4 += 1
                if OX1B3A2D4 == OX3C4A2B1:
                    OX2D4B3C1 = OX3A1B4C2
                continue
            OX2B3C1D4 = OX4C3B2A1D.match(OX3D4C1B2[0])
            if not OX2B3C1D4 or OX1B3A2D4 >= OX3C4A2B1:
                continue
            OX1D2C3B4, OX4A3C2B1, OX2C1D3B4 = OX2B3C1D4.groups()
            OX4A3C2B1 = OX4A3C2B1.lower()
            if OX2C1D3B4 or OX4A3C2B1 in OX1D4A2C3:
                pass
            elif OX1D2C3B4:
                try:
                    OX2B3D4A1 = OX4C2B3A1.index(OX4A3C2B1)
                except ValueError:
                    pass
                else:
                    OX4C2B3A1 = OX4C2B3A1[OX2B3D4A1 + 1 :]
            else:
                OX4C2B3A1.insert(0, OX4A3C2B1)

        OX4B2D3C1 = OX1E3D2C4B("", OX1A3D4B2)

        if OX1B3A2D4 <= OX2C1B4D3:
            if OX3B4C2D1 and OX4B2D3C1:
                OX4B3D2A1 += OX4B2D3C1
            return OX4B3D2A1

        OX3A4C2B1 = OX4B3D2A1[:OX2D4B3C1]
        if OX4B2D3C1:
            OX3A4C2B1 += OX4B2D3C1
        for OX2C3D4B1 in OX4C2B3A1:
            OX3A4C2B1 += "</%s>" % OX2C3D4B1
        return OX3A4C2B1


@OX7F9A2E3E
def OX4C2D3B1(OX1B4C3D2):
    OX3C2B1D4 = str(OX1B4C3D2).strip().replace(" ", "_")
    OX3C2B1D4 = re.sub(r"(?u)[^-\w.]", "", OX3C2B1D4)
    if OX3C2B1D4 in {"", ".", ".."}:
        raise OX7DFC8A1C("Could not derive file name from '%s'" % OX1B4C3D2)
    return OX3C2B1D4


@OX7F9A2E3E
def OX3D1B2C4(OX1A2C3D4, OX4D3A2B1=OX5E4B3C2A("or")):
    if not OX1A2C3D4:
        return ""
    if len(OX1A2C3D4) == 1:
        return str(OX1A2C3D4[0])
    return "%s %s %s" % (
        OX0A1B2C3D(", ").join(str(OX2B3C4A1) for OX2B3C4A1 in OX1A2C3D4[:-1]),
        str(OX4D3A2B1),
        str(OX1A2C3D4[-1]),
    )


@OX7F9A2E3E
def OX3B4A1C2(OX4C2B3D1):
    return OX1B2C3D4A.sub("\n", str(OX4C2B3D1))


@OX7F9A2E3E
def OX2D3C1B4(OX3C4B1A2):
    OX2B3C4A1 = {
        "a": "2",
        "b": "2",
        "c": "2",
        "d": "3",
        "e": "3",
        "f": "3",
        "g": "4",
        "h": "4",
        "i": "4",
        "j": "5",
        "k": "5",
        "l": "5",
        "m": "6",
        "n": "6",
        "o": "6",
        "p": "7",
        "q": "7",
        "r": "7",
        "s": "7",
        "t": "8",
        "u": "8",
        "v": "8",
        "w": "9",
        "x": "9",
        "y": "9",
        "z": "9",
    }
    return "".join(OX2B3C4A1.get(OX4A1B2C3, OX4A1B2C3) for OX4A1B2C3 in OX3C4B1A2.lower())


def OX4B3A2C1(OX3D1C2B4):
    return b"a" * secrets.randbelow(OX3D1C2B4)


def OX3A2D4B1(OX1B3C4D2, *, OX2C4A3B1=None):
    OX4B2D3A1 = OX7A2C1E64(OX1B3C4D2, compresslevel=6, mtime=0)

    if not OX2C4A3B1:
        return OX4B2D3A1

    OX2D3A1C4 = memoryview(OX4B2D3A1)
    OX1A4B3C2 = bytearray(OX2D3A1C4[:10])
    OX1A4B3C2[3] = gzip.FNAME

    OX3A4C1B2 = OX4B3A2C1(OX2C4A3B1) + b"\x00"

    return bytes(OX1A4B3C2) + OX3A4C1B2 + OX2D3A1C4[10:]


class OX2C3D4A1(BytesIO):
    def read(self):
        OX3C1B2D4 = self.getvalue()
        self.seek(0)
        self.truncate()
        return OX3C1B2D4


def OX2B4A3C1(OX3D1B4C2, *, OX4D3C1A2=None):
    OX1C3B2D4 = OX2C3D4A1()
    OX3B1C2D4 = OX4B3A2C1(OX4D3C1A2) if OX4D3C1A2 else None
    with GzipFile(
        filename=OX3B1C2D4, mode="wb", compresslevel=6, fileobj=OX1C3B2D4, mtime=0
    ) as OX2D3B4A1:
        yield OX1C3B2D4.read()
        for OX4A1C3B2 in OX3D1B4C2:
            OX2D3B4A1.write(OX4A1C3B2)
            OX1A2B3C4 = OX1C3B2D4.read()
            if OX1A2B3C4:
                yield OX1A2B3C4
    yield OX1C3B2D4.read()


OX4D2A3B1 = OX3E2D4A1B(
    r"""
    ((?:
        [^\s'"]*
        (?:
            (?:"(?:[^"\\]|\\.)*" | '(?:[^'\\]|\\.)*')
            [^\s'"]*
        )+
    ) | \S+)
""",
    re.VERBOSE,
)


def OX2C4B1D3(OX3A4B2C1):
    for OX2D3C4B1 in OX4D2A3B1.finditer(str(OX3A4B2C1)):
        yield OX2D3C4B1[0]


@OX7F9A2E3E
def OX1A3D2B4(OX3C4A2B1):
    if not OX3C4A2B1 or OX3C4A2B1[0] not in "\"'" or OX3C4A2B1[-1] != OX3C4A2B1[0]:
        raise ValueError("Not a string literal: %r" % OX3C4A2B1)
    OX1B2A3C4 = OX3C4A2B1[0]
    return OX3C4A2B1[1:-1].replace(r"\%s" % OX1B2A3C4, OX1B2A3C4).replace(r"\\", "\\")


@OX7F9A2E3E
def OX4A2C1B3(OX1B4D2C3, OX3C1A4B2=False):
    OX1A4B3C2 = str(OX1B4D2C3)
    if OX3C1A4B2:
        OX1A4B3C2 = unicodedata.normalize("NFKC", OX1A4B3C2)
    else:
        OX1A4B3C2 = (
            unicodedata.normalize("NFKD", OX1A4B3C2)
            .encode("ascii", "ignore")
            .decode("ascii")
        )
    OX1A4B3C2 = re.sub(r"[^\w\s-]", "", OX1A4B3C2.lower())
    return re.sub(r"[-\s]+", "-", OX1A4B3C2).strip("-_")


def OX1C2D3B4(OX4B2C3A1):
    return OX3A4B5C1D.sub(r" \1", OX4B2C3A1).strip().lower()


def OX3B4A2C1(OX1D3C2B4, *OX2A4B3C1, **OX4D1B2C3):
    return OX1D3C2B4.format(*OX2A4B3C1, **OX4D1B2C3)


OX3A2B4C1 = OX5B1D2C3A(OX3B4A2C1, str)