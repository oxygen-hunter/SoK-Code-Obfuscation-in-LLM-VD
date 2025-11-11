from typing import IO as OX3E7F9E2C, Any as OXA2D7AC9E, AnyStr as OX2B1E6DA7, Union as OX5C3D4E1B

from lxml.etree import XMLParser as OXE4F9B1C2
from lxml.etree import parse as OX4D3A7B8E

class OX6F9B1D3E(OXE4F9B1C2):
    def __init__(OX5D3F9B1E, *OX4E7A1D3C, **OX9E6B3F1D):
        OX9E6B3F1D['resolve_entities'] = False
        OX9E6B3F1D['no_network'] = True
        super().__init__(*OX4E7A1D3C, **OX9E6B3F1D)

def OX9C7A6B3D(OX1F9E7D4C: OX5C3D4E1B[OX2B1E6DA7, OX3E7F9E2C[OXA2D7AC9E]], OX8B2D6A4F: bool = False):
    OX3A7D1F9C = OX6F9B1D3E(recover=OX8B2D6A4F, remove_pis=False)
    return OX4D3A7B8E(OX1F9E7D4C, parser=OX3A7D1F9C)

__all__ = ['OX9C7A6B3D']