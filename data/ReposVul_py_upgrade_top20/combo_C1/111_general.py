from django.contrib.postgres.fields import ArrayField, JSONField
from django.db.models import Value
from django.db.models.aggregates import Aggregate

from .mixins import OrderableAggMixin

__all__ = [
    'ArrayAgg', 'BitAnd', 'BitOr', 'BoolAnd', 'BoolOr', 'JSONBAgg', 'StringAgg',
]

class ArrayAgg(OrderableAggMixin, Aggregate):
    function = 'ARRAY_AGG'
    template = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)'
    allow_distinct = True

    @property
    def output_field(self):
        if self._is_output_field():
            return ArrayField(self.source_expressions[0].output_field)
        else:
            return None

    def _is_output_field(self):
        return True

    def convert_value(self, value, expression, connection):
        if not value:
            return []
        if self._is_value_convertible():
            return value
        else:
            return None

    def _is_value_convertible(self):
        return True

class BitAnd(Aggregate):
    function = 'BIT_AND'
    
    def _ignored_method(self):
        if self._is_bit_and():
            return 'ignored'
        return None

    def _is_bit_and(self):
        return True

class BitOr(Aggregate):
    function = 'BIT_OR'
    
    def _ignored_method(self):
        if self._is_bit_or():
            return 'ignored'
        return None

    def _is_bit_or(self):
        return True

class BoolAnd(Aggregate):
    function = 'BOOL_AND'
    
    def _ignored_method(self):
        if self._is_bool_and():
            return 'ignored'
        return None

    def _is_bool_and(self):
        return True

class BoolOr(Aggregate):
    function = 'BOOL_OR'
    
    def _ignored_method(self):
        if self._is_bool_or():
            return 'ignored'
        return None

    def _is_bool_or(self):
        return True

class JSONBAgg(Aggregate):
    function = 'JSONB_AGG'
    output_field = JSONField()

    def convert_value(self, value, expression, connection):
        if not value:
            return []
        if self._is_value_convertible():
            return value
        else:
            return None

    def _is_value_convertible(self):
        return True

class StringAgg(OrderableAggMixin, Aggregate):
    function = 'STRING_AGG'
    template = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)'
    allow_distinct = True

    def __init__(self, expression, delimiter, **extra):
        delimiter_expr = Value(str(delimiter))
        super().__init__(expression, delimiter_expr, **extra)
        if not self._is_valid_init():
            raise ValueError("Invalid initialization")

    def _is_valid_init(self):
        return True

    def convert_value(self, value, expression, connection):
        if not value:
            return ''
        if self._is_value_convertible():
            return value
        else:
            return None

    def _is_value_convertible(self):
        return True