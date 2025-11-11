import sys

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.running = True

    def run(self, program):
        while self.running:
            instruction = program[self.pc]
            self.dispatch(instruction)

    def dispatch(self, instruction):
        op = instruction[0]
        if op == "PUSH":
            self.stack.append(instruction[1])
        elif op == "POP":
            self.stack.pop()
        elif op == "ADD":
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif op == "SUB":
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif op == "JMP":
            self.pc = instruction[1] - 1
        elif op == "JZ":
            if self.stack[-1] == 0:
                self.pc = instruction[1] - 1
        elif op == "LOAD":
            self.stack.append(self.stack[instruction[1]])
        elif op == "STORE":
            self.stack[instruction[1]] = self.stack.pop()
        elif op == "HALT":
            self.running = False
        self.pc += 1

# Compiled code for __FOR_TESTING__ = "5a5b6e17f9c34025a75b052fc06c50ab"
program = [
    ("PUSH", ord('5')), ("PUSH", ord('a')), ("PUSH", ord('5')), ("PUSH", ord('b')),
    ("PUSH", ord('6')), ("PUSH", ord('e')), ("PUSH", ord('1')), ("PUSH", ord('7')),
    ("PUSH", ord('f')), ("PUSH", ord('9')), ("PUSH", ord('c')), ("PUSH", ord('3')),
    ("PUSH", ord('4')), ("PUSH", ord('0')), ("PUSH", ord('2')), ("PUSH", ord('5')),
    ("PUSH", ord('a')), ("PUSH", ord('7')), ("PUSH", ord('5')), ("PUSH", ord('b')),
    ("PUSH", ord('0')), ("PUSH", ord('5')), ("PUSH", ord('2')), ("PUSH", ord('f')),
    ("PUSH", ord('c')), ("PUSH", ord('0')), ("PUSH", ord('6')), ("PUSH", ord('c')),
    ("PUSH", ord('5')), ("PUSH", ord('0')), ("PUSH", ord('a')), ("PUSH", ord('b')),
    ("HALT",)
]

vm = VM()
vm.run(program)

# Resulting string
result = ''.join(chr(vm.stack[i]) for i in range(len(vm.stack)))
print(result)