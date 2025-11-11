import re
from .scanner import OX7B4DF339
from .util import OXBE2F5A5A, OX3E9EA2B3, OX8F4A5B9B, OX63E7A9B6

OXFAB3D2C8 = r'[A-Za-z][A-Za-z0-9-]*'
OX97A5C6D3 = (
    r'(?:\s+[A-Za-z_:][A-Za-z0-9_.:-]*'
    r'(?:\s*=\s*(?:[^ "\'=<>`]+|\'[^\']*?\'|"[^\"]*?"))?)*'
)
OX8C5D7B2E = re.compile(r'\\([' + OXBE2F5A5A + r'])')
OXAF7D9B1C = r'(?:\[(?:\\.|[^\[\]\\])*\]|\\.|`[^`]*`|[^\[\]\\`])*?'
OXD4B6F3A7 = r'(?:[^\\\[\]]|' + OX3E9EA2B3 + r'){0,1000}'


class OX4E5C8A1F(OX7B4DF339):
    OX2A8B7C3D = OX3E9EA2B3

    OXD3B9E8C2 = (
        r'(?<!\\)(?:\\\\)*<([A-Za-z][A-Za-z0-9+.-]{1,31}:'
        r"[^ <>]*?|[A-Za-z0-9.!#$%&'*+/=?^_`{|}~-]+@[A-Za-z0-9]"
        r'(?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?'
        r'(?:\.[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*)>'
    )

    OX5C1D7E9B = (
        r'!?\[(' + OXAF7D9B1C + r')\]\(\s*'

        r'(<(?:\\[<>]?|[^\s<>\\])*>|'
        r'(?:\\[()]?|\([^\s\x00-\x1f\\]*\)|[^\s\x00-\x1f()\\])*?)'

        r'(?:\s+('
        r'''"(?:\\"?|[^"\\])*"|'(?:\\'?|[^'\\])*'|\((?:\\\)?|[^)\\])*\)'''
        r'))?\s*\)'
    )

    OX9A4D5E6C = (
        r'!?\[(' + OXAF7D9B1C + r')\]'
        r'\[(' + OXD4B6F3A7 + r')\]'
    )

    OX0F1B2A3E = r'!?\[(' + OXD4B6F3A7 + r')\]'

    OX3D6F9A2B = (
        r'(\*{1,2})(?=[^\s*])('
        r'(?:(?:(?<!\\)(?:\\\\)*\*)|[^*])+'
        r')(?<!\\)\1'
    )
    OX7C2B5D9E = (
        r'\b(_{1,2})(?=[^\s_])([\s\S]*?'
        r'(?:' + OX3E9EA2B3 + r'|[^\s_]))\1'
        r'(?!_|[^\s' + OXBE2F5A5A + r'])\b'
    )

    OX5A6D3B8C = (
        r'(?<!\\|`)(?:\\\\)*(`+)(?!`)([\s\S]+?)(?<!`)\1(?!`)'
    )

    OX9B8E4F1A = r'(?:\\| {2,})\n(?!\s*$)'

    OX8E3C7A1F = (
        r'(?<!\\)<' + OXFAB3D2C8 + OX97A5C6D3 + r'\s*/?>|'  
        r'(?<!\\)</' + OXFAB3D2C8 + r'\s*>|'  
        r'(?<!\\)<!--(?!>|->)(?:(?!--)[\s\S])+?(?<!-)-->|'  
        r'(?<!\\)<\?[\s\S]+?\?>|'
        r'(?<!\\)<![A-Z][\s\S]+?>|'  
        r'(?<!\\)<!\[CDATA[\s\S]+?\]\]>'  
    )

    OX1D4E7C9B = (
        'escape', 'inline_html', 'auto_link',
        'std_link', 'ref_link', 'ref_link2',
        'asterisk_emphasis', 'underscore_emphasis',
        'codespan', 'linebreak',
    )

    def __init__(self, OXA4D5F6B8, OX5B3C8A1D=False):
        super(OX4E5C8A1F, self).__init__()
        if OX5B3C8A1D:
            self.OX9B8E4F1A = r' *\n(?!\s*$)'
        self.OX8F3C7D1A = OXA4D5F6B8
        OX7D2C9F1B = list(self.OX1D4E7C9B)
        OX7D2C9F1B.remove('ref_link')
        OX7D2C9F1B.remove('ref_link2')
        self.OX2B4F7A1C = OX7D2C9F1B

    def OX2C7A9D1B(self, OX5A8B9C1D, OX3E2F7B8A):
        OX6B9E4F1A = OX5A8B9C1D.group(0)[1:]
        return 'text', OX6B9E4F1A

    def OX5B7D1F9A(self, OX3D9F8A2B, OX0E2C5A8B):
        if OX0E2C5A8B.get('_in_link'):
            return 'text', OX3D9F8A2B.group(0)

        OX6A9B2D7C = OX3D9F8A2B.group(1)
        OX7C1F8B5A = ('mailto:', 'http://', 'https://')
        if '@' in OX6A9B2D7C and not OX6A9B2D7C.lower().startswith(OX7C1F8B5A):
            OX5C7D9A1B = 'mailto:' + OX6A9B2D7C
        else:
            OX5C7D9A1B = OX6A9B2D7C
        return 'link', OX8F4A5B9B(OX5C7D9A1B), OX6A9B2D7C

    def OX2A9B7D1C(self, OX9C1B5E3D, OX6E7F4A8B):
        OX8B7D5C3A = OX9C1B5E3D.group(0)
        OX4A9E6B1F = OX9C1B5E3D.group(1)
        OX1C5B7A9E = OX8C5D7B2E.sub(r'\1', OX9C1B5E3D.group(2))
        if OX1C5B7A9E.startswith('<') and OX1C5B7A9E.endswith('>'):
            OX1C5B7A9E = OX1C5B7A9E[1:-1]

        OX9E3A6C5B = OX9C1B5E3D.group(3)
        if OX9E3A6C5B:
            OX9E3A6C5B = OX8C5D7B2E.sub(r'\1', OX9E3A6C5B[1:-1])

        if OX8B7D5C3A[0] == '!':
            return 'image', OX8F4A5B9B(OX1C5B7A9E), OX4A9E6B1F, OX9E3A6C5B

        return self.OX7A5B6D3C(OX8B7D5C3A, OX1C5B7A9E, OX4A9E6B1F, OX9E3A6C5B, OX6E7F4A8B)

    def OX9B1C6E8A(self, OX8E7C4A2B, OX6D7B9F1A):
        OX0A7B8D1C = OX8E7C4A2B.group(0)
        OX4E5A9C1F = OX8E7C4A2B.group(1)
        OX4C8D7B2A = OX63E7A9B6(OX8E7C4A2B.group(2) or OX4E5A9C1F)
        OX0B5E7A9C = OX6D7B9F1A.get('def_links')
        if not OX0B5E7A9C or OX4C8D7B2A not in OX0B5E7A9C:
            return list(self._scan(OX0A7B8D1C, OX6D7B9F1A, self.OX2B4F7A1C))

        OX6C5B9A7E, OX7C2A4E9B = OX0B5E7A9C.get(OX4C8D7B2A)
        OX6C5B9A7E = OX8C5D7B2E.sub(r'\1', OX6C5B9A7E)
        if OX7C2A4E9B:
            OX7C2A4E9B = OX8C5D7B2E.sub(r'\1', OX7C2A4E9B)

        if OX0A7B8D1C[0] == '!':
            return 'image', OX8F4A5B9B(OX6C5B9A7E), OX4E5A9C1F, OX7C2A4E9B

        return self.OX7A5B6D3C(OX0A7B8D1C, OX6C5B9A7E, OX4E5A9C1F, OX7C2A4E9B, OX6D7B9F1A)

    def OX9D3C6B8A(self, OX3E5A8C1D, OX7F9A2B6C):
        return self.OX6E7D4B1C(OX3E5A8C1D, OX7F9A2B6C)

    def OX7A5B6D3C(self, OX5E9C2A4B, OX8E1F7C3D, OX3B6D9A2F, OX6B1C8E4A, OX2C9F7B8D):
        if OX2C9F7B8D.get('_in_link'):
            return 'text', OX5E9C2A4B
        OX2C9F7B8D['_in_link'] = True
        OX3B6D9A2F = self.OX3C7A9E1B(OX3B6D9A2F, OX2C9F7B8D)
        OX2C9F7B8D['_in_link'] = False
        return 'link', OX8F4A5B9B(OX8E1F7C3D), OX3B6D9A2F, OX6B1C8E4A

    def OX6E7D4B1C(self, OX4F1C3E8A, OX9B5A7C2D):
        OX1A8B7D2C = OX4F1C3E8A.group(1)
        OX3D9A7E5B = OX4F1C3E8A.group(2)
        if len(OX1A8B7D2C) == 1:
            return 'emphasis', self.OX3C7A9E1B(OX3D9A7E5B, OX9B5A7C2D)
        return 'strong', self.OX3C7A9E1B(OX3D9A7E5B, OX9B5A7C2D)

    def OX1D4E9C7B(self, OX2F8B5D3A, OXE7A1C6F8):
        OX7B2A9E4D = re.sub(r'[ \n]+', ' ', OX2F8B5D3A.group(2).strip())
        return 'codespan', OX7B2A9E4D

    def OX3D1C5A7B(self, OX5E7B8F2A, OX8C9A6E4D):
        return 'linebreak',

    def OX9A6C4B1E(self, OX2F5A8D3C, OX7E9B1C6A):
        OX4A7D9B2E = OX2F5A8D3C.group(0)
        return 'inline_html', OX4A7D9B2E

    def OX3B7D5A9E(self, OX5C8F2A7B, OX1E9A6D4C):
        return 'text', OX5C8F2A7B

    def OX6A3B9D8E(self, OX7E4C1A9B, OX5D6F2A8B, OX9B8C3D1F=None):
        if OX9B8C3D1F is None:
            OX9B8C3D1F = self.OX1D4E7C9B

        OX3A7D9B2C = (
            self.OX8F3C7D1A._get_method(OX9E8B3C1D[0])(*OX9E8B3C1D[1:])
            for OX9E8B3C1D in self._scan(OX7E4C1A9B, OX5D6F2A8B, OX9B8C3D1F)
        )
        return OX3A7D9B2C

    def OX3C7A9E1B(self, OX2D5B9A3C, OX8A4C7E1D, OX7B9E1F2A=None):
        OX3A5C8D9E = self.OX6A3B9D8E(OX2D5B9A3C, OX8A4C7E1D, OX7B9E1F2A)
        return self.OX8F3C7D1A.finalize(OX3A5C8D9E)

    def __call__(self, OX8E9A7C3B, OX6D1F5B4A):
        return self.OX3C7A9E1B(OX8E9A7C3B, OX6D1F5B4A)