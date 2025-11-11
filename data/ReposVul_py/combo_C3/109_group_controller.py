import logging
logger = logging.getLogger(__name__)

from bottle import route, get, post, put, delete
from bottle import request, response

def error(code, message):
    response.status = code
    message['status'] = code
    return message

get_user_table = lambda db: db.get_table('users', primary_id='userid', primary_type='String(100)')

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
        self.memory = {}

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            op, *args = self.program[self.pc]
            self.pc += 1
            if op == 'PUSH':
                self.stack.append(args[0])
            elif op == 'POP':
                self.stack.pop()
            elif op == 'ADD':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif op == 'SUB':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif op == 'JMP':
                self.pc = args[0]
            elif op == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = args[0]
            elif op == 'LOAD':
                self.stack.append(self.memory[args[0]])
            elif op == 'STORE':
                self.memory[args[0]] = self.stack.pop()

def execute_delete_group(vm):
    vm.load_program([
        ('PUSH', 'groups'),
        ('LOAD', 0),
        ('PUSH', 'name'),
        ('PUSH', vm.stack[-1]),
        ('LOAD', 1),
        ('JZ', 10),
        ('PUSH', 404),
        ('PUSH', {'error': 'group not found'}),
        ('CALL', error),
        ('RET',),
        ('PUSH', 'groups'),
        ('PUSH', 'name'),
        ('PUSH', vm.stack[-1]),
        ('DELETE',),
        ('PUSH', {'status': 200}),
        ('RET',)
    ])
    return vm.run()

def execute_get_group(vm):
    vm.load_program([
        ('PUSH', 'groups'),
        ('LOAD', 0),
        ('PUSH', 'name'),
        ('PUSH', vm.stack[-1]),
        ('LOAD', 1),
        ('JZ', 10),
        ('PUSH', 404),
        ('PUSH', {'error': 'Not a valid group'}),
        ('CALL', error),
        ('RET',),
        ('PUSH', 'userid'),
        ('FILTER',),
        ('JZ', 18),
        ('PUSH', 'group_name'),
        ('PUSH', []),
        ('RET',),
        ('PUSH', {}),
        ('ENUM',),
        ('PUSH', 'userid'),
        ('PUSH', 'group_name'),
        ('PUSH', []),
        ('CALL', db.executable.execute),
        ('RET',)
    ])
    return vm.run()

def execute_post_group(vm):
    vm.load_program([
        ('PUSH', 'groups'),
        ('LOAD', 0),
        ('PUSH', 'name'),
        ('PUSH', vm.stack[-1]),
        ('LOAD', 1),
        ('JZ', 10),
        ('PUSH', 409),
        ('PUSH', {'error': 'Group already exists'}),
        ('CALL', error),
        ('RET',),
        ('PUSH', 'groups'),
        ('PUSH', {'name': vm.stack[-1], 'userid': None}),
        ('INSERT',),
        ('PUSH', {'status': 200}),
        ('RET',)
    ])
    return vm.run()

@delete('/groups/<group_name>')
def delete_group(db, group_name):
    vm = VM()
    vm.memory[0] = db.get_table('groups')
    vm.memory[1] = vm.memory[0].find_one(name=group_name)
    return execute_delete_group(vm)

@get('/groups/<group_name>')
def get_group(db, group_name):
    vm = VM()
    vm.memory[0] = db.get_table('groups')
    vm.memory[1] = vm.memory[0].find(name=group_name)
    return execute_get_group(vm)

@route('/groups/<group_name>', method=['POST', 'PUT'])
def post_group(db, group_name):
    vm = VM()
    vm.memory[0] = db.get_table('groups')
    vm.memory[1] = vm.memory[0].find_one(name=group_name)
    return execute_post_group(vm)