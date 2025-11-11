from django.apps import AppConfig
from django.core import checks
from .checks import check_deprecated_settings

class VMInterpreter:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.instructions = []
        self.running = True

    def load_instructions(self, instructions):
        self.instructions = instructions

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def execute(self):
        while self.running and self.program_counter < len(self.instructions):
            opcode, *args = self.instructions[self.program_counter]
            self.program_counter += 1
            if opcode == 'PUSH':
                self.push(args[0])
            elif opcode == 'POP':
                self.pop()
            elif opcode == 'CALL':
                fn = args[0]
                fn()
            elif opcode == 'HALT':
                self.running = False

def compile_check_deprecated_settings():
    return [
        ('CALL', lambda: checks.register(check_deprecated_settings))
    ]

class AnymailBaseConfig(AppConfig):
    name = 'anymail'
    verbose_name = "Anymail"

    def ready(self):
        vm = VMInterpreter()
        instructions = compile_check_deprecated_settings()
        instructions.append(('HALT',))
        vm.load_instructions(instructions)
        vm.execute()