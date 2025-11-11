#-----------------------------------------------------------------------------
# Custom VM Implementation
#-----------------------------------------------------------------------------

class VM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.instructions = []
        self.running = True

    def load_program(self, program):
        self.instructions = program

    def run(self):
        while self.running and self.program_counter < len(self.instructions):
            instruction = self.instructions[self.program_counter]
            self.execute(instruction)

    def execute(self, instruction):
        op = instruction[0]
        if op == 'PUSH':
            self.stack.append(instruction[1])
        elif op == 'POP':
            self.stack.pop()
        elif op == 'ADD':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a + b)
        elif op == 'SUB':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a - b)
        elif op == 'JMP':
            self.program_counter = instruction[1] - 1
        elif op == 'JZ':
            if self.stack.pop() == 0:
                self.program_counter = instruction[1] - 1
        elif op == 'LOAD_GLOBAL':
            self.stack.append(globals()[instruction[1]])
        elif op == 'STORE_GLOBAL':
            globals()[instruction[1]] = self.stack.pop()
        elif op == 'HALT':
            self.running = False
        self.program_counter += 1

#-----------------------------------------------------------------------------
# Compiled Bytecode
#-----------------------------------------------------------------------------

program = [
    ('LOAD_GLOBAL', 'sys'),        # Load 'sys' module
    ('LOAD_GLOBAL', 'version_info'), # Load 'version_info' attribute
    ('PUSH', (3, 8)),              # Push (3, 8) to stack
    ('SUB',),                      # Compare version_info < (3, 8)
    ('JZ', 11),                    # Jump to HALT if false
    ('PUSH', """
IPython 8+ supports Python 3.8 and above, following NEP 29.
...
    """),                         # Error message
    ('LOAD_GLOBAL', 'ImportError'), # Load ImportError class
    ('PUSH', 'raise'),            # Push 'raise' keyword
    ('ADD',),                     # Combine 'raise' with ImportError
    ('STORE_GLOBAL', 'error'),    # Store in 'error'
    ('LOAD_GLOBAL', 'error'),     # Load 'error'
    ('HALT',),                    # Halt the VM
]

vm = VM()
vm.load_program(program)
vm.run()