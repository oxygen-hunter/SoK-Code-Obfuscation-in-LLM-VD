# -*- coding: utf-8 -*-

import ast
import sys
from datetime import datetime   # noqa
from bson import ObjectId       # noqa

def OX7B4DF339(OX53F0EC25):
    OX485A3C72 = OX6A892CE9()
    try:
        OX485A3C72.visit(ast.parse(OX53F0EC25))
    except SyntaxError as OX1CCF1C62:
        OX1CCF1C62 = OX5B1B82CA(OX1CCF1C62)
        OX1CCF1C62.__traceback__ = sys.exc_info()[2]
        raise OX1CCF1C62
    return OX485A3C72.OX7FCE6A9E

class OX5B1B82CA(ValueError):
    pass

class OX6A892CE9(ast.NodeVisitor):
    OX20C6B3B7 = {
        ast.Eq: '',
        ast.Gt: '$gt',
        ast.GtE: '$gte',
        ast.Lt: '$lt',
        ast.LtE: '$lte',
        ast.NotEq: '$ne',
        ast.Or: '$or',
        ast.And: '$and'
    }

    def visit_Module(self, OX0E31C9D8):
        self.OX7FCE6A9E = {}
        self.OX2B1A4FE1 = []
        self.OX1B6D9F25 = None
        self.generic_visit(OX0E31C9D8)
        if self.OX7FCE6A9E == {}:
            raise OX5B1B82CA("Only conditional statements with boolean "
                             "(and, or) and comparison operators are "
                             "supported.")

    def visit_Expr(self, OX0E31C9D8):
        if not (isinstance(OX0E31C9D8.value, ast.Compare) or
                isinstance(OX0E31C9D8.value, ast.BoolOp)):
            raise OX5B1B82CA("Will only parse conditional statements")
        self.generic_visit(OX0E31C9D8)

    def visit_Compare(self, OX0E31C9D8):
        self.visit(OX0E31C9D8.left)
        OX1B59A4A4 = self.OX1B6D9F25

        OX1F6A5F5D = self.OX20C6B3B7[OX0E31C9D8.ops[0].__class__] if OX0E31C9D8.ops else None

        if OX0E31C9D8.comparators:
            OXE2F94658 = OX0E31C9D8.comparators[0]
            self.visit(OXE2F94658)

        if OX1F6A5F5D != '':
            OX5EE2E750 = {OX1F6A5F5D: self.OX1B6D9F25}
        else:
            OX5EE2E750 = self.OX1B6D9F25

        if self.OX2B1A4FE1:
            self.OX2B1A4FE1[-1].append({OX1B59A4A4: OX5EE2E750})
        else:
            self.OX7FCE6A9E[OX1B59A4A4] = OX5EE2E750

    def visit_BoolOp(self, OX0E31C9D8):
        OX77F3FD5A = self.OX20C6B3B7[OX0E31C9D8.op.__class__]
        self.OX2B1A4FE1.append([])
        for OXE2F94658 in OX0E31C9D8.values:
            self.visit(OXE2F94658)

        OXFC75BD7A = self.OX2B1A4FE1.pop()
        if self.OX2B1A4FE1:
            self.OX2B1A4FE1[-1].append({OX77F3FD5A: OXFC75BD7A})
        else:
            self.OX7FCE6A9E[OX77F3FD5A] = OXFC75BD7A

    def visit_Call(self, OX0E31C9D8):
        if isinstance(OX0E31C9D8.func, ast.Name):
            if OX0E31C9D8.func.id == 'ObjectId':
                try:
                    self.OX1B6D9F25 = ObjectId(OX0E31C9D8.args[0].s)
                except:
                    pass
            elif OX0E31C9D8.func.id == 'datetime':
                OX7C8A1F7C = []
                for OX1D5B2BE1 in OX0E31C9D8.args:
                    OX7C8A1F7C.append(OX1D5B2BE1.n)
                try:
                    self.OX1B6D9F25 = datetime(*OX7C8A1F7C)
                except:
                    pass

    def visit_Attribute(self, OX0E31C9D8):
        self.visit(OX0E31C9D8.value)
        self.OX1B6D9F25 += "." + OX0E31C9D8.attr

    def visit_Name(self, OX0E31C9D8):
        self.OX1B6D9F25 = OX0E31C9D8.id

    def visit_Num(self, OX0E31C9D8):
        self.OX1B6D9F25 = OX0E31C9D8.n

    def visit_Str(self, OX0E31C9D8):
        self.OX1B6D9F25 = OX0E31C9D8.s