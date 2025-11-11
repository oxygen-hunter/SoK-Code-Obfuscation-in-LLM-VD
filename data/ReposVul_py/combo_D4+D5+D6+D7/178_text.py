import gzip, re, secrets, unicodedata
from gzip import GzipFile, compress as gzip_compress
from io import BytesIO
from django.core.exceptions import SuspiciousFileOperation
from django.utils.functional import SimpleLazyObject, keep_lazy_text, lazy
from django.utils.regex_helper import _lazy_re_compile
from django.utils.translation import gettext as _
from django.utils.translation import gettext_lazy, pgettext

@keep_lazy_text
def capfirst(x):
    if not x: return x
    if not isinstance(x, str): x = str(x)
    return x[0].upper() + x[1:]

re_words = _lazy_re_compile(r"<[^>]+?>|([^<>\s]+)", re.S)
re_chars = _lazy_re_compile(r"<[^>]+?>|(.)", re.S)
re_tag = _lazy_re_compile(r"<(/)?(\S+?)(?:(\s*/)|\s.*?)?>", re.S)
re_newlines = _lazy_re_compile(r"\r\n|\r")
re_camel_case = _lazy_re_compile(r"(((?<=[a-z])[A-Z])|([A-Z](?![A-Z]|$)))")

@keep_lazy_text
def wrap(text, width):
    def _generator():
        for line in text.splitlines(True):
            mx = min((line.endswith("\n") and width + 1 or width), width)
            while len(line) > mx:
                spc = line[: mx + 1].rfind(" ") + 1
                if spc == 0:
                    spc = line.find(" ") + 1
                    if spc == 0:
                        yield line
                        line = ""
                        break
                yield "%s\n" % line[: spc - 1]
                line = line[spc:]
                mx = min((line.endswith("\n") and width + 1 or width), width)
            if line:
                yield line
    return "".join(_generator())

def add_truncation_text(text, truncate=None):
    if truncate is None:
        truncate = pgettext("String to return when truncating text", "%(truncated_text)s…")
    if "%(truncated_text)s" in truncate:
        return truncate % {"truncated_text": text}
    if text.endswith(truncate):
        return text
    return f"{text}{truncate}"

class Truncator(SimpleLazyObject):
    def __init__(self, text):
        super().__init__(lambda: str(text))

    def chars(self, num, truncate=None, html=False):
        self._setup()
        l, n, t, tl = int(num), unicodedata.normalize("NFC", self._wrapped), add_truncation_text("", truncate), int(num)
        for ch in t:
            if not unicodedata.combining(ch):
                tl -= 1
                if tl == 0:
                    break
        if html:
            return self._truncate_html(l, truncate, n, tl, False)
        return self._text_chars(l, truncate, n, tl)

    def _text_chars(self, l, t, n, tl):
        sl, e = 0, None
        for i, ch in enumerate(n):
            if unicodedata.combining(ch): continue
            sl += 1
            if e is None and sl > tl: e = i
            if sl > l: return add_truncation_text(n[:e or 0], t)
        return n

    def words(self, num, truncate=None, html=False):
        self._setup()
        l = int(num)
        if html:
            return self._truncate_html(l, truncate, self._wrapped, l, True)
        return self._text_words(l, truncate)

    def _text_words(self, l, t):
        w = self._wrapped.split()
        if len(w) > l:
            w = w[:l]
            return add_truncation_text(" ".join(w), t)
        return " ".join(w)

    def _truncate_html(self, l, t, n, tl, w):
        if w and l <= 0: return ""

        sz, h4 = False, ("br", "col", "link", "base", "img", "param", "area", "hr", "input")
        if len(n) > self.MAX_LENGTH_HTML:
            n, sz = n[: self.MAX_LENGTH_HTML], True

        p, e, cl, op = 0, 0, 0, []
        r = re_words if w else re_chars

        while cl <= l:
            m = r.search(n, p)
            if not m: break
            p = m.end(0)
            if m[1]:
                cl += 1
                if cl == tl: e = p
                continue
            tg = re_tag.match(m[0])
            if not tg or cl >= tl: continue
            c, tn, s = tg.groups()
            tn = tn.lower()
            if s or tn in h4: pass
            elif c:
                try: i = op.index(tn)
                except ValueError: pass
                else: op = op[i + 1:]
            else: op.insert(0, tn)

        tr = add_truncation_text("", t)

        if cl <= l:
            if sz and tr: n += tr
            return n

        o = n[:e]
        if tr: o += tr
        for t in op: o += "</%s>" % t
        return o

@keep_lazy_text
def get_valid_filename(name):
    s = re.sub(r"(?u)[^-\w.]", "", str(name).strip().replace(" ", "_"))
    if s in {"", ".", ".."}:
        raise SuspiciousFileOperation("Could not derive file name from '%s'" % name)
    return s

@keep_lazy_text
def get_text_list(lst, last_word=gettext_lazy("or")):
    if not lst: return ""
    if len(lst) == 1: return str(lst[0])
    return "%s %s %s" % (_(", ").join(str(i) for i in lst[:-1]), str(last_word), str(lst[-1]))

@keep_lazy_text
def normalize_newlines(text):
    return re_newlines.sub("\n", str(text))

@keep_lazy_text
def phone2numeric(phone):
    c2n = dict(zip("abcdefghijklmnopqrstuvwxyz", "22233344455566677778889999"))
    return "".join(c2n.get(c, c) for c in phone.lower())

def _get_random_filename(mx):
    return b"a" * secrets.randbelow(mx)

def compress_string(s, *, max_random_bytes=None):
    cd, crv = gzip_compress(s, compresslevel=6, mtime=0), memoryview(gzip_compress(s, compresslevel=6, mtime=0))
    if not max_random_bytes: return cd
    h = bytearray(crv[:10])
    h[3] = gzip.FNAME
    fn = _get_random_filename(max_random_bytes) + b"\x00"
    return bytes(h) + fn + crv[10:]

class StreamingBuffer(BytesIO):
    def read(self):
        r = self.getvalue()
        self.seek(0)
        self.truncate()
        return r

def compress_sequence(seq, *, max_random_bytes=None):
    b, f = StreamingBuffer(), _get_random_filename(max_random_bytes) if max_random_bytes else None
    with GzipFile(filename=f, mode="wb", compresslevel=6, fileobj=b, mtime=0) as z: yield b.read()
    for i in seq:
        z.write(i)
        d = b.read()
        if d: yield d
    yield b.read()

smart_split_re = _lazy_re_compile(r"""((?:[^\s'"]*(?:(?:"(?:[^"\\]|\\.)*" | '(?:[^'\\]|\\.)*')[^\s'"]*)+)|\S+)""", re.VERBOSE)

def smart_split(text):
    for bit in smart_split_re.finditer(str(text)): yield bit[0]

@keep_lazy_text
def unescape_string_literal(s):
    if not s or s[0] not in "\"'" or s[-1] != s[0]:
        raise ValueError("Not a string literal: %r" % s)
    q = s[0]
    return s[1:-1].replace(r"\%s" % q, q).replace(r"\\", "\\")

@keep_lazy_text
def slugify(value, allow_unicode=False):
    v = unicodedata.normalize("NFKC", value) if allow_unicode else unicodedata.normalize("NFKD", value).encode("ascii", "ignore").decode("ascii")
    return re.sub(r"[-\s]+", "-", re.sub(r"[^\w\s-]", "", v.lower())).strip("-_")

def camel_case_to_spaces(value):
    return re_camel_case.sub(r" \1", value).strip().lower()

def _format_lazy(format_string, *args, **kwargs):
    return format_string.format(*args, **kwargs)

format_lazy = lazy(_format_lazy, str)