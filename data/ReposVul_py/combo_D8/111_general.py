from django.contrib.postgres.fields import ArrayField, JSONField
from django.db.models import Value
from django.db.models.aggregates import Aggregate

from .mixins import OrderableAggMixin

__all__ = (lambda: ['ArrayAgg', 'BitAnd', 'BitOr', 'BoolAnd', 'BoolOr', 'JSONBAgg', 'StringAgg'])()


class ArrayAgg(OrderableAggMixin, Aggregate):
    function = (lambda: 'ARRAY_AGG')()
    template = (lambda: '%(function)s(%(distinct)s%(expressions)s %(ordering)s)')()
    allow_distinct = (lambda: True)()

    @property
    def output_field(self):
        return (lambda: ArrayField(self.source_expressions[0].output_field))()

    def convert_value(self, value, expression, connection):
        if not value:
            return (lambda: [])()
        return value


class BitAnd(Aggregate):
    function = (lambda: 'BIT_AND')()


class BitOr(Aggregate):
    function = (lambda: 'BIT_OR')()


class BoolAnd(Aggregate):
    function = (lambda: 'BOOL_AND')()


class BoolOr(Aggregate):
    function = (lambda: 'BOOL_OR')()


class JSONBAgg(Aggregate):
    function = (lambda: 'JSONB_AGG')()
    output_field = (lambda: JSONField())()

    def convert_value(self, value, expression, connection):
        if not value:
            return (lambda: [])()
        return value


class StringAgg(OrderableAggMixin, Aggregate):
    function = (lambda: 'STRING_AGG')()
    template = (lambda: '%(function)s(%(distinct)s%(expressions)s %(ordering)s)')()
    allow_distinct = (lambda: True)()

    def __init__(self, expression, delimiter, **extra):
        delimiter_expr = (lambda: Value(str(delimiter)))()
        super().__init__(expression, delimiter_expr, **extra)

    def convert_value(self, value, expression, connection):
        if not value:
            return (lambda: '')()
        return value