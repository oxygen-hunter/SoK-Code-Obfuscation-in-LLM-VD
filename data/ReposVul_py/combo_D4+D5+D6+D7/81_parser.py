# -*- coding: utf-8 -*-

import ast
import sys
from datetime import datetime
from bson import ObjectId

def parse(expr):
    visitor = MongoVisitor()
    try:
        visitor.visit(ast.parse(expr))
    except SyntaxError as err:
        err = ParseError(err)
        err.__traceback__ = sys.exc_info()[2]
        raise err
    return visitor.mongo_query

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
        query_info = {'mongo_query': {}, 'ops': [], 'current_value': None}
        self.mongo_query = query_info['mongo_query']
        self.ops = query_info['ops']
        self.current_value = query_info['current_value']

        self.generic_visit(node)

        if self.mongo_query == {}:
            raise ParseError("Only conditional statements with boolean "
                             "(and, or) and comparison operators are "
                             "supported.")

    def visit_Expr(self, node):
        if not (isinstance(node.value, ast.Compare) or
                isinstance(node.value, ast.BoolOp)):
            raise ParseError("Will only parse conditional statements")
        self.generic_visit(node)

    def visit_Compare(self, node):
        self.visit(node.left)
        left_side = self.current_value

        oper = self.op_mapper[node.ops[0].__class__] if node.ops else None

        if node.comparators:
            comp = node.comparators[0]
            self.visit(comp)

        if oper != '':
            val = {oper: self.current_value}
        else:
            val = self.current_value

        if self.ops:
            self.ops[-1].append({left_side: val})
        else:
            self.mongo_query[left_side] = val

    def visit_BoolOp(self, node):
        op = self.op_mapper[node.op.__class__]
        self.ops.append([])
        for val in node.values:
            self.visit(val)

        op_list = self.ops.pop()
        if self.ops:
            self.ops[-1].append({op: op_list})
        else:
            self.mongo_query[op] = op_list

    def visit_Call(self, node):
        if isinstance(node.func, ast.Name):
            if node.func.id == 'ObjectId':
                try:
                    self.current_value = ObjectId(node.args[0].s)
                except:
                    pass
            elif node.func.id == 'datetime':
                date_args = []
                for arg in node.args:
                    date_args.append(arg.n)
                try:
                    self.current_value = datetime(*date_args)
                except:
                    pass

    def visit_Attribute(self, node):
        self.visit(node.value)
        self.current_value += "." + node.attr

    def visit_Name(self, node):
        self.current_value = node.id

    def visit_Num(self, node):
        self.current_value = node.n

    def visit_Str(self, node):
        self.current_value = node.s