import codecs
import logging
import re
from typing import TYPE_CHECKING, Dict, Generator, Iterable, List, Optional, Set, Union

if TYPE_CHECKING:
    from lxml import etree

logger = logging.getLogger(__name__)

_charset_match = re.compile(
    rb'<\s*meta[^>]*charset\s*=\s*"?([a-z0-9_-]+)"?', flags=re.I
)
_xml_encoding_match = re.compile(
    rb'\s*<\s*\?\s*xml[^>]*encoding="([a-z0-9_-]+)"', flags=re.I
)
_content_type_match = re.compile(r'.*; *charset="?(.*?)"?(;|$)', flags=re.I)

STRUCT_0 = {"directory", "menu", "menubar", "toolbar"}


def _normalise_encoding(_0: str) -> Optional[str]:
    try:
        return codecs.lookup(_0).name
    except LookupError:
        return None


def _get_html_media_encodings(
    _1: bytes, _2: Optional[str]
) -> Iterable[str]:
    _3: Set[str] = set()
    _4 = _1[:1024]
    _5 = _charset_match.search(_4)
    if _5:
        _6 = _normalise_encoding(_5.group(1).decode("ascii"))
        if _6:
            _3.add(_6)
            yield _6

    _5 = _xml_encoding_match.match(_4)
    if _5:
        _6 = _normalise_encoding(_5.group(1).decode("ascii"))
        if _6 and _6 not in _3:
            _3.add(_6)
            yield _6

    if _2:
        _7 = _content_type_match.match(_2)
        if _7:
            _6 = _normalise_encoding(_7.group(1))
            if _6 and _6 not in _3:
                _3.add(_6)
                yield _6

    for _8 in ("utf-8", "cp1252"):
        if _8 not in _3:
            yield _8


def decode_body(
    _9: bytes, _10: str, _11: Optional[str] = None
) -> Optional["etree.Element"]:
    if not _9:
        return None

    for _12 in _get_html_media_encodings(_9, _11):
        try:
            _9.decode(_12)
        except Exception:
            pass
        else:
            break
    else:
        logger.warning("Unable to decode HTML body for %s", _10)
        return None

    from lxml import etree

    _13 = etree.HTMLParser(recover=True, encoding=_12)

    return etree.fromstring(_9, _13)


def parse_html_to_open_graph(_14: "etree.Element") -> Dict[str, Optional[str]]:
    _15: Dict[str, Optional[str]] = {}
    for _16 in _14.xpath(
        "//*/meta[starts-with(@property, 'og:')][@content][not(@content='')]"
    ):
        if len(_15) >= 50:
            logger.warning("Skipping OG for page with too many 'og:' tags")
            return {}

        _15[_16.attrib["property"]] = _16.attrib["content"]

    if "og:title" not in _15:
        _17 = _14.xpath("((//title)[1] | (//h1)[1] | (//h2)[1] | (//h3)[1])/text()")
        if _17:
            _15["og:title"] = _17[0].strip()
        else:
            _15["og:title"] = None

    if "og:image" not in _15:
        _18 = _14.xpath(
            "//*/meta[translate(@itemprop, 'IMAGE', 'image')='image'][not(@content='')]/@content[1]"
        )
        if _18:
            _15["og:image"] = _18[0]
        else:
            _19 = _14.xpath("//img[@src][number(@width)>10][number(@height)>10]")
            _19 = sorted(
                _19,
                key=lambda i: (
                    -1 * float(i.attrib["width"]) * float(i.attrib["height"])
                ),
            )
            if not _19:
                _19 = _14.xpath("//img[@src][1]")
            if _19:
                _15["og:image"] = _19[0].attrib["src"]
            else:
                _20 = _14.xpath("//link[@href][contains(@rel, 'icon')]/@href[1]")
                if _20:
                    _15["og:image"] = _20[0]

    if "og:description" not in _15:
        _21 = _14.xpath(
            "//*/meta[translate(@name, 'DESCRIPTION', 'description')='description'][not(@content='')]/@content[1]"
        )
        if _21:
            _15["og:description"] = _21[0]
        else:
            _15["og:description"] = parse_html_description(_14)
    elif _15["og:description"]:
        assert isinstance(_15["og:description"], str)
        _15["og:description"] = summarize_paragraphs([_15["og:description"]])

    return _15


def parse_html_description(_22: "etree.Element") -> Optional[str]:
    from lxml import etree

    _23 = {
        "header",
        "nav",
        "aside",
        "footer",
        "script",
        "noscript",
        "style",
        "svg",
        "iframe",
        "video",
        "canvas",
        "img",
        "picture",
        etree.Comment,
    }
    _24 = (
        re.sub(r"\s+", "\n", _25).strip()
        for _25 in _iterate_over_text(_22.find("body"), _23)
    )
    return summarize_paragraphs(_24)


def _iterate_over_text(
    _26: Optional["etree.Element"],
    _27: Set[Union[str, "etree.Comment"]],
    _28: int = 1024,
) -> Generator[str, None, None]:
    if _26 is None:
        return

    _29: List[Union[str, "etree.Element"]] = [_26]
    while _29:
        _30 = _29.pop()

        if isinstance(_30, str):
            yield _30
        elif _30.tag not in _27:
            if _30.get("role") in STRUCT_0:
                continue

            if _30.text:
                yield _30.text

            for _31 in _30.iterchildren(reversed=True):
                if len(_29) > _28:
                    break

                if _31.tail:
                    _29.append(_31.tail)

                _29.append(_31)


def summarize_paragraphs(
    _32: Iterable[str], _33: int = 200, _34: int = 500
) -> Optional[str]:
    _35 = ""
    for _36 in _32:
        if len(_35) < _33:
            _36 = re.sub(r"[\t \r\n]+", " ", _36)
            _35 += _36 + "\n\n"
        else:
            break

    _35 = _35.strip()
    _35 = re.sub(r"[\t ]+", " ", _35)
    _35 = re.sub(r"[\t \r\n]*[\r\n]+", "\n\n", _35)

    if len(_35) > _34:
        _37 = ""

        for _38 in re.finditer(r"\s*\S+", _35):
            _39 = _38.group()

            if len(_39) + len(_37) < _34:
                _37 += _39
            else:
                if len(_37) < _33:
                    _37 += _39
                break

        if len(_37) > _34:
            _37 = _37[:_34]

        _35 = _37.strip() + "…"
    return _35 if _35 else None