import argparse
import os
import sys
from . import SURFACES, VERSION

class VM:
    def __init__(self):
        self.stack = []
        self.instructions = []
        self.pc = 0

    def load_program(self, program):
        self.instructions = program

    def fetch(self):
        instr = self.instructions[self.pc]
        self.pc += 1
        return instr

    def run(self):
        while self.pc < len(self.instructions):
            opcode, operand = self.fetch()
            if opcode == "PUSH":
                self.stack.append(operand)
            elif opcode == "POP":
                self.stack.pop()
            elif opcode == "ADD":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif opcode == "SUB":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif opcode == "LOAD":
                self.stack.append(operand)
            elif opcode == "STORE":
                self.stack[-1] = operand
            elif opcode == "JMP":
                self.pc = operand
            elif opcode == "JZ":
                if self.stack.pop() == 0:
                    self.pc = operand
            elif opcode == "CALL":
                operand()
            elif opcode == "HALT":
                break

def execute_vm(argv=None, stdout=None, stdin=None):
    vm = VM()
    vm.load_program([
        ("CALL", get_options),
        ("CALL", prepare_context),
        ("CALL", execute_conversion),
        ("HALT", None)
    ])
    vm.run()

def get_options():
    parser = argparse.ArgumentParser(description='Convert SVG files to other formats')
    parser.add_argument('input', default='-', help='input filename or URL')
    parser.add_argument('-v', '--version', action='version', version=VERSION)
    parser.add_argument('-f', '--format', help='output format', choices=sorted([surface.lower() for surface in SURFACES]))
    parser.add_argument('-d', '--dpi', default=96, type=float, help='ratio between 1 inch and 1 pixel')
    parser.add_argument('-W', '--width', default=None, type=float, help='width of the parent container in pixels')
    parser.add_argument('-H', '--height', default=None, type=float, help='height of the parent container in pixels')
    parser.add_argument('-s', '--scale', default=1, type=float, help='output scaling factor')
    parser.add_argument('-b', '--background', metavar='COLOR', help='output background color')
    parser.add_argument('-n', '--negate-colors', action='store_true', help='replace every vector color with its complement')
    parser.add_argument('-i', '--invert-images', action='store_true', help='replace every raster pixel with its complementary color')
    parser.add_argument('-u', '--unsafe', action='store_true', help='fetch external files, resolve XML entities and allow very large files (WARNING: vulnerable to XXE attacks and various DoS)')
    parser.add_argument('--output-width', default=None, type=float, help='desired output width in pixels')
    parser.add_argument('--output-height', default=None, type=float, help='desired output height in pixels')
    parser.add_argument('-o', '--output', default='-', help='output filename')
    global options
    options = parser.parse_args()

def prepare_context():
    global kwargs
    kwargs = {
        'parent_width': options.width,
        'parent_height': options.height,
        'dpi': options.dpi,
        'scale': options.scale,
        'unsafe': options.unsafe,
        'background_color': options.background,
        'negate_colors': options.negate_colors,
        'invert_images': options.invert_images,
        'output_width': options.output_width,
        'output_height': options.output_height
    }
    kwargs['write_to'] = (sys.stdout.buffer if options.output == '-' else options.output)
    kwargs['file_obj'] = (sys.stdin.buffer if options.input == '-' else None)
    kwargs['url'] = (None if options.input == '-' else options.input)

def execute_conversion():
    output_format = (options.format or os.path.splitext(options.output)[1].lstrip('.') or 'pdf').upper()
    SURFACES[output_format].convert(**kwargs)

if __name__ == '__main__':  # pragma: no cover
    execute_vm()