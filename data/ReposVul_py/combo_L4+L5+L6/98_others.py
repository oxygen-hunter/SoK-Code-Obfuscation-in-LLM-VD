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

        def process_recursion(tidx, token):
            if not token:
                return
            pidx, prev_ = tlist.token_prev(tidx, skip_ws=False)
            nidx, next_ = tlist.token_next(tidx, skip_ws=False)
            if (prev_ is None or next_ is None
                    or prev_.is_whitespace or prev_.match(T.Punctuation, '(')
                    or next_.is_whitespace or next_.match(T.Punctuation, ')')):
                if prev_ is not None and not prev_.match(T.Punctuation, '('):
                    tlist.tokens.insert(tidx, _get_insert_token(token))
                tlist.tokens.remove(token)
            else:
                tlist.tokens[tidx] = _get_insert_token(token)
            tidx, token = get_next_comment()
            process_recursion(tidx, token)

        tidx, token = get_next_comment()
        process_recursion(tidx, token)

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
        def process_recursion(idx, token, last_was_ws, is_first_char):
            if idx >= len(tlist.tokens):
                return
            token = tlist.tokens[idx]
            if token.is_whitespace:
                token.value = '' if last_was_ws or is_first_char else ' '
            last_was_ws = token.is_whitespace
            is_first_char = False
            process_recursion(idx+1, token, last_was_ws, is_first_char)

        process_recursion(0, None, False, True)

    def _stripws_identifierlist(self, tlist):
        last_nl = None
        def process_recursion(idx):
            if idx >= len(tlist.tokens):
                return
            token = tlist.tokens[idx]
            if last_nl and token.ttype is T.Punctuation and token.value == ',':
                tlist.tokens.remove(last_nl)
            last_nl = token if token.is_whitespace else None
            process_recursion(idx+1)

        process_recursion(0)
        return self._stripws_default(tlist)

    def _stripws_parenthesis(self, tlist):
        def process_recursion(condition, index, method, default_method):
            if not condition:
                return
            method(index)
            process_recursion(condition, index, method, default_method)

        process_recursion(tlist.tokens[1].is_whitespace, 1, tlist.tokens.pop, self._stripws_default)
        process_recursion(tlist.tokens[-2].is_whitespace, -2, tlist.tokens.pop, self._stripws_default)

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

        def process_recursion(tidx, token):
            if not token:
                return
            nidx, next_ = tlist.token_next(tidx, skip_ws=False)
            if next_ and next_.ttype != T.Whitespace:
                tlist.insert_after(tidx, sql.Token(T.Whitespace, ' '))

            pidx, prev_ = tlist.token_prev(tidx, skip_ws=False)
            if prev_ and prev_.ttype != T.Whitespace:
                tlist.insert_before(tidx, sql.Token(T.Whitespace, ' '))
                tidx += 1

            tidx, token = tlist.token_next_by(t=ttypes, idx=tidx)
            process_recursion(tidx, token)

        tidx, token = tlist.token_next_by(t=ttypes)
        process_recursion(tidx, token)

    def process(self, stmt):
        [self.process(sgroup) for sgroup in stmt.get_sublists()]
        SpacesAroundOperatorsFilter._process(stmt)
        return stmt


class SerializerUnicode:
    @staticmethod
    def process(stmt):
        lines = split_unquoted_newlines(stmt)
        return '\n'.join(line.rstrip() for line in lines)