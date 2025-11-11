import re
from sqlparse import sql, tokens as T
from sqlparse.utils import split_unquoted_newlines

# Define VM instructions
PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT = range(11)

# VM class
class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.code = []
        self.running = True

    def run(self):
        while self.running:
            instr = self.code[self.pc]
            self.execute(instr)

    def execute(self, instr):
        opcode, *args = instr
        if opcode == PUSH:
            self.stack.append(args[0])
        elif opcode == POP:
            self.stack.pop()
        elif opcode == ADD:
            b, a = self.stack.pop(), self.stack.pop()
            self.stack.append(a + b)
        elif opcode == SUB:
            b, a = self.stack.pop(), self.stack.pop()
            self.stack.append(a - b)
        elif opcode == JMP:
            self.pc = args[0]
            return
        elif opcode == JZ:
            if self.stack.pop() == 0:
                self.pc = args[0]
                return
        elif opcode == LOAD:
            self.stack.append(args[0])
        elif opcode == STORE:
            self.stack[-1] = args[0]
        elif opcode == CALL:
            self.stack.append(self.pc + 1)
            self.pc = args[0]
            return
        elif opcode == RET:
            self.pc = self.stack.pop()
            return
        elif opcode == HALT:
            self.running = False
        self.pc += 1

# Compile logic into bytecode
def compile_strip_comments():
    vm = VM()
    vm.code = [
        (LOAD, "tlist"), 
        (CALL, 10), 
        (HALT,)
    ]
    vm.code += [
        (LOAD, "getitem"), 
        (CALL, 20), 
        (RET,),
        (LOAD, "tlist"), 
        (LOAD, "token_next_by"), 
        (CALL, 30), 
        (STORE, "tidx"), 
        (STORE, "token"), 
        (LOAD, "token"), 
        (JZ, 9), 
        (CALL, 40), 
        (JMP, 11),
        (RET,),
        (LOAD, "insert_whitespace"), 
        (RET,),
    ]
    return vm

def compile_strip_whitespace():
    vm = VM()
    vm.code = [
        (LOAD, "tlist"), 
        (CALL, 10), 
        (HALT,)
    ]
    vm.code += [
        (LOAD, "tokens"), 
        (LOAD, "is_whitespace"), 
        (CALL, 20), 
        (RET,)
    ]
    return vm

def compile_spaces_around_operators():
    vm = VM()
    vm.code = [
        (LOAD, "tlist"), 
        (CALL, 10), 
        (HALT,)
    ]
    vm.code += [
        (LOAD, "operations"), 
        (LOAD, "operators"), 
        (CALL, 20), 
        (RET,)
    ]
    return vm

class StripCommentsFilter:
    def process(self, stmt):
        vm = compile_strip_comments()
        vm.run()
        return stmt

class StripWhitespaceFilter:
    def process(self, stmt, depth=0):
        vm = compile_strip_whitespace()
        vm.run()
        return stmt

class SpacesAroundOperatorsFilter:
    def process(self, stmt):
        vm = compile_spaces_around_operators()
        vm.run()
        return stmt

class SerializerUnicode:
    @staticmethod
    def process(stmt):
        lines = split_unquoted_newlines(stmt)
        return '\n'.join(line.rstrip() for line in lines)