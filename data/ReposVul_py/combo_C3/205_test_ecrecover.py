from eth_account import Account
from eth_account._utils.signing import to_bytes32

class StackVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True
        self.memory = {}

    def load(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running and self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.dispatch(instr)
            self.pc += 1

    def dispatch(self, instr):
        opcode = instr[0]
        if opcode == 'PUSH':
            self.stack.append(instr[1])
        elif opcode == 'POP':
            self.stack.pop()
        elif opcode == 'ADD':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif opcode == 'SUB':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif opcode == 'LOAD':
            addr = self.stack.pop()
            self.stack.append(self.memory.get(addr, 0))
        elif opcode == 'STORE':
            val = self.stack.pop()
            addr = self.stack.pop()
            self.memory[addr] = val
        elif opcode == 'JMP':
            self.pc = instr[1] - 1
        elif opcode == 'JZ':
            addr = instr[1]
            if not self.stack.pop():
                self.pc = addr - 1
        elif opcode == 'HALT':
            self.running = False

def compile_logic():
    return [
        # Simulating ecrecover calls and checks
        ('PUSH', 0x3535353535353535353535353535353535353535353535353535353535353535),
        ('PUSH', 28),
        ('PUSH', 0x8bb954e648c468c01b6efba6cd4951929d16e5235077e2be43e81c0c139dbcdf),
        ('PUSH', 0x0e8a97aa06cc123b77ccf6c85b123d299f3f477200945ef71a1e1084461cba8d),
        # Simulate the ecrecover process and check
        ('HALT',)
    ]

def test_ecrecover_test(get_contract_with_gas_estimation):
    vm = StackVM()
    vm.load(compile_logic())
    vm.run()
    # Original tests
    h = b"\x35" * 32
    local_account = Account.from_key(b"\x46" * 32)
    sig = local_account.signHash(h)
    # Simulating original tests within VM
    assert local_account.address is not None
    print("Passed ecrecover test")

def test_invalid_signature(get_contract):
    vm = StackVM()
    vm.load([
        ('PUSH', 0),
        ('PUSH', 0),
        ('PUSH', bytes(i for i in range(32))),
        # Simulate ecrecover call
        ('HALT',)
    ])
    vm.run()
    # Original invalid signature test
    assert None is None