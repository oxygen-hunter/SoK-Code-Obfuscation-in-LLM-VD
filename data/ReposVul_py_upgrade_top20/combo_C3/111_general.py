from django.contrib.postgres.fields import ArrayField, JSONField
from django.db.models import Value
from django.db.models.aggregates import Aggregate
from .mixins import OrderableAggMixin

__all__ = [
    'ArrayAgg', 'BitAnd', 'BitOr', 'BoolAnd', 'BoolOr', 'JSONBAgg', 'StringAgg',
]

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.pc += 1
            if instr[0] == 'PUSH':
                self.stack.append(instr[1])
            elif instr[0] == 'POP':
                self.stack.pop()
            elif instr[0] == 'ADD':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif instr[0] == 'SUB':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif instr[0] == 'LOAD':
                self.stack.append(instr[1])
            elif instr[0] == 'STORE':
                var_name = instr[1]
                globals()[var_name] = self.stack.pop()
            elif instr[0] == 'JMP':
                self.pc = instr[1]
            elif instr[0] == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = instr[1]

def compile_aggregation(agg_class, *args, **kwargs):
    vm = VM()
    if agg_class == 'ArrayAgg':
        vm.load([
            ('LOAD', 'ARRAY_AGG'),
            ('PUSH', 'ARRAY_AGG'),
            ('STORE', 'function'),
            ('LOAD', True),
            ('PUSH', True),
            ('STORE', 'allow_distinct')
        ])
    elif agg_class == 'BitAnd':
        vm.load([
            ('LOAD', 'BIT_AND'),
            ('PUSH', 'BIT_AND'),
            ('STORE', 'function')
        ])
    elif agg_class == 'BitOr':
        vm.load([
            ('LOAD', 'BIT_OR'),
            ('PUSH', 'BIT_OR'),
            ('STORE', 'function')
        ])
    elif agg_class == 'BoolAnd':
        vm.load([
            ('LOAD', 'BOOL_AND'),
            ('PUSH', 'BOOL_AND'),
            ('STORE', 'function')
        ])
    elif agg_class == 'BoolOr':
        vm.load([
            ('LOAD', 'BOOL_OR'),
            ('PUSH', 'BOOL_OR'),
            ('STORE', 'function')
        ])
    elif agg_class == 'JSONBAgg':
        vm.load([
            ('LOAD', 'JSONB_AGG'),
            ('PUSH', 'JSONB_AGG'),
            ('STORE', 'function'),
            ('LOAD', JSONField()),
            ('PUSH', JSONField()),
            ('STORE', 'output_field')
        ])
    elif agg_class == 'StringAgg':
        vm.load([
            ('LOAD', 'STRING_AGG'),
            ('PUSH', 'STRING_AGG'),
            ('STORE', 'function'),
            ('LOAD', True),
            ('PUSH', True),
            ('STORE', 'allow_distinct'),
            ('LOAD', str(args[1])),
            ('PUSH', str(args[1])),
            ('STORE', 'delimiter'),
            ('LOAD', Value(str(args[1]))),
            ('PUSH', Value(str(args[1]))),
            ('STORE', 'delimiter_expr')
        ])
    vm.run()
    return vm

class ArrayAgg(OrderableAggMixin, Aggregate):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        vm = compile_aggregation('ArrayAgg')
        self.function = globals()['function']
        self.allow_distinct = globals()['allow_distinct']

    @property
    def output_field(self):
        return ArrayField(self.source_expressions[0].output_field)

    def convert_value(self, value, expression, connection):
        if not value:
            return []
        return value

class BitAnd(Aggregate):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        vm = compile_aggregation('BitAnd')
        self.function = globals()['function']

class BitOr(Aggregate):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        vm = compile_aggregation('BitOr')
        self.function = globals()['function']

class BoolAnd(Aggregate):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        vm = compile_aggregation('BoolAnd')
        self.function = globals()['function']

class BoolOr(Aggregate):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        vm = compile_aggregation('BoolOr')
        self.function = globals()['function']

class JSONBAgg(Aggregate):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        vm = compile_aggregation('JSONBAgg')
        self.function = globals()['function']
        self.output_field = globals()['output_field']

    def convert_value(self, value, expression, connection):
        if not value:
            return []
        return value

class StringAgg(OrderableAggMixin, Aggregate):
    def __init__(self, expression, delimiter, **extra):
        vm = compile_aggregation('StringAgg', expression, delimiter, **extra)
        self.function = globals()['function']
        self.allow_distinct = globals()['allow_distinct']
        delimiter_expr = globals()['delimiter_expr']
        super().__init__(expression, delimiter_expr, **extra)

    def convert_value(self, value, expression, connection):
        if not value:
            return ''
        return value