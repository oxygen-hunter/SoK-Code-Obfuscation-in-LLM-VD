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

ARIA_ROLES_TO_IGNORE = {"directory", "menu", "menubar", "toolbar"}

def _normalise_encoding(encoding: str) -> Optional[str]:
    try:
        return codecs.lookup(encoding).name
    except LookupError:
        return None

def _get_html_media_encodings(
    body: bytes, content_type: Optional[str]
) -> Iterable[str]:
    attempted_encodings: Set[str] = set()
    body_start = body[:1024]
    match = _charset_match.search(body_start)
    if match:
        encoding = _normalise_encoding(match.group(1).decode("ascii"))
        if encoding:
            attempted_encodings.add(encoding)
            yield encoding
    match = _xml_encoding_match.match(body_start)
    if match:
        encoding = _normalise_encoding(match.group(1).decode("ascii"))
        if encoding and encoding not in attempted_encodings:
            attempted_encodings.add(encoding)
            yield encoding
    if content_type:
        content_match = _content_type_match.match(content_type)
        if content_match:
            encoding = _normalise_encoding(content_match.group(1))
            if encoding and encoding not in attempted_encodings:
                attempted_encodings.add(encoding)
                yield encoding
    for fallback in ("utf-8", "cp1252"):
        if fallback not in attempted_encodings:
            yield fallback

def decode_body(
    body: bytes, uri: str, content_type: Optional[str] = None
) -> Optional["etree.Element"]:
    if not body:
        return None
    for encoding in _get_html_media_encodings(body, content_type):
        try:
            body.decode(encoding)
        except Exception:
            pass
        else:
            break
    else:
        logger.warning("Unable to decode HTML body for %s", uri)
        return None
    from lxml import etree
    parser = etree.HTMLParser(recover=True, encoding=encoding)
    return etree.fromstring(body, parser)

def parse_html_to_open_graph(tree: "etree.Element") -> Dict[str, Optional[str]]:
    og: Dict[str, Optional[str]] = {}
    for tag in tree.xpath(
        "//*/meta[starts-with(@property, 'og:')][@content][not(@content='')]"
    ):
        if len(og) >= 50:
            logger.warning("Skipping OG for page with too many 'og:' tags")
            return {}
        og[tag.attrib["property"]] = tag.attrib["content"]
    if "og:title" not in og:
        title = tree.xpath("((//title)[1] | (//h1)[1] | (//h2)[1] | (//h3)[1])/text()")
        if title:
            og["og:title"] = title[0].strip()
        else:
            og["og:title"] = None
    if "og:image" not in og:
        meta_image = tree.xpath(
            "//*/meta[translate(@itemprop, 'IMAGE', 'image')='image'][not(@content='')]/@content[1]"
        )
        if meta_image:
            og["og:image"] = meta_image[0]
        else:
            images = tree.xpath("//img[@src][number(@width)>10][number(@height)>10]")
            images = sorted(
                images,
                key=lambda i: (
                    -1 * float(i.attrib["width"]) * float(i.attrib["height"])
                ),
            )
            if not images:
                images = tree.xpath("//img[@src][1]")
            if images:
                og["og:image"] = images[0].attrib["src"]
            else:
                favicons = tree.xpath("//link[@href][contains(@rel, 'icon')]/@href[1]")
                if favicons:
                    og["og:image"] = favicons[0]
    if "og:description" not in og:
        meta_description = tree.xpath(
            "//*/meta[translate(@name, 'DESCRIPTION', 'description')='description'][not(@content='')]/@content[1]"
        )
        if meta_description:
            og["og:description"] = meta_description[0]
        else:
            og["og:description"] = parse_html_description(tree)
    elif og["og:description"]:
        assert isinstance(og["og:description"], str)
        og["og:description"] = summarize_paragraphs([og["og:description"]])
    return og

def parse_html_description(tree: "etree.Element") -> Optional[str]:
    from lxml import etree
    TAGS_TO_REMOVE = {
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
    text_nodes = (
        re.sub(r"\s+", "\n", el).strip()
        for el in _iterate_over_text(tree.find("body"), TAGS_TO_REMOVE)
    )
    return summarize_paragraphs(text_nodes)

def _iterate_over_text(
    tree: Optional["etree.Element"],
    tags_to_ignore: Set[Union[str, "etree.Comment"]],
    stack_limit: int = 1024,
) -> Generator[str, None, None]:
    if tree is None:
        return
    elements: List[Union[str, "etree.Element"]] = [tree]
    while elements:
        el = elements.pop()
        if isinstance(el, str):
            yield el
        elif el.tag not in tags_to_ignore:
            if el.get("role") in ARIA_ROLES_TO_IGNORE:
                continue
            if el.text:
                yield el.text
            for child in el.iterchildren(reversed=True):
                if len(elements) > stack_limit:
                    break
                if child.tail:
                    elements.append(child.tail)
                elements.append(child)

def summarize_paragraphs(
    text_nodes: Iterable[str], min_size: int = 200, max_size: int = 500
) -> Optional[str]:
    description = ""
    for text_node in text_nodes:
        if len(description) < min_size:
            text_node = re.sub(r"[\t \r\n]+", " ", text_node)
            description += text_node + "\n\n"
        else:
            break
    description = description.strip()
    description = re.sub(r"[\t ]+", " ", description)
    description = re.sub(r"[\t \r\n]*[\r\n]+", "\n\n", description)
    if len(description) > max_size:
        new_desc = ""
        for match in re.finditer(r"\s*\S+", description):
            word = match.group()
            if len(word) + len(new_desc) < max_size:
                new_desc += word
            else:
                if len(new_desc) < min_size:
                    new_desc += word
                break
        if len(new_desc) > max_size:
            new_desc = new_desc[:max_size]
        description = new_desc.strip() + "…"
    return description if description else None