# -*- coding: utf-8 -*-

import ast
import sys
from datetime import datetime   # noqa
from bson import ObjectId       # noqa

def getMongoVisitor():
    return MongoVisitor()

def getParseError(e):
    return ParseError(e)

def getSysExcInfo():
    return sys.exc_info()[2]

def getOpMapper():
    return {
        ast.Eq: '',
        ast.Gt: '$gt',
        ast.GtE: '$gte',
        ast.Lt: '$lt',
        ast.LtE: '$lte',
        ast.NotEq: '$ne',
        ast.Or: '$or',
        ast.And: '$and'
    }

def parse(expression):
    v = getMongoVisitor()
    try:
        v.visit(ast.parse(expression))
    except SyntaxError as e:
        e = getParseError(e)
        e.__traceback__ = getSysExcInfo()
        raise e
    return v.mongo_query

class ParseError(ValueError):
    pass

class MongoVisitor(ast.NodeVisitor):
    def getEmptyDict(self):
        return {}

    def getEmptyList(self):
        return []

    def visit_Module(self, node):
        self.mongo_query = self.getEmptyDict()
        self.ops = self.getEmptyList()
        self.current_value = None
        self.generic_visit(node)
        if self.mongo_query == self.getEmptyDict():
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
        left = self.current_value
        operator = getOpMapper()[node.ops[0].__class__] if node.ops else None
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
        op = getOpMapper()[node.op.__class__]
        self.ops.append(self.getEmptyList())
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
                values = self.getEmptyList()
                for arg in node.args:
                    values.append(arg.n)
                try:
                    self.current_value = datetime(*values)
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