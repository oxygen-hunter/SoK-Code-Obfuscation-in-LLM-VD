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
    """Capitalize the first letter of a string."""
    x = str(x) if not isinstance(x, str) else x
    return x if not x else x[0].upper() + x[1:]


re_words = _lazy_re_compile(r"<[^>]+?>|([^<>\s]+)", re.S)
re_chars = _lazy_re_compile(r"<[^>]+?>|(.)", re.S)
re_tag = _lazy_re_compile(r"<(/)?(\S+?)(?:(\s*/)|\s.*?)?>", re.S)
re_newlines = _lazy_re_compile(r"\r\n|\r")
re_camel_case = _lazy_re_compile(r"(((?<=[a-z])[A-Z])|([A-Z](?![A-Z]|$)))")


@keep_lazy_text
def wrap(text, width):
    def _generator(lines):
        if not lines:
            return
        line = lines[0]
        max_width = min((line.endswith("\n") and width + 1 or width), width)
        if len(line) > max_width:
            space = line[: max_width + 1].rfind(" ") + 1
            if space == 0:
                space = line.find(" ") + 1
                if space == 0:
                    yield line
                    line = ""
            yield "%s\n" % line[: space - 1]
            yield from _generator([line[space:]] + lines[1:])
        else:
            yield line
            yield from _generator(lines[1:])

    return "".join(_generator(text.splitlines(True)))


def add_truncation_text(text, truncate=None):
    truncate = pgettext("String to return when truncating text", "%(truncated_text)s…") if truncate is None else truncate
    if "%(truncated_text)s" in truncate:
        return truncate % {"truncated_text": text}
    return text if text.endswith(truncate) else f"{text}{truncate}"


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
        def _recursive_chars(i, s_len, end_index):
            if i >= len(text):
                return add_truncation_text(text[: end_index or 0], truncate) if s_len > length else text
            char = text[i]
            if unicodedata.combining(char):
                return _recursive_chars(i + 1, s_len, end_index)
            s_len += 1
            end_index = end_index if not (end_index is None and s_len > truncate_len) else i
            return _recursive_chars(i + 1, s_len, end_index)

        return _recursive_chars(0, 0, None)

    def words(self, num, truncate=None, html=False):
        self._setup()
        length = int(num)
        if html:
            return self._truncate_html(length, truncate, self._wrapped, length, True)
        return self._text_words(length, truncate)

    def _text_words(self, length, truncate):
        words = self._wrapped.split()
        return add_truncation_text(" ".join(words[:length]), truncate) if len(words) > length else " ".join(words)

    def _truncate_html(self, length, truncate, text, truncate_len, words):
        if words and length <= 0:
            return ""

        size_limited = False
        if len(text) > self.MAX_LENGTH_HTML:
            text = text[: self.MAX_LENGTH_HTML]
            size_limited = True

        html4_singlets = ("br", "col", "link", "base", "img", "param", "area", "hr", "input")

        def _recursive_html(pos, end_text_pos, current_len, open_tags):
            if current_len > length:
                return add_truncation_text(text[:end_text_pos], truncate) + "".join(f"</{tag}>" for tag in open_tags)
            m = (re_words if words else re_chars).search(text, pos)
            if not m:
                return text + (add_truncation_text("", truncate) if size_limited else "")
            pos = m.end(0)
            if m[1]:
                current_len += 1
                end_text_pos = pos if current_len == truncate_len else end_text_pos
                return _recursive_html(pos, end_text_pos, current_len, open_tags)
            tag = re_tag.match(m[0])
            if not tag or current_len >= truncate_len:
                return _recursive_html(pos, end_text_pos, current_len, open_tags)
            closing_tag, tagname, self_closing = tag.groups()
            tagname = tagname.lower()
            if not self_closing and tagname not in html4_singlets:
                if closing_tag:
                    try:
                        i = open_tags.index(tagname)
                        open_tags = open_tags[i + 1 :]
                    except ValueError:
                        pass
                else:
                    open_tags.insert(0, tagname)
            return _recursive_html(pos, end_text_pos, current_len, open_tags)

        return _recursive_html(0, 0, 0, [])

@keep_lazy_text
def get_valid_filename(name):
    s = re.sub(r"(?u)[^-\w.]", "", str(name).strip().replace(" ", "_"))
    if s in {"", ".", ".."}:
        raise SuspiciousFileOperation("Could not derive file name from '%s'" % name)
    return s


@keep_lazy_text
def get_text_list(list_, last_word=gettext_lazy("or")):
    return "" if not list_ else str(list_[0]) if len(list_) == 1 else "%s %s %s" % (_(", ").join(str(i) for i in list_[:-1]), str(last_word), str(list_[-1]))


@keep_lazy_text
def normalize_newlines(text):
    return re_newlines.sub("\n", str(text))


@keep_lazy_text
def phone2numeric(phone):
    return "".join({"a": "2", "b": "2", "c": "2", "d": "3", "e": "3", "f": "3", "g": "4", "h": "4", "i": "4", "j": "5", "k": "5", "l": "5", "m": "6", "n": "6", "o": "6", "p": "7", "q": "7", "r": "7", "s": "7", "t": "8", "u": "8", "v": "8", "w": "9", "x": "9", "y": "9", "z": "9"}.get(c, c) for c in phone.lower())


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
    value = unicodedata.normalize("NFKC", value) if allow_unicode else unicodedata.normalize("NFKD", value).encode("ascii", "ignore").decode("ascii")
    value = re.sub(r"[^\w\s-]", "", value.lower())
    return re.sub(r"[-\s]+", "-", value).strip("-_")


def camel_case_to_spaces(value):
    return re_camel_case.sub(r" \1", value).strip().lower()


def _format_lazy(format_string, *args, **kwargs):
    return format_string.format(*args, **kwargs)


format_lazy = lazy(_format_lazy, str)