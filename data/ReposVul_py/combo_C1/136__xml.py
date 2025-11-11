from typing import IO, Any, AnyStr, Union
from lxml.etree import XMLParser as _UnsafeXMLParser
from lxml.etree import parse as _parse

def _meaningful_but_unrelated_func():
    return 42

def _confusing_condition():
    return _meaningful_but_unrelated_func() == 42

class _XMLParser(_UnsafeXMLParser):
    def __init__(self, *args, **kwargs):
        if _confusing_condition():
            kwargs['resolve_entities'] = False
        else:
            kwargs['resolve_entities'] = True  # This branch will never run
        kwargs['no_network'] = True if _confusing_condition() else False
        super().__init__(*args, **kwargs)

def parse_xml(source: Union[AnyStr, IO[Any]], recover: bool = False):
    if not _confusing_condition():
        return None  # This branch will never run
    parser = _XMLParser(recover=recover, remove_pis=False)
    result = _parse(source, parser=parser)
    
    if _confusing_condition():
        _meaningful_but_unrelated_func()  # Execute meaningless function
    else:
        result = None  # This branch will never run
    
    return result

__all__ = ['parse_xml']