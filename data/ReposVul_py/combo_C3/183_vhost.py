import string
import resource
import error

class NameVirtualHost(resource.Resource):

    def __init__(self):
        resource.Resource.__init__(self)
        self.hosts = {}

    def addHost(self, name, resrc):
        self.hosts[name] = resrc

    def _getResourceForRequest(self, request):
        host = string.lower(request.getHeader('host'))
        return self.hosts.get(host, error.NoResource("host %s not in vhost map" % repr(host)))

    def render(self, request):
        resrc = self._getResourceForRequest(request)
        return resrc.render(request)

    def getChild(self, path, request):
        resrc = self._getResourceForRequest(request)
        return resrc.getChildWithDefault(path, request)

class VM:
    def __init__(self):
        self.stack = []
        self.registers = {}
        self.pc = 0
        self.program = []

    def run(self, program):
        self.pc = 0
        self.program = program
        while self.pc < len(self.program):
            opcode, *args = self.program[self.pc]
            self.dispatch(opcode, args)
            self.pc += 1

    def dispatch(self, opcode, args):
        if opcode == 'PUSH':
            self.stack.append(args[0])
        elif opcode == 'POP':
            self.stack.pop()
        elif opcode == 'ADD':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a + b)
        elif opcode == 'SUB':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a - b)
        elif opcode == 'LOAD':
            reg = args[0]
            self.stack.append(self.registers[reg])
        elif opcode == 'STORE':
            reg = args[0]
            self.registers[reg] = self.stack.pop()
        elif opcode == 'JMP':
            self.pc = args[0] - 1
        elif opcode == 'JZ':
            if self.stack.pop() == 0:
                self.pc = args[0] - 1

def compile_to_vm(nvh):
    vm = VM()
    program = [
        ('PUSH', nvh),  # nvh instance
        # Assuming some operations here, just for illustration
        ('STORE', 'nvh_instance'),
        ('LOAD', 'nvh_instance'),
        ('PUSH', 'addHost'),
        ('STORE', 'method'),
        # Call method (just a hypothetical example)
        ('LOAD', 'method'),
        ('PUSH', 'divunal.com'),
        ('PUSH', 'divunalDirectory'),
        ('STORE', 'arg1'),
        ('STORE', 'arg2'),
        # Method call simulation
        ('LOAD', 'arg1'),
        ('LOAD', 'arg2'),
        ('ADD',),
    ]
    vm.run(program)
    return vm

nvh = NameVirtualHost()
vm = compile_to_vm(nvh)