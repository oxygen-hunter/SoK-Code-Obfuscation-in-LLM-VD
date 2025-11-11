import gzip
import re
import secrets
import unicodedata
from gzip import GzipFile
from gzip import compress as gzip_compress
from io import BytesIO
from django.core.exceptions import SuspiciousFileOperation
from django.utils.functional import SimpleLazyObject, keep_lazy_text, lazy
from django.utils.regex_helper import _lazy_re_compile
from django.utils.translation import gettext as _
from django.utils.translation import gettext_lazy, pgettext

@keep_lazy_text
def capfirst(x):
    if not x:
        return x
    if not isinstance(x, str):
        x = str(x)
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
            max_width = min((line.endswith("\n") and width + 1 or width), width)
            while len(line) > max_width:
                space = line[: max_width + 1].rfind(" ") + 1
                if space == 0:
                    space = line.find(" ") + 1
                    if space == 0:
                        yield line
                        line = ""
                        break
                yield "%s\n" % line[: space - 1]
                line = line[space:]
                max_width = min((line.endswith("\n") and width + 1 or width), width)
            if line:
                yield line
    return "".join(_generator())

def add_truncation_text(text, truncate=None):
    if truncate is None:
        truncate = pgettext(
            "String to return when truncating text", "%(truncated_text)s…"
        )
    if "%(truncated_text)s" in truncate:
        return truncate % {"truncated_text": text}
    if text.endswith(truncate):
        return text
    return f"{text}{truncate}"

class Truncator(SimpleLazyObject):
    MAX_LENGTH_HTML = 5_000_000
    def __init__(self, text):
        super().__init__(lambda: str(text))
    def chars(self, num, truncate=None, html=False):
        self._setup()
        length = int(num)
        text = unicodedata.normalize("NFC", self._wrapped)
        truncate_len = length
        for char in add_truncation_text("", truncate):
            if not unicodedata.combining(char):
                truncate_len -= 1
                if truncate_len == 0:
                    break
        if html:
            return self._truncate_html(length, truncate, text, truncate_len, False)
        return self._text_chars(length, truncate, text, truncate_len)
    def _text_chars(self, length, truncate, text, truncate_len):
        s_len = 0
        end_index = None
        for i, char in enumerate(text):
            if unicodedata.combining(char):
                continue
            s_len += 1
            if end_index is None and s_len > truncate_len:
                end_index = i
            if s_len > length:
                return add_truncation_text(text[: end_index or 0], truncate)
        return text
    def words(self, num, truncate=None, html=False):
        self._setup()
        length = int(num)
        if html:
            return self._truncate_html(length, truncate, self._wrapped, length, True)
        return self._text_words(length, truncate)
    def _text_words(self, length, truncate):
        words = self._wrapped.split()
        if len(words) > length:
            words = words[:length]
            return add_truncation_text(" ".join(words), truncate)
        return " ".join(words)
    def _truncate_html(self, length, truncate, text, truncate_len, words):
        if words and length <= 0:
            return ""
        size_limited = False
        if len(text) > self.MAX_LENGTH_HTML:
            text = text[: self.MAX_LENGTH_HTML]
            size_limited = True
        html4_singlets = (
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
        pos = 0
        end_text_pos = 0
        current_len = 0
        open_tags = []
        regex = re_words if words else re_chars
        while current_len <= length:
            m = regex.search(text, pos)
            if not m:
                break
            pos = m.end(0)
            if m[1]:
                current_len += 1
                if current_len == truncate_len:
                    end_text_pos = pos
                continue
            tag = re_tag.match(m[0])
            if not tag or current_len >= truncate_len:
                continue
            closing_tag, tagname, self_closing = tag.groups()
            tagname = tagname.lower()
            if self_closing or tagname in html4_singlets:
                pass
            elif closing_tag:
                try:
                    i = open_tags.index(tagname)
                except ValueError:
                    pass
                else:
                    open_tags = open_tags[i + 1 :]
            else:
                open_tags.insert(0, tagname)
        truncate_text = add_truncation_text("", truncate)
        if current_len <= length:
            if size_limited and truncate_text:
                text += truncate_text
            return text
        out = text[:end_text_pos]
        if truncate_text:
            out += truncate_text
        for tag in open_tags:
            out += "</%s>" % tag
        return out

@keep_lazy_text
def get_valid_filename(name):
    s = str(name).strip().replace(" ", "_")
    s = re.sub(r"(?u)[^-\w.]", "", s)
    if s in {"", ".", ".."}:
        raise SuspiciousFileOperation("Could not derive file name from '%s'" % name)
    return s

@keep_lazy_text
def get_text_list(list_, last_word=gettext_lazy("or")):
    if not list_:
        return ""
    if len(list_) == 1:
        return str(list_[0])
    return "%s %s %s" % (
        _(", ").join(str(i) for i in list_[:-1]),
        str(last_word),
        str(list_[-1]),
    )

@keep_lazy_text
def normalize_newlines(text):
    return re_newlines.sub("\n", str(text))

@keep_lazy_text
def phone2numeric(phone):
    char2number = {
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
    return "".join(char2number.get(c, c) for c in phone.lower())

def _get_random_filename(max_random_bytes):
    return b"a" * secrets.randbelow(max_random_bytes)

def compress_string(s, *, max_random_bytes=None):
    compressed_data = gzip_compress(s, compresslevel=6, mtime=0)
    if not max_random_bytes:
        return compressed_data
    compressed_view = memoryview(compressed_data)
    header = bytearray(compressed_view[:10])
    header[3] = gzip.FNAME
    filename = _get_random_filename(max_random_bytes) + b"\x00"
    return bytes(header) + filename + compressed_view[10:]

class StreamingBuffer(BytesIO):
    def read(self):
        ret = self.getvalue()
        self.seek(0)
        self.truncate()
        return ret

def compress_sequence(sequence, *, max_random_bytes=None):
    buf = StreamingBuffer()
    filename = _get_random_filename(max_random_bytes) if max_random_bytes else None
    with GzipFile(
        filename=filename, mode="wb", compresslevel=6, fileobj=buf, mtime=0
    ) as zfile:
        yield buf.read()
        for item in sequence:
            zfile.write(item)
            data = buf.read()
            if data:
                yield data
    yield buf.read()

smart_split_re = _lazy_re_compile(
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

def smart_split(text):
    for bit in smart_split_re.finditer(str(text)):
        yield bit[0]

@keep_lazy_text
def unescape_string_literal(s):
    if not s or s[0] not in "\"'" or s[-1] != s[0]:
        raise ValueError("Not a string literal: %r" % s)
    quote = s[0]
    return s[1:-1].replace(r"\%s" % quote, quote).replace(r"\\", "\\")

@keep_lazy_text
def slugify(value, allow_unicode=False):
    value = str(value)
    if allow_unicode:
        value = unicodedata.normalize("NFKC", value)
    else:
        value = (
            unicodedata.normalize("NFKD", value)
            .encode("ascii", "ignore")
            .decode("ascii")
        )
    value = re.sub(r"[^\w\s-]", "", value.lower())
    return re.sub(r"[-\s]+", "-", value).strip("-_")

def camel_case_to_spaces(value):
    return re_camel_case.sub(r" \1", value).strip().lower()

def _format_lazy(format_string, *args, **kwargs):
    return format_string.format(*args, **kwargs)

format_lazy = lazy(_format_lazy, str)