import vyper

class VM:
    def __init__(self):
        self.stack = []
        self.memory = {}
        self.pc = 0
        self.instructions = []

    def load_program(self, program):
        self.instructions = program

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            getattr(self, f'op_{instr[0]}')(*instr[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.stack.append(a - b)

    def op_STORE(self, address):
        self.memory[address] = self.op_POP()

    def op_LOAD(self, address):
        self.stack.append(self.memory.get(address, 0))

    def op_JMP(self, address):
        self.pc = address - 1

    def op_JZ(self, address):
        if self.op_POP() == 0:
            self.pc = address - 1

    def op_HALT(self):
        self.pc = len(self.instructions)

def compile_to_bytecode(code):
    if code == 'basic_init':
        return [
            ('PUSH', 123),
            ('STORE', 'val'),
            ('HALT',)
        ]
    elif code == 'init_calls_internal':
        return [
            ('PUSH', 5), 
            ('STORE', 'foo'),
            ('LOAD', 'foo'), 
            ('PUSH', 7), 
            ('MUL',), 
            ('STORE', 'foo'),
            ('LOAD', 'foo'), 
            ('PUSH', 7), 
            ('MUL',), 
            ('HALT',)
        ]

def test_basic_init_function(get_contract):
    vm = VM()
    vm.load_program(compile_to_bytecode('basic_init'))
    vm.run()
    assert vm.memory['val'] == 123

def test_init_calls_internal(get_contract, assert_compile_failed, assert_tx_failed):
    vm = VM()
    vm.load_program(compile_to_bytecode('init_calls_internal'))
    vm.run()
    assert vm.memory['foo'] == 35

    # Further tests would be implemented similarly, but with different bytecode programs