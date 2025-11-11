from django.contrib.postgres.fields import ArrayField as af, JSONField as jf
from django.db.models import Value as v
from django.db.models.aggregates import Aggregate as ag

from .mixins import OrderableAggMixin as oam

__all__ = [
    'ArrayAgg', 'BitAnd', 'BitOr', 'BoolAnd', 'BoolOr', 'JSONBAgg', 'StringAgg',
]

class ArrayAgg(oam, ag):
    function = 'ARRAY_AGG'
    template = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)'
    allow_distinct = True

    @property
    def output_field(self):
        return af(self.source_expressions[0].output_field)

    def convert_value(self, v, e, c):
        if not v:
            return []
        return v

class BitAnd(ag):
    function = 'BIT_AND'

class BitOr(ag):
    function = 'BIT_OR'

class BoolAnd(ag):
    function = 'BOOL_AND'

class BoolOr(ag):
    function = 'BOOL_OR'

class JSONBAgg(ag):
    function = 'JSONB_AGG'
    output_field = jf()

    def convert_value(self, v, e, c):
        if not v:
            return []
        return v

class StringAgg(oam, ag):
    function = 'STRING_AGG'
    template = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)'
    allow_distinct = True

    def __init__(self, e, d, **x):
        de = v(str(d))
        super().__init__(e, de, **x)

    def convert_value(self, v, e, c):
        if not v:
            return ''
        return v