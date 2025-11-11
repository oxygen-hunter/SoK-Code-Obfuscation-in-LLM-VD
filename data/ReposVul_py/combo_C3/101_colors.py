import re

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
        self.running = True

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.running:
            instr = self.program[self.pc]
            opcode = instr[0]
            operands = instr[1:]
            self.execute(opcode, operands)

    def execute(self, opcode, operands):
        if opcode == 'PUSH':
            self.stack.append(operands[0])
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
        elif opcode == 'MUL':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a * b)
        elif opcode == 'DIV':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a / b)
        elif opcode == 'JMP':
            self.pc = operands[0] - 1
        elif opcode == 'JZ':
            condition = self.stack.pop()
            if condition == 0:
                self.pc = operands[0] - 1
        elif opcode == 'LOAD':
            self.stack.append(operands[0])
        elif opcode == 'STORE':
            value = self.stack.pop()
            self.stack.append(value)
        elif opcode == 'HALT':
            self.running = False

        self.pc += 1

COLORS = {
    'aliceblue': (240 / 255, 248 / 255, 255 / 255, 1),
    'antiquewhite': (250 / 255, 235 / 255, 215 / 255, 1),
    'aqua': (0 / 255, 255 / 255, 255 / 255, 1),
    # ... other colors ...
    'yellowgreen': (154 / 255, 205 / 255, 50 / 255, 1),
    'none': (0, 0, 0, 0),
    'transparent': (0, 0, 0, 0),
}

RGBA = re.compile(r'rgba\((.+?)\)')
RGB = re.compile(r'rgb\((.+?)\)')
HEX_RRGGBB = re.compile('#[0-9a-f]{6}')
HEX_RGB = re.compile('#[0-9a-f]{3}')

def vm_color(string, opacity=1):
    program = []
    if not string:
        program.append(('PUSH', (0, 0, 0, 0)))
        program.append(('HALT',))
    else:
        string = string.strip().lower()
        if string in COLORS:
            r, g, b, a = COLORS[string]
            program.append(('PUSH', (r, g, b, a * opacity)))
            program.append(('HALT',))
        else:
            match = RGBA.search(string)
            if match:
                program.append(('LOAD', match))
                program.append(('PUSH', (opacity,)))
                program.append(('MUL',))
                program.append(('HALT',))
            else:
                match = RGB.search(string)
                if match:
                    program.append(('LOAD', match))
                    program.append(('PUSH', (opacity,)))
                    program.append(('HALT',))
                else:
                    match = HEX_RRGGBB.search(string)
                    if match:
                        program.append(('LOAD', match))
                        program.append(('PUSH', (opacity,)))
                        program.append(('HALT',))
                    else:
                        match = HEX_RGB.search(string)
                        if match:
                            program.append(('LOAD', match))
                            program.append(('PUSH', (opacity,)))
                            program.append(('HALT',))
                        else:
                            program.append(('PUSH', (0, 0, 0, 1)))
                            program.append(('HALT',))

    vm = VM()
    vm.load_program(program)
    vm.run()
    return vm.stack[-1]

def vm_negate_color(rgba_tuple):
    program = [
        ('PUSH', rgba_tuple),
        ('PUSH', (1, 1, 1, 0)),
        ('SUB',),
        ('HALT',)
    ]
    vm = VM()
    vm.load_program(program)
    vm.run()
    return vm.stack[-1]