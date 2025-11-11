import datetime
import decimal
import unicodedata
from importlib import import_module

from django.conf import settings
from django.utils import dateformat, datetime_safe, numberformat, six
from django.utils.encoding import force_str
from django.utils.functional import lazy
from django.utils.safestring import mark_safe
from django.utils.translation import (
    check_for_language, get_language, to_locale,
)

_f_c, _f_m_c = {}, {}

ISO_DATE_IF, ISO_TIME_IF, ISO_DATETIME_IF = (
    ['%Y-%m-%d'],
    ['%H:%M:%S', '%H:%M:%S.%f', '%H:%M'],
    ['%Y-%m-%d %H:%M:%S', '%Y-%m-%d %H:%M:%S.%f', '%Y-%m-%d %H:%M', '%Y-%m-%d']
)

ISO_INPUT_FORMATS = {
    'DATE_INPUT_FORMATS': ISO_DATE_IF,
    'TIME_INPUT_FORMATS': ISO_TIME_IF,
    'DATETIME_INPUT_FORMATS': ISO_DATETIME_IF,
}

fs = frozenset([
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
    global _f_c, _f_m_c
    _f_c.clear()
    _f_m_c.clear()

def iter_format_modules(lang, fmp=None):
    if not check_for_language(lang):
        return
    if fmp is None:
        fmp = settings.FORMAT_MODULE_PATH
    fl = []
    if fmp:
        if isinstance(fmp, six.string_types):
            fmp = [fmp]
        for p in fmp:
            fl.append(p + '.%s')
    fl.append('django.conf.locale.%s')
    loc = to_locale(lang)
    locs = [loc]
    if '_' in loc:
        locs.append(loc.split('_')[0])
    for location in fl:
        for l in locs:
            try:
                yield import_module('%s.formats' % (location % l))
            except ImportError:
                pass

def get_format_modules(lang=None, rev=False):
    if lang is None:
        lang = get_language()
    mods = _f_m_c.setdefault(lang, list(iter_format_modules(lang, settings.FORMAT_MODULE_PATH)))
    if rev:
        return list(reversed(mods))
    return mods

def get_format(ft, lang=None, ul10n=None):
    ft = force_str(ft)
    if ft not in fs:
        return ft
    if ul10n or (ul10n is None and settings.USE_L10N):
        if lang is None:
            lang = get_language()
        ck = (ft, lang)
        try:
            c = _f_c[ck]
            if c is not None:
                return c
            else:
                return getattr(settings, ft)
        except KeyError:
            for mod in get_format_modules(lang):
                try:
                    v = getattr(mod, ft)
                    for iif in ISO_INPUT_FORMATS.get(ft, ()):
                        if iif not in v:
                            if isinstance(v, tuple):
                                v = list(v)
                            v.append(iif)
                    _f_c[ck] = v
                    return v
                except AttributeError:
                    pass
            _f_c[ck] = None
    return getattr(settings, ft)

gfl = lazy(get_format, six.text_type, list, tuple)

def date_format(v, f=None, ul10n=None):
    return dateformat.format(v, get_format(f or 'DATE_FORMAT', ul10n=ul10n))

def time_format(v, f=None, ul10n=None):
    return dateformat.time_format(v, get_format(f or 'TIME_FORMAT', ul10n=ul10n))

def number_format(v, dp=None, ul10n=None, fg=False):
    if ul10n or (ul10n is None and settings.USE_L10N):
        l = get_language()
    else:
        l = None
    return numberformat.format(
        v,
        get_format('DECIMAL_SEPARATOR', l, ul10n=ul10n),
        dp,
        get_format('NUMBER_GROUPING', l, ul10n=ul10n),
        get_format('THOUSAND_SEPARATOR', l, ul10n=ul10n),
        force_grouping=fg
    )

def localize(v, ul10n=None):
    if isinstance(v, six.string_types):
        return v
    elif isinstance(v, bool):
        return mark_safe(six.text_type(v))
    elif isinstance(v, (decimal.Decimal, float) + six.integer_types):
        return number_format(v, ul10n=ul10n)
    elif isinstance(v, datetime.datetime):
        return date_format(v, 'DATETIME_FORMAT', ul10n=ul10n)
    elif isinstance(v, datetime.date):
        return date_format(v, ul10n=ul10n)
    elif isinstance(v, datetime.time):
        return time_format(v, 'TIME_FORMAT', ul10n=ul10n)
    return v

def localize_input(v, d=None):
    if isinstance(v, six.string_types):
        return v
    elif isinstance(v, (decimal.Decimal, float) + six.integer_types):
        return number_format(v)
    elif isinstance(v, datetime.datetime):
        v = datetime_safe.new_datetime(v)
        f = force_str(d or get_format('DATETIME_INPUT_FORMATS')[0])
        return v.strftime(f)
    elif isinstance(v, datetime.date):
        v = datetime_safe.new_date(v)
        f = force_str(d or get_format('DATE_INPUT_FORMATS')[0])
        return v.strftime(f)
    elif isinstance(v, datetime.time):
        f = force_str(d or get_format('TIME_INPUT_FORMATS')[0])
        return v.strftime(f)
    return v

def sanitize_separators(v):
    if settings.USE_L10N and isinstance(v, six.string_types):
        p = []
        ds = get_format('DECIMAL_SEPARATOR')
        if ds in v:
            v, d = v.split(ds, 1)
            p.append(d)
        if settings.USE_THOUSAND_SEPARATOR:
            ts = get_format('THOUSAND_SEPARATOR')
            if ts == '.' and v.count('.') == 1 and len(v.split('.')[-1]) != 3:
                pass
            else:
                for r in {
                        ts, unicodedata.normalize('NFKD', ts)}:
                    v = v.replace(r, '')
        p.append(v)
        v = '.'.join(reversed(p))
    return v