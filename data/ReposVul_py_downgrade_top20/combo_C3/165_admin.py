from import_export.resources import ModelResource

class InvenTreeVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            instruction = self.program[self.pc]
            op = instruction[0]
            args = instruction[1:]

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
                continue
            elif op == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = args[0]
                    continue
            elif op == 'LOAD':
                self.stack.append(args[0])
            elif op == 'STORE':
                value = self.stack.pop()
                self.stack[args[0]] = value

            self.pc += 1

class InvenTreeResource(ModelResource):
    def export_resource(self, obj):
        row = super().export_resource(obj)

        illegal_start_vals = ['@', '=', '+', '-', '@', '\t', '\r', '\n']

        def is_illegal(val):
            for char in illegal_start_vals:
                if val.startswith(char):
                    return True
            return False

        def preprocess_vm(val):
            vm = InvenTreeVM()
            vm.load_program([
                ('PUSH', val),
                ('LOAD', 0),
                ('PUSH', 0),
                ('JZ', 7),
                ('PUSH', val[1:]),
                ('STORE', 0),
                ('JMP', 1),
                ('POP',),
            ])
            vm.run()
            return vm.stack[-1]

        for idx, val in enumerate(row):
            if isinstance(val, str):
                val = val.strip()
                while len(val) > 0 and is_illegal(val):
                    val = preprocess_vm(val)
                row[idx] = val

        return row