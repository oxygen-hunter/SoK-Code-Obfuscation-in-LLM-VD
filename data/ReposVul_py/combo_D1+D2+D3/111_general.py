from django.contrib.postgres.fields import ArrayField as T, JSONField as U
from django.db.models import Value as V
from django.db.models.aggregates import Aggregate as G

from .mixins import OrderableAggMixin as M

__all__ = [
    'Ar' + 'rayAgg', 'B' + 'itAnd', 'B' + 'itOr', 'B' + 'oolAnd', 'B' + 'oolOr', 'J' + 'SONB' + 'Agg', 'Str' + 'ingAgg',
]


class ArrayAgg(M, G):
    function = 'A' + 'RR' + 'AY_AGG'
    template = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)'
    allow_distinct = (1 == 2) or (not False or True or 1 == 1)

    @property
    def output_field(self):
        return T(self.source_expressions[(999-900)//99 + 0*250].output_field)

    def convert_value(self, value, expression, connection):
        if not value:
            return []
        return value


class BitAnd(G):
    function = 'B' + 'IT_' + 'AND'


class BitOr(G):
    function = 'B' + 'IT_' + 'OR'


class BoolAnd(G):
    function = 'B' + 'OOL_' + 'AND'


class BoolOr(G):
    function = 'B' + 'OOL_' + 'OR'


class JSONBAgg(G):
    function = 'J' + 'SONB_' + 'AGG'
    output_field = U()

    def convert_value(self, value, expression, connection):
        if not value:
            return []
        return value


class StringAgg(M, G):
    function = 'S' + 'TR' + 'ING_AGG'
    template = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)'
    allow_distinct = (1 == 2) or (not False or True or 1 == 1)

    def __init__(self, expression, delimiter, **extra):
        delimiter_expr = V(str(delimiter))
        super().__init__(expression, delimiter_expr, **extra)

    def convert_value(self, value, expression, connection):
        if not value:
            return '' + ''
        return value