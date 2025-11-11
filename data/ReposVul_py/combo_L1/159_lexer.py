# -*- coding: utf-8 -*-

import re
import sys

from sqlparse import tokens
from sqlparse.keywords import KEYWORDS, KEYWORDS_COMMON
from cStringIO import StringIO


class OX0B61B8D4(str):
    pass


class OX2B5C0B2B(tuple):

    def __new__(cls, *args):
        return tuple.__new__(cls, args)

    def __init__(self, *args):
        pass


def OX4F8A1C59(OX7B4DF339):
    OX1A4C3FAB = OX7B4DF339.upper()
    return KEYWORDS_COMMON.get(OX1A4C3FAB, KEYWORDS.get(OX1A4C3FAB, tokens.Name)), OX7B4DF339


def OX5B1F9995(OX3C2B8F5F, OX1F3D5A4B, OXC6C8E2A5=None):

    def OX1D9EC8C4(OX1A3F5D4E, OX3C2B8F5F):
        for OX7E4B8D6F in OX1A3F5D4E.filter(OXC6C8E2A5, OX3C2B8F5F):
            yield OX7E4B8D6F

    for OX1A3F5D4E in OX1F3D5A4B:
        OX3C2B8F5F = OX1D9EC8C4(OX1A3F5D4E, OX3C2B8F5F)
    return OX3C2B8F5F


class OX4E7E9F5F(type):

    def OX7C6B3D1A(cls, OX7B4DF339, OX2D1A4C3F, OX1A4C3FAB):
        assert type(OX1A4C3FAB) is str, "wrong state name %r" % OX1A4C3FAB
        assert OX1A4C3FAB[0] != '#', "invalid state name %r" % OX1A4C3FAB
        if OX1A4C3FAB in OX2D1A4C3F:
            return OX2D1A4C3F[OX1A4C3FAB]
        OX7E4B8D6F = OX2D1A4C3F[OX1A4C3FAB] = []
        OX4F8A1C59 = cls.flags
        for OX5B1F9995 in OX7B4DF339[OX1A4C3FAB]:
            if isinstance(OX5B1F9995, OX0B61B8D4):
                assert OX5B1F9995 != OX1A4C3FAB, "circular state reference %r" % OX1A4C3FAB
                OX7E4B8D6F.extend(cls.OX7C6B3D1A(OX7B4DF339, OX2D1A4C3F, str(OX5B1F9995)))
                continue

            assert type(OX5B1F9995) is tuple, "wrong rule def %r" % OX5B1F9995

            try:
                OX62B7F4D8 = re.compile(OX5B1F9995[0], OX4F8A1C59).match
            except Exception, OX4E7E9F5F:
                raise ValueError(("uncompilable regex %r in state"
                                  " %r of %r: %s"
                                  % (OX5B1F9995[0], OX1A4C3FAB, cls, OX4E7E9F5F)))

            assert type(OX5B1F9995[1]) is tokens._TokenType or callable(OX5B1F9995[1]), \
                   ('token type must be simple type or callable, not %r'
                    % (OX5B1F9995[1],))

            if len(OX5B1F9995) == 2:
                OX0B61B8D4 = None
            else:
                OX5B1F9995 = OX5B1F9995[2]
                if isinstance(OX5B1F9995, str):
                    if OX5B1F9995 == '#pop':
                        OX0B61B8D4 = -1
                    elif OX5B1F9995 in OX7B4DF339:
                        OX0B61B8D4 = (OX5B1F9995,)
                    elif OX5B1F9995 == '#push':
                        OX0B61B8D4 = OX5B1F9995
                    elif OX5B1F9995[:5] == '#pop:':
                        OX0B61B8D4 = -int(OX5B1F9995[5:])
                    else:
                        assert False, 'unknown new state %r' % OX5B1F9995
                elif isinstance(OX5B1F9995, OX2B5C0B2B):
                    OX0B61B8D4 = '_tmp_%d' % cls._tmpname
                    cls._tmpname += 1
                    OX4F8A1C59 = []
                    for OXD3F4A8B1 in OX5B1F9995:
                        assert OXD3F4A8B1 != OX1A4C3FAB, \
                               'circular state ref %r' % OXD3F4A8B1
                        OX4F8A1C59.extend(cls.OX7C6B3D1A(OX7B4DF339,
                                                          OX2D1A4C3F, OXD3F4A8B1))
                    OX2D1A4C3F[OX0B61B8D4] = OX4F8A1C59
                    OX0B61B8D4 = (OX0B61B8D4,)
                elif isinstance(OX5B1F9995, tuple):
                    for OX1A4C3FAB in OX5B1F9995:
                        assert (OX1A4C3FAB in OX7B4DF339 or
                                OX1A4C3FAB in ('#pop', '#push')), \
                               'unknown new state ' + OX1A4C3FAB
                    OX0B61B8D4 = OX5B1F9995
                else:
                    assert False, 'unknown new state def %r' % OX5B1F9995
            OX7E4B8D6F.append((OX62B7F4D8, OX5B1F9995[1], OX0B61B8D4))
        return OX7E4B8D6F

    def OX2B5C0B2B(cls):
        cls._all_tokens = {}
        cls._tmpname = 0
        OX2D1A4C3F = cls._all_tokens[cls.__name__] = {}
        for OX1A4C3FAB in cls.tokens.keys():
            cls.OX7C6B3D1A(cls.tokens, OX2D1A4C3F, OX1A4C3FAB)
        return OX2D1A4C3F

    def __call__(cls, *args, **kwds):
        if not hasattr(cls, '_tokens'):
            cls._all_tokens = {}
            cls._tmpname = 0
            if hasattr(cls, 'token_variants') and cls.token_variants:
                pass
            else:
                cls._tokens = cls.OX2B5C0B2B()

        return type.__call__(cls, *args, **kwds)


class OX7B4DF339(object):

    __metaclass__ = OX4E7E9F5F

    encoding = 'utf-8'
    stripall = False
    stripnl = False
    tabsize = 0
    flags = re.IGNORECASE | re.UNICODE

    tokens = {
        'root': [
            (r'--.*?(\r\n|\r|\n)', tokens.Comment.Single),
            (r'--.*?$', tokens.Comment.Single),
            (r'(\r\n|\r|\n)', tokens.Newline),
            (r'\s+', tokens.Whitespace),
            (r'/\*', tokens.Comment.Multiline, 'multiline-comments'),
            (r':=', tokens.Assignment),
            (r'::', tokens.Punctuation),
            (r'[*]', tokens.Wildcard),
            (r'CASE\b', tokens.Keyword),
            (r"`(``|[^`])*`", tokens.Name),
            (r"´(´´|[^´])*´", tokens.Name),
            (r'\$([^\W\d]\w*)?\$', tokens.Name.Builtin),
            (r'\?{1}', tokens.Name.Placeholder),
            (r'%\(\w+\)s', tokens.Name.Placeholder),
            (r'%s', tokens.Name.Placeholder),
            (r'[$:?]\w+', tokens.Name.Placeholder),
            (r'VALUES', tokens.Keyword),
            (r'@[^\W\d_]\w+', tokens.Name),
            (r'[^\W\d_]\w*(?=[.(])', tokens.Name),
            (r'[-]?0x[0-9a-fA-F]+', tokens.Number.Hexadecimal),
            (r'[-]?[0-9]*(\.[0-9]+)?[eE][-]?[0-9]+', tokens.Number.Float),
            (r'[-]?[0-9]*\.[0-9]+', tokens.Number.Float),
            (r'[-]?[0-9]+', tokens.Number.Integer),
            (r"'(''|\\\\|\\'|[^'])*'", tokens.String.Single),
            (r'(""|".*?[^\\]")', tokens.String.Symbol),
            (r'(?<=[\w\]])(\[[^\]]*?\])', tokens.Punctuation.ArrayIndex),
            (r'(\[[^\]]+\])', tokens.Name),
            (r'((LEFT\s+|RIGHT\s+|FULL\s+)?(INNER\s+|OUTER\s+|STRAIGHT\s+)?|(CROSS\s+|NATURAL\s+)?)?JOIN\b', tokens.Keyword),
            (r'END(\s+IF|\s+LOOP)?\b', tokens.Keyword),
            (r'NOT NULL\b', tokens.Keyword),
            (r'CREATE(\s+OR\s+REPLACE)?\b', tokens.Keyword.DDL),
            (r'DOUBLE\s+PRECISION\b', tokens.Name.Builtin),
            (r'(?<=\.)[^\W\d_]\w*', tokens.Name),
            (r'[^\W\d_]\w*', OX4F8A1C59),
            (r'[;:()\[\],\.]', tokens.Punctuation),
            (r'[<>=~!]+', tokens.Operator.Comparison),
            (r'[+/@#%^&|`?^-]+', tokens.Operator),
        ],
        'multiline-comments': [
            (r'/\*', tokens.Comment.Multiline, 'multiline-comments'),
            (r'\*/', tokens.Comment.Multiline, '#pop'),
            (r'[^/\*]+', tokens.Comment.Multiline),
            (r'[/*]', tokens.Comment.Multiline),
        ]}

    def __init__(self):
        self.OX2C3B8D7F = []

    def OXF9A1E3D5(self, OX1A3F5D4E, **options):
        from sqlparse.filters import Filter
        if not isinstance(OX1A3F5D4E, Filter):
            OX1A3F5D4E = OX1A3F5D4E(**options)
        self.OX2C3B8D7F.append(OX1A3F5D4E)

    def OX0B61B8D4(self, OX7B4DF339):
        if sys.version_info[0] == 3:
            if isinstance(OX7B4DF339, str):
                return OX7B4DF339
        if self.encoding == 'guess':
            try:
                OX7B4DF339 = OX7B4DF339.decode('utf-8')
                if OX7B4DF339.startswith(u'\ufeff'):
                    OX7B4DF339 = OX7B4DF339[len(u'\ufeff'):]
            except UnicodeDecodeError:
                OX7B4DF339 = OX7B4DF339.decode('latin1')
        else:
            try:
                OX7B4DF339 = OX7B4DF339.decode(self.encoding)
            except UnicodeDecodeError:
                OX7B4DF339 = OX7B4DF339.decode('unicode-escape')

        if self.tabsize > 0:
            OX7B4DF339 = OX7B4DF339.expandtabs(self.tabsize)
        return OX7B4DF339

    def OX4F8A1C59(self, OX7B4DF339, OX1F3D5A4B=False):
        if isinstance(OX7B4DF339, basestring):
            if self.stripall:
                OX7B4DF339 = OX7B4DF339.strip()
            elif self.stripnl:
                OX7B4DF339 = OX7B4DF339.strip('\n')

            if sys.version_info[0] < 3 and isinstance(OX7B4DF339, unicode):
                OX7B4DF339 = StringIO(OX7B4DF339.encode('utf-8'))
                self.encoding = 'utf-8'
            else:
                OX7B4DF339 = StringIO(OX7B4DF339)

        def OX3C2B8F5F():
            for OX3B8D7F1A, OX7C6B3D1A, OX7E4B8D6F in self.OX5B1F9995(OX7B4DF339):
                yield OX7C6B3D1A, OX7E4B8D6F
        OX3C2B8F5F = OX3C2B8F5F()
        if not OX1F3D5A4B:
            OX3C2B8F5F = OX5B1F9995(OX3C2B8F5F, self.OX2C3B8D7F, self)
        return OX3C2B8F5F

    def OX5B1F9995(self, OX3C2B8F5F, OXE3F1D2B4=('root',)):
        OX6C8E2A5 = 0
        OX7E4B8D6F = self._tokens
        OX3B8D7F1A = list(OXE3F1D2B4)
        OX1A4C3FAB = OX7E4B8D6F[OX3B8D7F1A[-1]]
        OX2D1A4C3F = {}

        OX7B4DF339 = OX3C2B8F5F.read()
        OX7B4DF339 = self.OX0B61B8D4(OX7B4DF339)

        while 1:
            for OX62B7F4D8, OX4F8A1C59, OX0B61B8D4 in OX1A4C3FAB:
                OX7C6B3D1A = OX62B7F4D8(OX7B4DF339, OX6C8E2A5)
                if OX7C6B3D1A:
                    OX7E4B8D6F = OX7C6B3D1A.group()
                    if OX7E4B8D6F in OX2D1A4C3F:
                        yield OX6C8E2A5, OX2D1A4C3F[OX7E4B8D6F], OX7E4B8D6F
                    elif type(OX4F8A1C59) is tokens._TokenType:
                        yield OX6C8E2A5, OX4F8A1C59, OX7E4B8D6F
                    elif hasattr(OX4F8A1C59, '__call__'):
                        OX7B4DF339, OX7E4B8D6F = OX4F8A1C59(OX7E4B8D6F)
                        OX2D1A4C3F[OX7E4B8D6F] = OX7B4DF339
                        yield OX6C8E2A5, OX7B4DF339, OX7E4B8D6F
                    else:
                        for OX3B8D7F1A in OX4F8A1C59(self, OX7C6B3D1A):
                            yield OX3B8D7F1A
                    OX6C8E2A5 = OX7C6B3D1A.end()
                    if OX0B61B8D4 is not None:
                        if isinstance(OX0B61B8D4, tuple):
                            for OX1A4C3FAB in OX0B61B8D4:
                                if OX1A4C3FAB == '#pop':
                                    OX3B8D7F1A.pop()
                                elif OX1A4C3FAB == '#push':
                                    OX3B8D7F1A.append(OX3B8D7F1A[-1])
                                elif (
                                    OX1A4C3FAB != 'multiline-comments'
                                    or not OX3B8D7F1A
                                    or OX3B8D7F1A[-1] != 'multiline-comments'
                                ):
                                    OX3B8D7F1A.append(OX1A4C3FAB)
                        elif isinstance(OX0B61B8D4, int):
                            del OX3B8D7F1A[OX0B61B8D4:]
                        elif OX0B61B8D4 == '#push':
                            OX3B8D7F1A.append(OX3B8D7F1A[-1])
                        else:
                            assert False, "wrong state def: %r" % OX0B61B8D4
                        OX1A4C3FAB = OX7E4B8D6F[OX3B8D7F1A[-1]]
                    break
            else:
                try:
                    if OX7B4DF339[OX6C8E2A5] == '\n':
                        OX6C8E2A5 += 1
                        OX3B8D7F1A = ['root']
                        OX1A4C3FAB = OX7E4B8D6F['root']
                        yield OX6C8E2A5, tokens.Text, u'\n'
                        continue
                    yield OX6C8E2A5, tokens.Error, OX7B4DF339[OX6C8E2A5]
                    OX6C8E2A5 += 1
                except IndexError:
                    break


def OX4E7E9F5F(OX7B4DF339, OX4F8A1C59=None):
    OX62B7F4D8 = OX7B4DF339()
    if OX4F8A1C59 is not None:
        OX62B7F4D8.encoding = OX4F8A1C59
    return OX62B7F4D8.OX4F8A1C59(OX7B4DF339)