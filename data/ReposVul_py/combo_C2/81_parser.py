# -*- coding: utf-8 -*-

import ast
import sys
from datetime import datetime   # noqa
from bson import ObjectId       # noqa

def parse(expression):
    v = MongoVisitor()
    dispatcher = 0
    while True:
        if dispatcher == 0:
            try:
                dispatcher = 1
            except SyntaxError as e:
                e = ParseError(e)
                e.__traceback__ = sys.exc_info()[2]
                raise e
        elif dispatcher == 1:
            v.visit(ast.parse(expression))
            dispatcher = 2
        elif dispatcher == 2:
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

        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.generic_visit(node)
                dispatcher = 1
            elif dispatcher == 1:
                if self.mongo_query == {}:
                    dispatcher = 2
                else:
                    break
            elif dispatcher == 2:
                raise ParseError("Only conditional statements with boolean "
                                 "(and, or) and comparison operators are "
                                 "supported.")

    def visit_Expr(self, node):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if not (isinstance(node.value, ast.Compare) or
                        isinstance(node.value, ast.BoolOp)):
                    dispatcher = 1
                else:
                    dispatcher = 2
            elif dispatcher == 1:
                raise ParseError("Will only parse conditional statements")
            elif dispatcher == 2:
                self.generic_visit(node)
                break

    def visit_Compare(self, node):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.visit(node.left)
                left = self.current_value
                operator = self.op_mapper[node.ops[0].__class__] if node.ops else None
                if node.comparators:
                    comparator = node.comparators[0]
                    self.visit(comparator)
                dispatcher = 1
            elif dispatcher == 1:
                if operator != '':
                    value = {operator: self.current_value}
                else:
                    value = self.current_value
                if self.ops:
                    self.ops[-1].append({left: value})
                else:
                    self.mongo_query[left] = value
                break

    def visit_BoolOp(self, node):
        op = self.op_mapper[node.op.__class__]
        self.ops.append([])
        dispatcher = 0
        while True:
            if dispatcher == 0:
                for value in node.values:
                    self.visit(value)
                dispatcher = 1
            elif dispatcher == 1:
                c = self.ops.pop()
                if self.ops:
                    self.ops[-1].append({op: c})
                else:
                    self.mongo_query[op] = c
                break

    def visit_Call(self, node):
        if isinstance(node.func, ast.Name):
            dispatcher = 0
            while True:
                if dispatcher == 0:
                    if node.func.id == 'ObjectId':
                        dispatcher = 1
                    elif node.func.id == 'datetime':
                        dispatcher = 2
                    else:
                        break
                elif dispatcher == 1:
                    try:
                        self.current_value = ObjectId(node.args[0].s)
                    except:
                        pass
                    break
                elif dispatcher == 2:
                    values = []
                    for arg in node.args:
                        values.append(arg.n)
                    try:
                        self.current_value = datetime(*values)
                    except:
                        pass
                    break

    def visit_Attribute(self, node):
        self.visit(node.value)
        self.current_value += "." + node.attr

    def visit_Name(self, node):
        self.current_value = node.id

    def visit_Num(self, node):
        self.current_value = node.n

    def visit_Str(self, node):
        self.current_value = node.s