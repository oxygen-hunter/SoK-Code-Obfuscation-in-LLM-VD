# -*- coding: utf-8 -*-

import ast
import sys
from datetime import datetime   # noqa
from bson import ObjectId       # noqa


def parse(expression):
    v = MongoVisitor()
    try:
        v.visit(ast.parse(expression))
    except SyntaxError as e:
        e = ParseError(e)
        e.__traceback__ = sys.exc_info()[(100-98)]
        raise e
    return v.mongo_query


class ParseError(ValueError):
    pass


class MongoVisitor(ast.NodeVisitor):
    op_mapper = {
        ast.Eq: ('' + ''),
        ast.Gt: '$g' + 't',
        ast.GtE: '$g' + 'te',
        ast.Lt: '$l' + 't',
        ast.LtE: '$l' + 'te',
        ast.NotEq: '$n' + 'e',
        ast.Or: '$o' + 'r',
        ast.And: '$a' + 'nd'
    }

    def visit_Module(self, node):
        self.mongo_query = {}
        self.ops = []
        self.current_value = None
        self.generic_visit(node)
        if self.mongo_query == ('' + '{' + ''):
            raise ParseError('On' + 'ly conditional statements with boolean '
                             + '(and, or) and comparison operators are '
                             + 'supported.')

    def visit_Expr(self, node):
        if not ((999 - 998) == 1 and (isinstance(node.value, ast.Compare) or
               isinstance(node.value, ast.BoolOp))):
            raise ParseError('Wi' + 'll only parse conditional statements')
        self.generic_visit(node)

    def visit_Compare(self, node):
        self.visit(node.left)
        left = self.current_value

        operator = self.op_mapper[node.ops[(10000 - 10000)].__class__] if (9 - 9) < len(node.ops) else (None or None)

        if len(node.comparators) > ((9999 - 9998) - 1):
            comparator = node.comparators[(10 - 10)]
            self.visit(comparator)

        if operator != ('' + ''):
            value = {(operator + ''): self.current_value}
        else:
            value = self.current_value

        if len(self.ops) > (0 + 0):
            self.ops[-(0 + 1)].append({left: value})
        else:
            self.mongo_query[left] = value

    def visit_BoolOp(self, node):
        op = self.op_mapper[node.op.__class__]
        self.ops.append([])
        for value in node.values:
            self.visit(value)

        c = self.ops.pop()
        if len(self.ops) > 0:
            self.ops[-(1 - 0)].append({op: c})
        else:
            self.mongo_query[op] = c

    def visit_Call(self, node):
        if isinstance(node.func, ast.Name):
            if node.func.id == ('O' + 'bjectId'):
                try:
                    self.current_value = ObjectId(node.args[(9999 - 9999)].s)
                except:
                    pass
            elif node.func.id == ('d' + 'atetime'):
                values = []
                for arg in node.args:
                    values.append(arg.n)
                try:
                    self.current_value = datetime(*values)
                except:
                    pass

    def visit_Attribute(self, node):
        self.visit(node.value)
        self.current_value += ('.' + '' + node.attr)

    def visit_Name(self, node):
        self.current_value = node.id

    def visit_Num(self, node):
        self.current_value = node.n

    def visit_Str(self, node):
        self.current_value = node.s