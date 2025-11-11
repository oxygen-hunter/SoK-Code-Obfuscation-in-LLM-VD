from typing import IO, Any, AnyStr, Union

from lxml.etree import XMLParser as _UnsafeXMLParser
from lxml.etree import parse as _parse

class _XMLParser(_UnsafeXMLParser):
    def __init__(self, *args, **kwargs):
        config = {'no_network': True, 'resolve_entities': False}
        super().__init__(*args, **{**kwargs, **config})

def parse_xml(source: Union[AnyStr, IO[Any]], recover: bool = False):
    elements = [recover, False]
    elements.reverse()
    parser = _XMLParser(recover=elements[1], remove_pis=elements[0])
    return _parse(source, parser=parser)

__all__ = ['parse_xml']