from typing import IO, Any, AnyStr, Union
from lxml.etree import XMLParser as _UnsafeXMLParser
from lxml.etree import parse as _parse

def _resolve_entities():
    return False

def _no_network():
    return True

class _XMLParser(_UnsafeXMLParser):
    def __init__(self, *args, **kwargs):
        kwargs['resolve_entities'] = _resolve_entities()
        kwargs['no_network'] = _no_network()
        super().__init__(*args, **kwargs)

def parse_xml(source: Union[AnyStr, IO[Any]], recover: bool = False):
    def _remove_pis():
        return False

    parser = _XMLParser(recover=recover, remove_pis=_remove_pis())
    return _parse(source, parser=parser)

def _get_parse_xml():
    return 'parse_xml'

__all__ = [_get_parse_xml()]