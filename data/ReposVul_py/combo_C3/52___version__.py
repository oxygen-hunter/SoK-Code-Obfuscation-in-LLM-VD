class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.bytecode = []
        self.registers = {}

    def load_program(self, bytecode):
        self.bytecode = bytecode

    def run(self):
        while self.pc < len(self.bytecode):
            op = self.bytecode[self.pc]
            self.pc += 1
            if op == "PUSH":
                value = self.bytecode[self.pc]
                self.pc += 1
                self.stack.append(value)
            elif op == "POP":
                self.stack.pop()
            elif op == "ADD":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif op == "SUB":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif op == "JMP":
                target = self.bytecode[self.pc]
                self.pc = target
            elif op == "JZ":
                target = self.bytecode[self.pc]
                self.pc += 1
                if self.stack.pop() == 0:
                    self.pc = target
            elif op == "LOAD":
                reg = self.bytecode[self.pc]
                self.pc += 1
                self.stack.append(self.registers.get(reg, 0))
            elif op == "STORE":
                reg = self.bytecode[self.pc]
                self.pc += 1
                self.registers[reg] = self.stack.pop()

def compile_version_to_vm():
    bytecode = [
        "PUSH", 0, 
        "PUSH", 9, 
        "PUSH", ord('9'),
        "STORE", "VERSION",
        "LOAD", "VERSION",
        "PUSH", ord('.'),
        "ADD",
        "PUSH", ord('.'),
        "ADD",
        "STORE", "VERSION_STR",
        "LOAD", "VERSION_STR",
    ]
    return bytecode

vm = VM()
vm.load_program(compile_version_to_vm())
vm.run()
VERSION_STR = chr(vm.stack.pop())

def version():
    return VERSION_STR