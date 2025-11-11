import re

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.registers = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            getattr(self, op)(*args)
            self.pc += 1

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        return self.stack.pop()

    def JMP(self, target):
        self.pc = target - 1

    def JZ(self, target):
        if not self.POP():
            self.pc = target - 1

    def STORE(self, name):
        self.registers[name] = self.POP()

    def LOAD(self, name):
        self.stack.append(self.registers[name])

    def ADD(self):
        b = self.POP()
        a = self.POP()
        self.stack.append(a + b)

    def SUB(self):
        b = self.POP()
        a = self.POP()
        self.stack.append(a - b)

    def CALL(self, name):
        getattr(self, name)()

def valid_regex_vm(vm):
    vm.load_program([
        ('LOAD', 'regex'),
        ('PUSH', None),
        ('JZ', 4),
        ('PUSH', True),
        ('PUSH', None),
        ('PUSH', None),
        ('JMP', 16),
        ('LOAD', 'regex'),
        ('CALL', 'compile_regex'),
        ('STORE', 'compiled_regex'),
        ('PUSH', True),
        ('LOAD', 'compiled_regex'),
        ('PUSH', None),
        ('JMP', 16),
        ('LOAD', 'regex_err'),
        ('CALL', 'regex_error_msg'),
        ('STORE', 'err_msg'),
        ('PUSH', False),
        ('PUSH', None),
        ('LOAD', 'err_msg')
    ])
    vm.run()
    return vm.stack[-3:]

def valid_exclude_list_vm(vm):
    vm.load_program([
        ('LOAD', 'exclude_list'),
        ('PUSH', None),
        ('JZ', 3),
        ('PUSH', True),
        ('PUSH', None),
        ('PUSH', None),
        ('JMP', 11),
        ('LOAD', 'exclude_list'),
        ('CALL', 'combine_exclude_list'),
        ('STORE', 'combined_regex'),
        ('LOAD', 'combined_regex'),
        ('CALL', 'valid_regex_vm')
    ])
    vm.run()
    return vm.stack[-3:]

def valid_hex_vm(vm):
    vm.load_program([
        ('LOAD', 'value'),
        ('CALL', 'convert_hex'),
        ('STORE', 'is_valid_hex'),
        ('LOAD', 'is_valid_hex')
    ])
    vm.run()
    return vm.stack[-1]

def valid_uuid_vm(vm):
    vm.load_program([
        ('LOAD', 'uuid'),
        ('CALL', 'match_uuid'),
        ('STORE', 'is_valid_uuid'),
        ('LOAD', 'is_valid_uuid')
    ])
    vm.run()
    return vm.stack[-1]

def valid_agent_id_vm(vm):
    vm.load_program([
        ('LOAD', 'agent_id'),
        ('CALL', 'match_agent_id'),
        ('STORE', 'is_valid_agent_id'),
        ('LOAD', 'is_valid_agent_id')
    ])
    vm.run()
    return vm.stack[-1]

def compile_regex(vm):
    try:
        vm.registers['compiled_regex'] = re.compile(vm.registers['regex'])
    except re.error as regex_err:
        vm.registers['regex_err'] = regex_err

def regex_error_msg(vm):
    vm.stack.append("Invalid regex: " + vm.registers['regex_err'].msg + ".")

def combine_exclude_list(vm):
    vm.stack.append("(" + ")|(".join(vm.registers['exclude_list']) + ")")

def convert_hex(vm):
    try:
        int(vm.registers['value'], 16)
        vm.stack.append(True)
    except Exception:
        vm.stack.append(False)

def match_uuid(vm):
    try:
        valid = bool(
            re.fullmatch(
                r"[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12}",
                vm.registers['uuid'],
                re.I,
            )
        )
        vm.stack.append(valid)
    except Exception:
        vm.stack.append(False)

def match_agent_id(vm):
    try:
        valid = bool(re.fullmatch(r"[\w.-]+", vm.registers['agent_id']))
        vm.stack.append(valid)
    except Exception:
        vm.stack.append(False)

def valid_regex(regex):
    vm = VM()
    vm.registers['regex'] = regex
    return valid_regex_vm(vm)

def valid_exclude_list(exclude_list):
    vm = VM()
    vm.registers['exclude_list'] = exclude_list
    return valid_exclude_list_vm(vm)

def valid_hex(value):
    vm = VM()
    vm.registers['value'] = value
    return valid_hex_vm(vm)

def valid_uuid(uuid):
    vm = VM()
    vm.registers['uuid'] = uuid
    return valid_uuid_vm(vm)

def valid_agent_id(agent_id):
    vm = VM()
    vm.registers['agent_id'] = agent_id
    return valid_agent_id_vm(vm)