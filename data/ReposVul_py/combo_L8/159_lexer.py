# -*- coding: utf-8 -*-
import re
import sys
from sqlparse import tokens
from sqlparse.keywords import KEYWORDS, KEYWORDS_COMMON
from cStringIO import StringIO
from ctypes import CDLL, c_char_p, c_void_p, c_size_t

libc = CDLL(None)

class include(str):
    pass

class combined(tuple):
    def __new__(cls, *args):
        return tuple.__new__(cls, args)

    def __init__(self, *args):
        pass

def is_keyword(value):
    test = value.upper()
    return KEYWORDS_COMMON.get(test, KEYWORDS.get(test, tokens.Name)), value

def apply_filters(stream, filters, lexer=None):
    def _apply(filter_, stream):
        for token in filter_.filter(lexer, stream):
            yield token

    for filter_ in filters:
        stream = _apply(filter_, stream)
    return stream

class LexerMeta(type):
    def _process_state(cls, unprocessed, processed, state):
        assert type(state) is str, "wrong state name %r" % state
        assert state[0] != '#', "invalid state name %r" % state
        if state in processed:
            return processed[state]
        tokenlist = processed[state] = []
        rflags = cls.flags
        for tdef in unprocessed[state]:
            if isinstance(tdef, include):
                assert tdef != state, "circular state reference %r" % state
                tokenlist.extend(cls._process_state(
                    unprocessed, processed, str(tdef)))
                continue

            assert type(tdef) is tuple, "wrong rule def %r" % tdef

            try:
                rex = re.compile(tdef[0], rflags).match
            except Exception as err:
                raise ValueError(("uncompilable regex %r in state"
                                  " %r of %r: %s"
                                  % (tdef[0], state, cls, err)))

            assert type(tdef[1]) is tokens._TokenType or callable(tdef[1]), \
                   ('token type must be simple type or callable, not %r'
                    % (tdef[1],))

            if len(tdef) == 2:
                new_state = None
            else:
                tdef2 = tdef[2]
                if isinstance(tdef2, str):
                    if tdef2 == '#pop':
                        new_state = -1
                    elif tdef2 in unprocessed:
                        new_state = (tdef2,)
                    elif tdef2 == '#push':
                        new_state = tdef2
                    elif tdef2[:5] == '#pop:':
                        new_state = -int(tdef2[5:])
                    else:
                        assert False, 'unknown new state %r' % tdef2
                elif isinstance(tdef2, combined):
                    new_state = '_tmp_%d' % cls._tmpname
                    cls._tmpname += 1
                    itokens = []
                    for istate in tdef2:
                        assert istate != state, \
                               'circular state ref %r' % istate
                        itokens.extend(cls._process_state(unprocessed,
                                                          processed, istate))
                    processed[new_state] = itokens
                    new_state = (new_state,)
                elif isinstance(tdef2, tuple):
                    for state in tdef2:
                        assert (state in unprocessed or
                                state in ('#pop', '#push')), \
                               'unknown new state ' + state
                    new_state = tdef2
                else:
                    assert False, 'unknown new state def %r' % tdef2
            tokenlist.append((rex, tdef[1], new_state))
        return tokenlist

    def process_tokendef(cls):
        cls._all_tokens = {}
        cls._tmpname = 0
        processed = cls._all_tokens[cls.__name__] = {}
        for state in cls.tokens.keys():
            cls._process_state(cls.tokens, processed, state)
        return processed

    def __call__(cls, *args, **kwds):
        if not hasattr(cls, '_tokens'):
            cls._all_tokens = {}
            cls._tmpname = 0
            if hasattr(cls, 'token_variants') and cls.token_variants:
                pass
            else:
                cls._tokens = cls.process_tokendef()

        return type.__call__(cls, *args, **kwds)

class Lexer(object):
    __metaclass__ = LexerMeta

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
            (r'[^\W\d_]\w*', is_keyword),
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
        self.filters = []

    def add_filter(self, filter_, **options):
        from sqlparse.filters import Filter
        if not isinstance(filter_, Filter):
            filter_ = filter_(**options)
        self.filters.append(filter_)

    def _decode(self, text):
        if sys.version_info[0] == 3:
            if isinstance(text, str):
                return text
        if self.encoding == 'guess':
            try:
                text = text.decode('utf-8')
                if text.startswith(u'\ufeff'):
                    text = text[len(u'\ufeff'):]
            except UnicodeDecodeError:
                text = text.decode('latin1')
        else:
            try:
                text = text.decode(self.encoding)
            except UnicodeDecodeError:
                text = text.decode('unicode-escape')

        if self.tabsize > 0:
            text = text.expandtabs(self.tabsize)
        return text

    def get_tokens(self, text, unfiltered=False):
        if isinstance(text, basestring):
            if self.stripall:
                text = text.strip()
            elif self.stripnl:
                text = text.strip('\n')

            if sys.version_info[0] < 3 and isinstance(text, unicode):
                text = StringIO(text.encode('utf-8'))
                self.encoding = 'utf-8'
            else:
                text = StringIO(text)

        def streamer():
            for i, t, v in self.get_tokens_unprocessed(text):
                yield t, v
        stream = streamer()
        if not unfiltered:
            stream = apply_filters(stream, self.filters, self)
        return stream

    def get_tokens_unprocessed(self, stream, stack=('root',)):
        pos = 0
        tokendefs = self._tokens
        statestack = list(stack)
        statetokens = tokendefs[statestack[-1]]
        known_names = {}

        text = stream.read()
        text = self._decode(text)

        while 1:
            for rexmatch, action, new_state in statetokens:
                m = rexmatch(text, pos)
                if m:
                    value = m.group()
                    if value in known_names:
                        yield pos, known_names[value], value
                    elif type(action) is tokens._TokenType:
                        yield pos, action, value
                    elif hasattr(action, '__call__'):
                        ttype, value = action(value)
                        known_names[value] = ttype
                        yield pos, ttype, value
                    else:
                        for item in action(self, m):
                            yield item
                    pos = m.end()
                    if new_state is not None:
                        if isinstance(new_state, tuple):
                            for state in new_state:
                                if state == '#pop':
                                    statestack.pop()
                                elif state == '#push':
                                    statestack.append(statestack[-1])
                                elif (
                                    state != 'multiline-comments'
                                    or not statestack
                                    or statestack[-1] != 'multiline-comments'
                                ):
                                    statestack.append(state)
                        elif isinstance(new_state, int):
                            del statestack[new_state:]
                        elif new_state == '#push':
                            statestack.append(statestack[-1])
                        else:
                            assert False, "wrong state def: %r" % new_state
                        statetokens = tokendefs[statestack[-1]]
                    break
            else:
                try:
                    if text[pos] == '\n':
                        pos += 1
                        statestack = ['root']
                        statetokens = tokendefs['root']
                        yield pos, tokens.Text, u'\n'
                        continue
                    yield pos, tokens.Error, text[pos]
                    pos += 1
                except IndexError:
                    break

def tokenize(sql, encoding=None):
    lexer = Lexer()
    if encoding is not None:
        lexer.encoding = encoding
    return lexer.get_tokens(sql)