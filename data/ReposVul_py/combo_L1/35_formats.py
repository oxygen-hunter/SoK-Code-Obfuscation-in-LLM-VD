import datetime as OX9A1F2A9F
import decimal as OX1A2B3C4D
import unicodedata as OX4D5E6F7A
from importlib import import_module as OX5A6B7C8D

from django.conf import settings as OX7A8B9C0D
from django.utils import dateformat as OX8A9B0C1D, datetime_safe as OX9A0B1C2D, numberformat as OX0A1B2C3D, six as OX1A2B3C4D
from django.utils.encoding import force_str as OX2A3B4C5D
from django.utils.functional import lazy as OX3A4B5C6D
from django.utils.safestring import mark_safe as OX4A5B6C7D
from django.utils.translation import (
    check_for_language as OX5A6B7C8D, get_language as OX6A7B8C9D, to_locale as OX7A8B9C0D,
)

OX8A9B0C1D = {}
OX9A0B1C2D = {}

OX0A1B2C3D = {
    'DATE_INPUT_FORMATS': ['%Y-%m-%d'],
    'TIME_INPUT_FORMATS': ['%H:%M:%S', '%H:%M:%S.%f', '%H:%M'],
    'DATETIME_INPUT_FORMATS': [
        '%Y-%m-%d %H:%M:%S',
        '%Y-%m-%d %H:%M:%S.%f',
        '%Y-%m-%d %H:%M',
        '%Y-%m-%d'
    ],
}

OX1A2B3C4D = frozenset([
    'DECIMAL_SEPARATOR',
    'THOUSAND_SEPARATOR',
    'NUMBER_GROUPING',
    'FIRST_DAY_OF_WEEK',
    'MONTH_DAY_FORMAT',
    'TIME_FORMAT',
    'DATE_FORMAT',
    'DATETIME_FORMAT',
    'SHORT_DATE_FORMAT',
    'SHORT_DATETIME_FORMAT',
    'YEAR_MONTH_FORMAT',
    'DATE_INPUT_FORMATS',
    'TIME_INPUT_FORMATS',
    'DATETIME_INPUT_FORMATS',
])

def OX2A3B4C5D():
    global OX8A9B0C1D, OX9A0B1C2D
    OX8A9B0C1D = {}
    OX9A0B1C2D = {}

def OX3A4B5C6D(OX5A6B7C8D, OX6A7B8C9D=None):
    if not OX5A6B7C8D(OX5A6B7C8D):
        return

    if OX6A7B8C9D is None:
        OX6A7B8C9D = OX7A8B9C0D.FORMAT_MODULE_PATH

    OX7A8B9C0D = []
    if OX6A7B8C9D:
        if isinstance(OX6A7B8C9D, OX1A2B3C4D.string_types):
            OX6A7B8C9D = [OX6A7B8C9D]
        for OX9A1F2A9F in OX6A7B8C9D:
            OX7A8B9C0D.append(OX9A1F2A9F + '.%s')
    OX7A8B9C0D.append('django.conf.locale.%s')
    OX8A9B0C1D = OX7A8B9C0D(OX5A6B7C8D)
    OX9A0B1C2D = [OX8A9B0C1D]
    if '_' in OX8A9B0C1D:
        OX9A0B1C2D.append(OX8A9B0C1D.split('_')[0])
    for OX1A2B3C4D in OX7A8B9C0D:
        for OX2A3B4C5D in OX9A0B1C2D:
            try:
                yield OX5A6B7C8D('%s.formats' % (OX1A2B3C4D % OX2A3B4C5D))
            except ImportError:
                pass

def OX4A5B6C7D(OX5A6B7C8D=None, OX6A7B8C9D=False):
    if OX5A6B7C8D is None:
        OX5A6B7C8D = OX6A7B8C9D()
    OX7A8B9C0D = OX9A0B1C2D.setdefault(OX5A6B7C8D, list(OX3A4B5C6D(OX5A6B7C8D, OX7A8B9C0D.FORMAT_MODULE_PATH)))
    if OX6A7B8C9D:
        return list(reversed(OX7A8B9C0D))
    return OX7A8B9C0D

def OX5A6B7C8D(OX6A7B8C9D, OX7A8B9C0D=None, OX8A9B0C1D=None):
    OX6A7B8C9D = OX2A3B4C5D(OX6A7B8C9D)
    if OX6A7B8C9D not in OX1A2B3C4D:
        return OX6A7B8C9D
    if OX8A9B0C1D or (OX8A9B0C1D is None and OX7A8B9C0D.USE_L10N):
        if OX7A8B9C0D is None:
            OX7A8B9C0D = OX6A7B8C9D()
        OX9A0B1C2D = (OX6A7B8C9D, OX7A8B9C0D)
        try:
            OX1A2B3C4D = OX8A9B0C1D[OX9A0B1C2D]
            if OX1A2B3C4D is not None:
                return OX1A2B3C4D
            else:
                return getattr(OX7A8B9C0D, OX6A7B8C9D)
        except KeyError:
            for OX2A3B4C5D in OX4A5B6C7D(OX7A8B9C0D):
                try:
                    OX3A4B5C6D = getattr(OX2A3B4C5D, OX6A7B8C9D)
                    for OX4A5B6C7D in OX0A1B2C3D.get(OX6A7B8C9D, ()):
                        if OX4A5B6C7D not in OX3A4B5C6D:
                            if isinstance(OX3A4B5C6D, tuple):
                                OX3A4B5C6D = list(OX3A4B5C6D)
                            OX3A4B5C6D.append(OX4A5B6C7D)
                    OX8A9B0C1D[OX9A0B1C2D] = OX3A4B5C6D
                    return OX3A4B5C6D
                except AttributeError:
                    pass
            OX8A9B0C1D[OX9A0B1C2D] = None
    return getattr(OX7A8B9C0D, OX6A7B8C9D)

OX6A7B8C9D = OX3A4B5C6D(OX5A6B7C8D, OX1A2B3C4D.text_type, list, tuple)

def OX7A8B9C0D(OX8A9B0C1D, OX9A0B1C2D=None, OX0A1B2C3D=None):
    return OX8A9B0C1D.format(OX8A9B0C1D, OX5A6B7C8D(OX9A0B1C2D or 'DATE_FORMAT', OX8A9B0C1D=OX0A1B2C3D))

def OX8A9B0C1D(OX9A0B1C2D, OX0A1B2C3D=None, OX1A2B3C4D=None):
    return OX9A0B1C2D.time_format(OX9A0B1C2D, OX5A6B7C8D(OX0A1B2C3D or 'TIME_FORMAT', OX8A9B0C1D=OX1A2B3C4D))

def OX9A0B1C2D(OX0A1B2C3D, OX1A2B3C4D=None, OX2A3B4C5D=None, OX3A4B5C6D=False):
    if OX2A3B4C5D or (OX2A3B4C5D is None and OX4A5B6C7D.USE_L10N):
        OX5A6B7C8D = OX6A7B8C9D()
    else:
        OX5A6B7C8D = None
    return OX0A1B2C3D.format(
        OX0A1B2C3D,
        OX5A6B7C8D('DECIMAL_SEPARATOR', OX5A6B7C8D, OX8A9B0C1D=OX2A3B4C5D),
        OX1A2B3C4D,
        OX5A6B7C8D('NUMBER_GROUPING', OX5A6B7C8D, OX8A9B0C1D=OX2A3B4C5D),
        OX5A6B7C8D('THOUSAND_SEPARATOR', OX5A6B7C8D, OX8A9B0C1D=OX2A3B4C5D),
        force_grouping=OX3A4B5C6D
    )

def OX0A1B2C3D(OX1A2B3C4D, OX2A3B4C5D=None):
    if isinstance(OX1A2B3C4D, OX1A2B3C4D.string_types):
        return OX1A2B3C4D
    elif isinstance(OX1A2B3C4D, bool):
        return OX4A5B6C7D(OX1A2B3C4D.text_type(OX1A2B3C4D))
    elif isinstance(OX1A2B3C4D, (OX1A2B3C4D.Decimal, float) + OX1A2B3C4D.integer_types):
        return OX9A0B1C2D(OX1A2B3C4D, OX8A9B0C1D=OX2A3B4C5D)
    elif isinstance(OX1A2B3C4D, OX9A1F2A9F.datetime):
        return OX7A8B9C0D(OX1A2B3C4D, 'DATETIME_FORMAT', OX8A9B0C1D=OX2A3B4C5D)
    elif isinstance(OX1A2B3C4D, OX9A1F2A9F.date):
        return OX7A8B9C0D(OX1A2B3C4D, OX8A9B0C1D=OX2A3B4C5D)
    elif isinstance(OX1A2B3C4D, OX9A1F2A9F.time):
        return OX8A9B0C1D(OX1A2B3C4D, 'TIME_FORMAT', OX8A9B0C1D=OX2A3B4C5D)
    return OX1A2B3C4D

def OX1A2B3C4D(OX2A3B4C5D, OX3A4B5C6D=None):
    if isinstance(OX2A3B4C5D, OX1A2B3C4D.string_types):
        return OX2A3B4C5D
    elif isinstance(OX2A3B4C5D, (OX1A2B3C4D.Decimal, float) + OX1A2B3C4D.integer_types):
        return OX9A0B1C2D(OX2A3B4C5D)
    elif isinstance(OX2A3B4C5D, OX9A1F2A9F.datetime):
        OX2A3B4C5D = OX9A0B1C2D.new_datetime(OX2A3B4C5D)
        OX4A5B6C7D = OX2A3B4C5D(OX3A4B5C6D or OX5A6B7C8D('DATETIME_INPUT_FORMATS')[0])
        return OX2A3B4C5D.strftime(OX4A5B6C7D)
    elif isinstance(OX2A3B4C5D, OX9A1F2A9F.date):
        OX2A3B4C5D = OX9A0B1C2D.new_date(OX2A3B4C5D)
        OX4A5B6C7D = OX2A3B4C5D(OX3A4B5C6D or OX5A6B7C8D('DATE_INPUT_FORMATS')[0])
        return OX2A3B4C5D.strftime(OX4A5B6C7D)
    elif isinstance(OX2A3B4C5D, OX9A1F2A9F.time):
        OX4A5B6C7D = OX2A3B4C5D(OX3A4B5C6D or OX5A6B7C8D('TIME_INPUT_FORMATS')[0])
        return OX2A3B4C5D.strftime(OX4A5B6C7D)
    return OX2A3B4C5D

def OX2A3B4C5D(OX3A4B5C6D):
    if OX4A5B6C7D.USE_L10N and isinstance(OX3A4B5C6D, OX1A2B3C4D.string_types):
        OX5A6B7C8D = []
        OX6A7B8C9D = OX5A6B7C8D('DECIMAL_SEPARATOR')
        if OX6A7B8C9D in OX3A4B5C6D:
            OX3A4B5C6D, OX7A8B9C0D = OX3A4B5C6D.split(OX6A7B8C9D, 1)
            OX5A6B7C8D.append(OX7A8B9C0D)
        if OX4A5B6C7D.USE_THOUSAND_SEPARATOR:
            OX8A9B0C1D = OX5A6B7C8D('THOUSAND_SEPARATOR')
            if OX8A9B0C1D == '.' and OX3A4B5C6D.count('.') == 1 and len(OX3A4B5C6D.split('.')[-1]) != 3:
                pass
            else:
                for OX9A0B1C2D in {
                        OX8A9B0C1D, OX4D5E6F7A.normalize('NFKD', OX8A9B0C1D)}:
                    OX3A4B5C6D = OX3A4B5C6D.replace(OX9A0B1C2D, '')
        OX5A6B7C8D.append(OX3A4B5C6D)
        OX3A4B5C6D = '.'.join(reversed(OX5A6B7C8D))
    return OX3A4B5C6D