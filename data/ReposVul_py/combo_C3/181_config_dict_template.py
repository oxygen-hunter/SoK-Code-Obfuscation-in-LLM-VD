import random
import string
from .constants import DEFAULT_BACKUP_DIRECTORY

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.globals = {}

    def run(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            cmd = instr[0]
            if cmd == 'PUSH':
                self.stack.append(instr[1])
            elif cmd == 'POP':
                self.stack.pop()
            elif cmd == 'ADD':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif cmd == 'SUB':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(b - a)
            elif cmd == 'JMP':
                self.pc = instr[1] - 1
            elif cmd == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = instr[1] - 1
            elif cmd == 'LOAD':
                self.stack.append(self.globals[instr[1]])
            elif cmd == 'STORE':
                self.globals[instr[1]] = self.stack.pop()
            elif cmd == 'CALL':
                func = instr[1]
                func(self)
            self.pc += 1

def make_password(vm):
    length = vm.stack.pop()
    vm.stack.append("".join(random.SystemRandom().choice(string.ascii_letters + string.digits) for _ in range(length)))

def is_email(vm):
    user_input = vm.stack.pop()
    vm.stack.append("@" in user_input)
    vm.stack.append("Please enter a valid email")

instructions = [
    # make_password instructions
    ('PUSH', 16),
    ('CALL', make_password),
    ('STORE', 'generated_password'),

    # is_email instructions
    ('PUSH', 'test@example.com'),
    ('CALL', is_email),
    ('STORE', 'is_valid'),

    # Configuration dictionary
    ('PUSH', [
        {
            "name": "general",
            "values": [
                {
                    "option": "hostname",
                    "default": "mail.%(domain)s",
                }
            ]
        },
        {
            "name": "certificate",
            "values": [
                {
                    "option": "generate",
                    "default": "true",
                },
                {
                    "option": "type",
                    "default": "self-signed",
                    "customizable": True,
                    "question": "Please choose your certificate type",
                    "values": ["self-signed", "letsencrypt"],
                }
            ],
        },
        # ... other configuration entries
        {
            "name": "backup",
            "values": [
                {
                    "option": "default_path",
                    "default": DEFAULT_BACKUP_DIRECTORY
                }
            ]
        }
    ]),
    ('STORE', 'ConfigDictTemplate'),
]

vm = VM()
vm.run(instructions)