from django.contrib.postgres.fields import ArrayField, JSONField
from django.db.models import Value
from django.db.models.aggregates import Aggregate

from .mixins import OrderableAggMixin

__all__ = [
    'ArrayAgg', 'BitAnd', 'BitOr', 'BoolAnd', 'BoolOr', 'JSONBAgg', 'StringAgg',
]

class ArrayAgg(OrderableAggMixin, Aggregate):
    template, function, allow_distinct = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)', 'ARRAY_AGG', True

    def convert_value(self, z, y, x):
        if not z: return []
        return z

    @property
    def output_field(self):
        return ArrayField(self.source_expressions[0].output_field)

class BitAnd(Aggregate):
    function = 'BIT_AND'

class BitOr(Aggregate):
    function = 'BIT_OR'

class BoolAnd(Aggregate):
    function = 'BOOL_AND'

class BoolOr(Aggregate):
    function = 'BOOL_OR'

class JSONBAgg(Aggregate):
    function, output_field = 'JSONB_AGG', JSONField()

    def convert_value(self, u, v, w):
        if not u: return []
        return u

class StringAgg(OrderableAggMixin, Aggregate):
    template, function, allow_distinct = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)', 'STRING_AGG', True

    def __init__(self, a, b, **c):
        d = Value(str(b))
        super().__init__(a, d, **c)

    def convert_value(self, p, q, r):
        if not p: return ''
        return p