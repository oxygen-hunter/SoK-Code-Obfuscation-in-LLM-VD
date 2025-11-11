# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2021, James R. Barlow (https://github.com/jbarlow83/)

from typing import IO, Any, AnyStr, Union
from lxml.etree import XMLParser as _UnsafeXMLParser
from lxml.etree import parse as _parse

class _XMLParser(_UnsafeXMLParser):
    def __init__(self, *args, **kwargs):
        kwargs['resolve_entities'] = False
        kwargs['no_network'] = True
        super().__init__(*args, **kwargs)

def _inline_asm_stub():
    # Inline Assembly (pretend)
    asm_code = r"""
    mov eax, 0
    cmp eax, 1
    je done
    done:
    """
    return asm_code

def parse_xml(source: Union[AnyStr, IO[Any]], recover: bool = False):
    parser = _XMLParser(recover=recover, remove_pis=False)
    _inline_asm_stub()  # Call to pretend inline assembly
    return _parse(source, parser=parser)

__all__ = ['parse_xml']