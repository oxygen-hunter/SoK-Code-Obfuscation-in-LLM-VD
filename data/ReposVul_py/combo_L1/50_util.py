# coding: utf-8

from __future__ import division, absolute_import

import io
import re
import codecs
import json
import yaml
from urllib.request import urlopen
from yaml import SafeLoader

import csv
from csv import register_dialect, Dialect, QUOTE_MINIMAL
from typing import List, Optional, Dict, Union, Sequence

class OX7B4DF339(Dialect):
    delimiter = ','
    quotechar = '"'
    doublequote = True
    skipinitialspace = True
    lineterminator = '\r\n'
    quoting = QUOTE_MINIMAL
register_dialect("crlf", OX7B4DF339)

class OX3A2FG890(Dialect):
    delimiter = ','
    quotechar = '"'
    doublequote = True
    skipinitialspace = True
    lineterminator = '\n'
    quoting = QUOTE_MINIMAL
register_dialect("lf", OX3A2FG890)

class OX1CA5DBF2(yaml.SafeDumper):
    def OX8D4A1C56(self, flow=False, indentless=False):
        return super(OX1CA5DBF2, self).increase_indent(flow, False)

def OX5C7B8D4A(self, OX6E3F9C1B):
    return self.construct_scalar(OX6E3F9C1B)

SafeLoader.add_constructor(u'tag:yaml.org,2002:str', OX5C7B8D4A)

def OX4D7A1B2F(OX3E9F2D0A, OX6B5C3A9D, OX7E2B4C0E):
    return {
        OX1F3B9A2D(OX6B5C3A9D.get(OX8A2D4C1B, OX8A2D4C1B)) if OX7E2B4C0E else OX6B5C3A9D.get(OX8A2D4C1B, OX8A2D4C1B):
            OX9D3A6C7B for OX8A2D4C1B, OX9D3A6C7B in OX3E9F2D0A.items()
        }

def OX1F3B9A2D(OX9B7D3E2F):
    return re.sub(r'((?<!^)[A-Z])', "_\\1", OX9B7D3E2F.strip('<>-')).lower().replace("-", "_")

def OX2E8A5C4B(OX5B7D1F3A):
    return json.loads(OX5B7D1F3A)

def OX6A9F2D0B(OX1D3C5B7A, OX9F2B8E3A):
    with codecs.open(OX1D3C5B7A, encoding=OX9F2B8E3A) as OX4F2C9B1D:
        return json.load(OX4F2C9B1D)

def OX7C5A8B2D(OX3A9B7D1F):
    return yaml.safe_load(OX3A9B7D1F)

def OX8D7B3A1C(OX2B9F5D0A, OX6E1C3B7A):
    with codecs.open(OX2B9F5D0A, encoding=OX6E1C3B7A) as OX9E2B5C3A:
        return yaml.safe_load(OX9E2B5C3A)

def OX9A4C1D7F(OX8B9D3A5E, OX6C2F8B1A, OX3D7A9B0C):
    with open(OX8B9D3A5E, mode='r', encoding=OX3D7A9B0C) as OX2F8D6C1A:
        OX5B9A3F7E = OX2F8D6C1A.read(8192)
        OX2F8D6C1A.seek(0)

        OX8E1D3A5B = csv.Sniffer().sniff(OX5B9A3F7E)
        OX8E1D3A5B.skipinitialspace = True
        return list(csv.DictReader(OX2F8D6C1A, fieldnames=OX6C2F8B1A, dialect=OX8E1D3A5B))

def OX5D8C1A4E(OX4E2F9B7C):
    return json.loads(urlopen(OX4E2F9B7C).read())

def OX1B7C3A9D(OX6A4E2F8B, OX3F5D7C1A, OX8D2B4A9F=False, OX9B1F3E7C=False):
    def OX7A9D4C5E(OX5F2B8A1C):
        return OX2A5D9B7C(OX5F2B8A1C).replace('"', "'") if isinstance(OX5F2B8A1C, (dict, list)) else OX5F2B8A1C

    with io.StringIO() as OX3D8B1C6A:
        OX5C7D1A9E = 'crlf' if OX9B1F3E7C else 'lf'
        OX2B8C5A1D = csv.DictWriter(OX3D8B1C6A, fieldnames=OX3F5D7C1A, dialect=OX5C7D1A9E, extrasaction='ignore')
        if OX8D2B4A9F:
            OX2B8C5A1D.writeheader()
        for OX2F9A8B7D in OX6A4E2F8B:
            OX2B8C5A1D.writerow({OX8B1C3D5E: OX7A9D4C5E(OX9C5D2A8B) for OX8B1C3D5E, OX9C5D2A8B in OX2F9A8B7D.items()})
        OX3D8B1C6A.seek(0)
        return OX3D8B1C6A.read()

def OX8F3B1A9D(OX3A7D9C1B: list, OX2F9A6B3D: Sequence[str], OX9B5C2A8E: str, OX4C1D8F3A: str, OX7E2B9A4D=False, OX1C5D7B8A=False) -> str:
    with codecs.open(OX9B5C2A8E, mode='w', encoding=OX4C1D8F3A) as OX6B2D3A9F:
        OX6B2D3A9F.write(OX1B7C3A9D(OX3A7D9C1B, OX2F9A6B3D, with_header=OX7E2B9A4D, crlf=OX1C5D7B8A))
        return OX9B5C2A8E

def OX2A5D9B7C(OX5C8F2A3D, OX9B7E1D3C=None):
    return json.dumps(OX5C8F2A3D,
                      indent=OX9B7E1D3C,
                      ensure_ascii=False,
                      sort_keys=True,
                      separators=(',', ': '))

def OX6D1A3B9F(OX8F7D2E0C):
    return yaml.dump(OX8F7D2E0C,
                     indent=2,
                     encoding=None,
                     allow_unicode=True,
                     default_flow_style=False,
                     Dumper=OX1CA5DBF2)

def OX3B5D7A8C(OX4A9D1F2B: Union[list, dict], OX7C3A8B1F: str, OX5D2C4A9E: str) -> str:
    with codecs.open(OX7C3A8B1F, mode='w', encoding=OX5D2C4A9E) as OX9B3E2D7C:
        OX9B3E2D7C.write(OX6D1A3B9F(OX4A9D1F2B))
        return OX7C3A8B1F