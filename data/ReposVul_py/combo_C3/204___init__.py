class VM:
    def __init__(self):
        self.stack = []
        self.memory = {}
        self.pc = 0
        self.instructions = []

    def load_program(self, program):
        self.instructions = program

    def run(self):
        dispatch = {
            'PUSH': self.push,
            'POP': self.pop,
            'LOAD': self.load,
            'STORE': self.store,
            'JMP': self.jmp,
            'JZ': self.jz,
            'CALL': self.call,
            'RET': self.ret,
        }
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            dispatch[op](*args)
            self.pc += 1

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def load(self, var_name):
        self.stack.append(self.memory[var_name])

    def store(self, var_name):
        self.memory[var_name] = self.stack.pop()

    def jmp(self, address):
        self.pc = address - 1

    def jz(self, address):
        if self.pop() == 0:
            self.pc = address - 1

    def call(self, address):
        self.stack.append(self.pc)
        self.pc = address - 1

    def ret(self):
        self.pc = self.pop()

vm = VM()
program = [
    ('PUSH', 'contents'),
    ('STORE', 'var1'),
    ('PUSH', 'where'),
    ('STORE', 'var2'),
    ('PUSH', "2023.07.22"),
    ('STORE', 'version'),
    ('LOAD', 'var1'),
    ('LOAD', 'var2'),
    ('LOAD', 'version'),
]

vm.load_program(program)
vm.run()
__all__ = [vm.memory['var1'], vm.memory['var2']]
__version__ = vm.memory['version']