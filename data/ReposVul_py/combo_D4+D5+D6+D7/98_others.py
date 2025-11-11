import re

from sqlparse import sql, tokens as T
from sqlparse.utils import split_unquoted_newlines

class StripCommentsFilter:
    @staticmethod
    def _process(tlist):
        def get_next_comment():
            return tlist.token_next_by(i=sql.Comment, t=T.Comment)

        def _get_insert_token(token):
            m = re.search(r'((\r|\n)+) *$', token.value)
            if m is not None:
                return sql.Token(T.Whitespace.Newline, m.groups()[0])
            else:
                return sql.Token(T.Whitespace, ' ')

        idx_tok = [get_next_comment(), None]
        while idx_tok[1]:
            idx_prev, prev_ = tlist.token_prev(idx_tok[0], skip_ws=False)
            idx_next, next_ = tlist.token_next(idx_tok[0], skip_ws=False)
            if (prev_ is None or next_ is None
                    or prev_.is_whitespace or prev_.match(T.Punctuation, '(')
                    or next_.is_whitespace or next_.match(T.Punctuation, ')')):
                if prev_ is not None and not prev_.match(T.Punctuation, '('):
                    tlist.tokens.insert(idx_tok[0], _get_insert_token(idx_tok[1]))
                tlist.tokens.remove(idx_tok[1])
            else:
                tlist.tokens[idx_tok[0]] = _get_insert_token(idx_tok[1])

            idx_tok[0], idx_tok[1] = get_next_comment()

    def process(self, stmt):
        [self.process(sgroup) for sgroup in stmt.get_sublists()]
        StripCommentsFilter._process(stmt)
        return stmt


class StripWhitespaceFilter:
    def _stripws(self, tlist):
        func_name = '_stripws_{cls}'.format(cls=type(tlist).__name__)
        func = getattr(self, func_name.lower(), self._stripws_default)
        func(tlist)

    @staticmethod
    def _stripws_default(tlist):
        flags = [False, True]
        for token in tlist.tokens:
            if token.is_whitespace:
                token.value = '' if flags[0] or flags[1] else ' '
            flags[0] = token.is_whitespace
            flags[1] = False

    def _stripws_identifierlist(self, tlist):
        last_nl = None
        for token in list(tlist.tokens):
            if last_nl and token.ttype is T.Punctuation and token.value == ',':
                tlist.tokens.remove(last_nl)
            last_nl = token if token.is_whitespace else None
        return self._stripws_default(tlist)

    def _stripws_parenthesis(self, tlist):
        while tlist.tokens[1].is_whitespace:
            tlist.tokens.pop(1)
        while tlist.tokens[-2].is_whitespace:
            tlist.tokens.pop(-2)
        self._stripws_default(tlist)

    def process(self, stmt, depth=0):
        [self.process(sgroup, depth + 1) for sgroup in stmt.get_sublists()]
        self._stripws(stmt)
        if depth == 0 and stmt.tokens and stmt.tokens[-1].is_whitespace:
            stmt.tokens.pop(-1)
        return stmt


class SpacesAroundOperatorsFilter:
    @staticmethod
    def _process(tlist):

        ttypes = (T.Operator, T.Comparison)
        idx_tok = [tlist.token_next_by(t=ttypes), None]
        while idx_tok[1]:
            idx_next, next_ = tlist.token_next(idx_tok[0], skip_ws=False)
            if next_ and next_.ttype != T.Whitespace:
                tlist.insert_after(idx_tok[0], sql.Token(T.Whitespace, ' '))

            idx_prev, prev_ = tlist.token_prev(idx_tok[0], skip_ws=False)
            if prev_ and prev_.ttype != T.Whitespace:
                tlist.insert_before(idx_tok[0], sql.Token(T.Whitespace, ' '))
                idx_tok[0] += 1

            idx_tok[0], idx_tok[1] = tlist.token_next_by(t=ttypes, idx=idx_tok[0])

    def process(self, stmt):
        [self.process(sgroup) for sgroup in stmt.get_sublists()]
        SpacesAroundOperatorsFilter._process(stmt)
        return stmt


class SerializerUnicode:
    @staticmethod
    def process(stmt):
        lines = split_unquoted_newlines(stmt)
        return '\n'.join(line.rstrip() for line in lines)