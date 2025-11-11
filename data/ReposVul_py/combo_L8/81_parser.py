# -*- coding: utf-8 -*-

import ast
import sys
from datetime import datetime   # noqa
from bson import ObjectId       # noqa
from ctypes import *

# C code as a string
c_code = """
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *message;
} ParseError;

ParseError* create_parse_error(const char *msg) {
    ParseError *error = (ParseError *)malloc(sizeof(ParseError));
    error->message = strdup(msg);
    return error;
}

void raise_parse_error(ParseError *error) {
    fprintf(stderr, "%s\\n", error->message);
    free(error->message);
    free(error);
    exit(1);
}
"""

# Save the C code to a temporary file
with open("parse_error.c", "w") as f:
    f.write(c_code)

# Compile the C code into a shared library
import os
os.system("gcc -shared -o parse_error.so -fPIC parse_error.c")

# Load the shared library
c_lib = CDLL("./parse_error.so")

class ParseError(ValueError):
    pass

def parse(expression):
    v = MongoVisitor()
    try:
        v.visit(ast.parse(expression))
    except SyntaxError as e:
        c_error = c_lib.create_parse_error(b"SyntaxError occurred")
        c_lib.raise_parse_error(c_error)
    return v.mongo_query

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
            c_error = c_lib.create_parse_error(b"Unsupported Python expression")
            c_lib.raise_parse_error(c_error)

    def visit_Expr(self, node):
        if not (isinstance(node.value, ast.Compare) or
                isinstance(node.value, ast.BoolOp)):
            c_error = c_lib.create_parse_error(b"Only parsing conditional statements")
            c_lib.raise_parse_error(c_error)
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
        self.visit(node.value)
        self.current_value += "." + node.attr

    def visit_Name(self, node):
        self.current_value = node.id

    def visit_Num(self, node):
        self.current_value = node.n

    def visit_Str(self, node):
        self.current_value = node.s