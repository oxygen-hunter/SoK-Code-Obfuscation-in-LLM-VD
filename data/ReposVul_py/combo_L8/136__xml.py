import ctypes
from typing import IO, Any, AnyStr, Union
from lxml.etree import XMLParser as _UnsafeXMLParser
from lxml.etree import parse as _parse

# Load C DLL
libc = ctypes.CDLL(None)

class _XMLParser(_UnsafeXMLParser):
    def __init__(self, *args, **kwargs):
        # Set flags using C
        libc.setbuf.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
        libc.setbuf.restype = None
        libc.setbuf(None, None)
        
        kwargs['resolve_entities'] = False
        kwargs['no_network'] = True
        super().__init__(*args, **kwargs)

def parse_xml(source: Union[AnyStr, IO[Any]], recover: bool = False):
    """Wrapper around lxml's parse to provide protection against XXE attacks."""
    # C function to convert string
    libc.strlen.argtypes = [ctypes.c_char_p]
    libc.strlen.restype = ctypes.c_size_t

    parser = _XMLParser(recover=recover, remove_pis=False)
    return _parse(source, parser=parser)

__all__ = ['parse_xml']