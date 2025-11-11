import ast
import sys
from datetime import datetime
from bson import ObjectId

def parse(expression):
    program = compile_to_bytecode(expression)
    vm = VirtualMachine()
    try:
        vm.run(program)
    except SyntaxError as e:
        e = ParseError(e)
        e.__traceback__ = sys.exc_info()[2]
        raise e
    return vm.get_query()

class ParseError(ValueError):
    pass

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.query = {}
        self.program_counter = 0

    def get_query(self):
        return self.query

    def run(self, program):
        while self.program_counter < len(program):
            instruction, *args = program[self.program_counter]
            getattr(self, 'op_' + instruction)(*args)
            self.program_counter += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_STORE(self, index):
        self.query[index] = self.stack.pop()

    def op_LOAD(self, index):
        self.stack.append(self.query.get(index))

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_GT(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append({'$gt': b})

    def op_LT(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append({'$lt': b})

    def op_EQ(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append({'$eq': b})

    def op_NE(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append({'$ne': b})

    def op_GTE(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append({'$gte': b})

    def op_LTE(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append({'$lte': b})

    def op_AND(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append({'$and': [a, b]})

    def op_OR(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append({'$or': [a, b]})

    def op_LOAD_ATTR(self, attr):
        value = self.stack.pop()
        self.stack.append(value + '.' + attr)

def compile_to_bytecode(expression):
    tree = ast.parse(expression)
    compiler = BytecodeCompiler()
    compiler.visit(tree)
    return compiler.bytecode

class BytecodeCompiler(ast.NodeVisitor):
    def __init__(self):
        self.bytecode = []

    def visit_Module(self, node):
        for stmt in node.body:
            self.visit(stmt)

    def visit_Expr(self, node):
        self.visit(node.value)

    def visit_Compare(self, node):
        self.visit(node.left)
        left = self.bytecode.pop()
        self.visit(node.comparators[0])
        right = self.bytecode.pop()
        operator = node.ops[0]
        self.bytecode.append(('PUSH', left))
        self.bytecode.append(('PUSH', right))
        self.bytecode.append(('OP_' + type(operator).__name__.upper(),))

    def visit_BoolOp(self, node):
        for value in node.values:
            self.visit(value)
        op = type(node.op).__name__.upper()
        self.bytecode.append(('OP_' + op,))

    def visit_Name(self, node):
        self.bytecode.append(('PUSH', node.id))

    def visit_Num(self, node):
        self.bytecode.append(('PUSH', node.n))

    def visit_Str(self, node):
        self.bytecode.append(('PUSH', node.s))

    def visit_Attribute(self, node):
        self.visit(node.value)
        self.bytecode.append(('LOAD_ATTR', node.attr))

    def visit_Call(self, node):
        if isinstance(node.func, ast.Name):
            if node.func.id == 'ObjectId':
                arg = node.args[0].s
                self.bytecode.append(('PUSH', ObjectId(arg)))
            elif node.func.id == 'datetime':
                args = [arg.n for arg in node.args]
                self.bytecode.append(('PUSH', datetime(*args)))