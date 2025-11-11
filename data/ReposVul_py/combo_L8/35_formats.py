import datetime
import decimal
import unicodedata
from importlib import import_module
from ctypes import CDLL, c_char_p, c_int, c_void_p

from django.conf import settings
from django.utils import dateformat, datetime_safe, numberformat, six
from django.utils.encoding import force_str
from django.utils.functional import lazy
from django.utils.safestring import mark_safe
from django.utils.translation import (
    check_for_language, get_language, to_locale,
)

_format_cache = {}
_format_modules_cache = {}

ISO_INPUT_FORMATS = {
    'DATE_INPUT_FORMATS': ['%Y-%m-%d'],
    'TIME_INPUT_FORMATS': ['%H:%M:%S', '%H:%M:%S.%f', '%H:%M'],
    'DATETIME_INPUT_FORMATS': [
        '%Y-%m-%d %H:%M:%S',
        '%Y-%m-%d %H:%M:%S.%f',
        '%Y-%m-%d %H:%M',
        '%Y-%m-%d'
    ],
}


FORMAT_SETTINGS = frozenset([
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

def reset_format_cache():
    global _format_cache, _format_modules_cache
    _format_cache = {}
    _format_modules_cache = {}

def iter_format_modules(lang, format_module_path=None):
    if not check_for_language(lang):
        return

    if format_module_path is None:
        format_module_path = settings.FORMAT_MODULE_PATH

    format_locations = []
    if format_module_path:
        if isinstance(format_module_path, six.string_types):
            format_module_path = [format_module_path]
        for path in format_module_path:
            format_locations.append(path + '.%s')
    format_locations.append('django.conf.locale.%s')
    locale = to_locale(lang)
    locales = [locale]
    if '_' in locale:
        locales.append(locale.split('_')[0])
    for location in format_locations:
        for loc in locales:
            try:
                yield import_module('%s.formats' % (location % loc))
            except ImportError:
                pass

def get_format_modules(lang=None, reverse=False):
    if lang is None:
        lang = get_language()
    modules = _format_modules_cache.setdefault(lang, list(iter_format_modules(lang, settings.FORMAT_MODULE_PATH)))
    if reverse:
        return list(reversed(modules))
    return modules

def get_format(format_type, lang=None, use_l10n=None):
    format_type = force_str(format_type)
    if format_type not in FORMAT_SETTINGS:
        return format_type
    if use_l10n or (use_l10n is None and settings.USE_L10N):
        if lang is None:
            lang = get_language()
        cache_key = (format_type, lang)
        try:
            cached = _format_cache[cache_key]
            if cached is not None:
                return cached
            else:
                return getattr(settings, format_type)
        except KeyError:
            for module in get_format_modules(lang):
                try:
                    val = getattr(module, format_type)
                    for iso_input in ISO_INPUT_FORMATS.get(format_type, ()):
                        if iso_input not in val:
                            if isinstance(val, tuple):
                                val = list(val)
                            val.append(iso_input)
                    _format_cache[cache_key] = val
                    return val
                except AttributeError:
                    pass
            _format_cache[cache_key] = None
    return getattr(settings, format_type)

get_format_lazy = lazy(get_format, six.text_type, list, tuple)

c_lib = CDLL('path/to/your/clib.so')

def call_c_function(value):
    c_lib.your_function.argtypes = [c_void_p, c_char_p]
    c_lib.your_function.restype = c_int
    return c_lib.your_function(c_void_p(id(value)), c_char_p(value.encode('utf-8')))

def date_format(value, format=None, use_l10n=None):
    return dateformat.format(value, get_format(format or 'DATE_FORMAT', use_l10n=use_l10n))

def time_format(value, format=None, use_l10n=None):
    return dateformat.time_format(value, get_format(format or 'TIME_FORMAT', use_l10n=use_l10n))

def number_format(value, decimal_pos=None, use_l10n=None, force_grouping=False):
    if use_l10n or (use_l10n is None and settings.USE_L10N):
        lang = get_language()
    else:
        lang = None
    return numberformat.format(
        value,
        get_format('DECIMAL_SEPARATOR', lang, use_l10n=use_l10n),
        decimal_pos,
        get_format('NUMBER_GROUPING', lang, use_l10n=use_l10n),
        get_format('THOUSAND_SEPARATOR', lang, use_l10n=use_l10n),
        force_grouping=force_grouping
    )

def localize(value, use_l10n=None):
    if isinstance(value, six.string_types):
        return value
    elif isinstance(value, bool):
        return mark_safe(six.text_type(value))
    elif isinstance(value, (decimal.Decimal, float) + six.integer_types):
        return number_format(value, use_l10n=use_l10n)
    elif isinstance(value, datetime.datetime):
        return date_format(value, 'DATETIME_FORMAT', use_l10n=use_l10n)
    elif isinstance(value, datetime.date):
        return date_format(value, use_l10n=use_l10n)
    elif isinstance(value, datetime.time):
        return time_format(value, 'TIME_FORMAT', use_l10n=use_l10n)
    return value

def localize_input(value, default=None):
    if isinstance(value, six.string_types):
        return value
    elif isinstance(value, (decimal.Decimal, float) + six.integer_types):
        return number_format(value)
    elif isinstance(value, datetime.datetime):
        value = datetime_safe.new_datetime(value)
        format = force_str(default or get_format('DATETIME_INPUT_FORMATS')[0])
        return value.strftime(format)
    elif isinstance(value, datetime.date):
        value = datetime_safe.new_date(value)
        format = force_str(default or get_format('DATE_INPUT_FORMATS')[0])
        return value.strftime(format)
    elif isinstance(value, datetime.time):
        format = force_str(default or get_format('TIME_INPUT_FORMATS')[0])
        return value.strftime(format)
    return value

def sanitize_separators(value):
    if settings.USE_L10N and isinstance(value, six.string_types):
        parts = []
        decimal_separator = get_format('DECIMAL_SEPARATOR')
        if decimal_separator in value:
            value, decimals = value.split(decimal_separator, 1)
            parts.append(decimals)
        if settings.USE_THOUSAND_SEPARATOR:
            thousand_sep = get_format('THOUSAND_SEPARATOR')
            if thousand_sep == '.' and value.count('.') == 1 and len(value.split('.')[-1]) != 3:
                pass
            else:
                for replacement in {
                        thousand_sep, unicodedata.normalize('NFKD', thousand_sep)}:
                    value = value.replace(replacement, '')
        parts.append(value)
        value = '.'.join(reversed(parts))
    return value