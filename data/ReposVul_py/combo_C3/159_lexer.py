```python
import re
import sys
from sqlparse import tokens
from sqlparse.keywords import KEYWORDS, KEYWORDS_COMMON
from cStringIO import StringIO

# VM Instruction Set
PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET = range(10)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.memory = {}
        self.running = True

    def execute(self, bytecode):
        while self.running:
            instr = bytecode[self.pc]
            self.pc += 1
            if instr[0] == PUSH:
                self.stack.append(instr[1])
            elif instr[0] == POP:
                self.stack.pop()
            elif instr[0] == ADD:
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif instr[0] == SUB:
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif instr[0] == JMP:
                self.pc = instr[1]
            elif instr[0] == JZ:
                if self.stack.pop() == 0:
                    self.pc = instr[1]
            elif instr[0] == LOAD:
                self.stack.append(self.memory[instr[1]])
            elif instr[0] == STORE:
                self.memory[instr[1]] = self.stack.pop()
            elif instr[0] == CALL:
                self.stack.append(self.pc)
                self.pc = instr[1]
            elif instr[0] == RET:
                self.pc = self.stack.pop()
            else:
                self.running = False

def is_keyword_vm(value):
    vm = VM()
    bytecode = [
        (PUSH, value.upper()),
        (CALL, 5),
        (JMP, 10),
        (LOAD, 'KEYWORDS_COMMON'),
        (LOAD, 'KEYWORDS'),
        (RET,)
    ]
    vm.memory['KEYWORDS_COMMON'] = KEYWORDS_COMMON