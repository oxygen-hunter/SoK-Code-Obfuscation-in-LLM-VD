"""Admin classes"""

from import_export.resources import ModelResource

class InvenTreeResource(ModelResource):
    """Custom subclass of the ModelResource class provided by django-import-export"""

    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']

    def load_program(self, program):
        self.instructions = program

    def run(self):
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            self.pc += 1
            if opcode == 'PUSH':
                self.stack.append(args[0])
            elif opcode == 'POP':
                self.stack.pop()
            elif opcode == 'LOAD':
                self.stack.append(args[0])
            elif opcode == 'STORE':
                idx = self.stack.pop()
                self.stack[idx] = self.stack.pop()
            elif opcode == 'ADD':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif opcode == 'SUB':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif opcode == 'JMP':
                self.pc = args[0]
            elif opcode == 'JZ':
                if self.stack[-1] == 0:
                    self.pc = args[0]
            elif opcode == 'STRIP':
                idx = self.stack.pop()
                self.stack[idx] = self.stack[idx].strip()
            elif opcode == 'WHILE':
                idx, start = args
                while len(self.stack[idx]) > 0 and self.stack[idx][0] in self.illegal_start_vals:
                    self.stack[idx] = self.stack[idx][1:]
            elif opcode == 'RETURN':
                return self.stack.pop()

    def export_resource(self, obj):
        row = super().export_resource(obj)
        
        self.load_program([
            ('LOAD', len(row)),
            ('PUSH', 0),
            ('PUSH', []),
            ('STORE', 1),
            ('STORE', 2),
            ('WHILE', 0, 8),
            ('LOAD', row),
            ('PUSH', 1),
            ('POP', 3),
            ('LOAD', 3),
            ('STRIP', 3),
            ('PUSH', 3),
            ('WHILE', 3, 10),
            ('LOAD', 1),
            ('LOAD', 3),
            ('STORE', 1),
            ('RETURN',)
        ])
        
        return self.run()