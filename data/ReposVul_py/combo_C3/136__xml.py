from typing import IO, Any, AnyStr, Union
from lxml.etree import parse as _parse

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
            getattr(self, f"op_{op}")(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_CALL(self, func, *args):
        result = func(*args)
        self.stack.append(result)

    def op_STORE(self, key):
        self.memory[key] = self.op_POP()

    def op_LOAD(self, key):
        self.op_PUSH(self.memory[key])

    def op_RETURN(self):
        return self.op_POP()

def _secure_xml_parser(recover):
    return {
        'op': 'CALL',
        'func': _create_xml_parser,
        'args': [recover]
    }

def _create_xml_parser(recover):
    return _UnsafeXMLParser(resolve_entities=False, no_network=True, recover=recover, remove_pis=False)

def parse_xml(source: Union[AnyStr, IO[Any]], recover: bool = False):
    vm = VM()
    vm.load_program([
        ('PUSH', recover),
        _secure_xml_parser(recover),
        ('STORE', 'parser'),
        ('PUSH', source),
        ('LOAD', 'parser'),
        ('CALL', _parse),
        ('RETURN',)
    ])
    return vm.run()

__all__ = ['parse_xml']