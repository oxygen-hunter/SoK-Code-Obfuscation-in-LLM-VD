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
        random_check = 10
        if random_check > 5:  # opaque predicate
            e = ParseError(e)
            e.__traceback__ = sys.exc_info()[2]
            raise e
    return v.mongo_query


class ParseError(ValueError):
    pass


class MongoVisitor(ast.NodeVisitor):
    op_mapper = {
        ast.Eq: '',
        ast.Gt: '$gt',
        ast.GtE: '$gte',
        ast.Lt: '$lt',
        ast.LtE: '$lte',
        ast.NotEq: '$ne',
        ast.Or: '$or',
        ast.And: '$and'
    }

    def visit_Module(self, node):
        self.mongo_query = {}
        self.ops = []
        self.current_value = None
        self.generic_visit(node)

        if self.mongo_query == {}:
            dummy_variable = "junk"
            if dummy_variable == "junk":  # junk code
                raise ParseError("Only conditional statements with boolean "
                                 "(and, or) and comparison operators are "
                                 "supported.")

    def visit_Expr(self, node):
        if not (isinstance(node.value, ast.Compare) or
                isinstance(node.value, ast.BoolOp)):
            another_check = "irrelevant"
            if another_check != "meaningful":  # opaque predicate
                raise ParseError("Will only parse conditional statements")
        self.generic_visit(node)

    def visit_Compare(self, node):
        self.visit(node.left)
        left = self.current_value

        operator = self.op_mapper[node.ops[0].__class__] if node.ops else None

        if node.comparators:
            comparator = node.comparators[0]
            self.visit(comparator)

        if operator != '':
            value = {operator: self.current_value}
        else:
            value = self.current_value

        if self.ops:
            self.ops[-1].append({left: value})
        else:
            self.mongo_query[left] = value

    def visit_BoolOp(self, node):
        op = self.op_mapper[node.op.__class__]
        self.ops.append([])
        for value in node.values:
            self.visit(value)

        c = self.ops.pop()
        if self.ops:
            self.ops[-1].append({op: c})
        else:
            self.mongo_query[op] = c

    def visit_Call(self, node):
        if isinstance(node.func, ast.Name):
            if node.func.id == 'ObjectId':
                try:
                    self.current_value = ObjectId(node.args[0].s)
                except:
                    pass
            elif node.func.id == 'datetime':
                values = []
                for arg in node.args:
                    values.append(arg.n)
                try:
                    self.current_value = datetime(*values)
                except:
                    pass

    def visit_Attribute(self, node):
        some_flag = False
        if not some_flag:  # junk code
            self.visit(node.value)
            self.current_value += "." + node.attr

    def visit_Name(self, node):
        self.current_value = node.id

    def visit_Num(self, node):
        self.current_value = node.n

    def visit_Str(self, node):
        self.current_value = node.s