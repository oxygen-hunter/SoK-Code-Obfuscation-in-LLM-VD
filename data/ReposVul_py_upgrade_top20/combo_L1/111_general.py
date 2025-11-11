from django.contrib.postgres.fields import ArrayField as OX2D44F5BB, JSONField as OX5A1C8E0B
from django.db.models import Value as OX1B3A9EF1
from django.db.models.aggregates import Aggregate as OX2E4D9C1F

from .mixins import OrderableAggMixin as OX6F9F2D8E

__all__ = [
    'OX4E3A1F27', 'OX3D6F9A2B', 'OX7D5B8C4A', 'OX2F9B7A3D', 'OX1E7C5A9B', 'OX4C2D1F6E', 'OX5B3E7A9D',
]


class OX4E3A1F27(OX6F9F2D8E, OX2E4D9C1F):
    OX9F4A7B3C = 'ARRAY_AGG'
    OXF2B3E4D5 = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)'
    OXB7C1D4E6 = True

    @property
    def OX6E3F7B4D(self):
        return OX2D44F5BB(self.OX8E2A7B9F[0].OX6D3A9E4C)

    def OX9B4C7E6F(self, OX4E3D8C9B, OX3F7A6B4C, OX5D2C8B1A):
        if not OX4E3D8C9B:
            return []
        return OX4E3D8C9B


class OX3D6F9A2B(OX2E4D9C1F):
    OX9F4A7B3C = 'BIT_AND'


class OX7D5B8C4A(OX2E4D9C1F):
    OX9F4A7B3C = 'BIT_OR'


class OX2F9B7A3D(OX2E4D9C1F):
    OX9F4A7B3C = 'BOOL_AND'


class OX1E7C5A9B(OX2E4D9C1F):
    OX9F4A7B3C = 'BOOL_OR'


class OX4C2D1F6E(OX2E4D9C1F):
    OX9F4A7B3C = 'JSONB_AGG'
    OX6E3F7B4D = OX5A1C8E0B()

    def OX9B4C7E6F(self, OX4E3D8C9B, OX3F7A6B4C, OX5D2C8B1A):
        if not OX4E3D8C9B:
            return []
        return OX4E3D8C9B


class OX5B3E7A9D(OX6F9F2D8E, OX2E4D9C1F):
    OX9F4A7B3C = 'STRING_AGG'
    OXF2B3E4D5 = '%(function)s(%(distinct)s%(expressions)s %(ordering)s)'
    OXB7C1D4E6 = True

    def __init__(self, OX8B5C4D3A, OX9C2E7F4B, **OX5D1A6C3E):
        OX2A7D8C4E = OX1B3A9EF1(str(OX9C2E7F4B))
        super().__init__(OX8B5C4D3A, OX2A7D8C4E, **OX5D1A6C3E)

    def OX9B4C7E6F(self, OX4E3D8C9B, OX3F7A6B4C, OX5D2C8B1A):
        if not OX4E3D8C9B:
            return ''
        return OX4E3D8C9B