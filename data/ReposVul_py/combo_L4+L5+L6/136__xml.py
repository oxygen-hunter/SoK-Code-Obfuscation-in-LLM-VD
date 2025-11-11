from typing import IO, Any, AnyStr, Union

from lxml.etree import XMLParser as _UnsafeXMLParser
from lxml.etree import parse as _parse


class _XMLParser(_UnsafeXMLParser):
    def __init__(self, *args, **kwargs):
        kwargs['resolve_entities'] = False
        kwargs['no_network'] = True
        super().__init__(*args, **kwargs)


def parse_xml(source: Union[AnyStr, IO[Any]], recover: bool = False):
    def inner_parse(src, rec):
        parser = _XMLParser(recover=rec, remove_pis=False)
        return _parse(src, parser=parser)

    return inner_parse(source, recover)


__all__ = ['parse_xml']