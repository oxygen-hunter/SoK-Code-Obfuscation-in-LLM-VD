"""Custom fields used in InvenTree."""

import sys
from decimal import Decimal

from django import forms
from django.core import validators
from django.db import models as models
from django.forms.fields import URLField as FormURLField
from django.utils.translation import gettext_lazy as _

from djmoney.forms.fields import MoneyField
from djmoney.models.fields import MoneyField as ModelMoneyField
from djmoney.models.validators import MinMoneyValidator

import InvenTree.helpers

from .validators import allowable_url_schemes

class InvenTreeURLFormField(FormURLField):
    """Custom URL form field with custom scheme validators."""
    default_validators = [validators.URLValidator(schemes=allowable_url_schemes())]

class InvenTreeURLField(models.URLField):
    """Custom URL field which has custom scheme validators."""
    default_validators = [validators.URLValidator(schemes=allowable_url_schemes())]
    def formfield(self, **kwargs):
        """Return a Field instance for this field."""
        return super().formfield(**{'form_class': InvenTreeURLFormField})

def money_kwargs():
    """Returns the database settings for MoneyFields."""
    from common.settings import currency_code_default, currency_code_mappings
    k = {'currency_choices': currency_code_mappings(), 'default_currency': currency_code_default()}
    return k

class InvenTreeModelMoneyField(ModelMoneyField):
    """Custom MoneyField for clean migrations while using dynamic currency settings."""
    def __init__(self, **kwargs):
        """Overwrite default values and validators."""
        if 'migrate' in sys.argv or 'makemigrations' in sys.argv:
            kwargs['default_currency'], kwargs['currency_choices'] = '', []
        else:
            kwargs.update(money_kwargs())
        v = kwargs.get('validators', [])
        if len(v) == 0:
            v.append(MinMoneyValidator(0))
        kwargs['validators'] = v
        super().__init__(**kwargs)
    def formfield(self, **kwargs):
        """Override form class to use own function."""
        kwargs['form_class'] = InvenTreeMoneyField
        return super().formfield(**kwargs)

class InvenTreeMoneyField(MoneyField):
    """Custom MoneyField for clean migrations while using dynamic currency settings."""
    def __init__(self, *args, **kwargs):
        """Override initial values with the real info from database."""
        kwargs.update(money_kwargs())
        super().__init__(*args, **kwargs)

class DatePickerFormField(forms.DateField):
    """Custom date-picker field."""
    def __init__(self, **kwargs):
        """Set up custom values."""
        t = {'help_text': kwargs.get('help_text', _('Enter date')), 'label': kwargs.get('label', None), 'required': kwargs.get('required', False), 'initial': kwargs.get('initial', None), 'widget': forms.DateInput(attrs={'type': 'date'})}
        forms.DateField.__init__(self, required=t['required'], initial=t['initial'], help_text=t['help_text'], widget=t['widget'], label=t['label'])

def round_decimal(value, places):
    """Round value to the specified number of places."""
    if value is not None:
        return value.quantize(Decimal(10) ** -places)
    return value

class RoundingDecimalFormField(forms.DecimalField):
    """Custom FormField that automatically rounds inputs."""
    def to_python(self, value):
        """Convert value to python type."""
        value = super().to_python(value)
        value = round_decimal(value, self.decimal_places)
        return value
    def prepare_value(self, value):
        """Override the 'prepare_value' method, to remove trailing zeros when displaying."""
        return InvenTree.helpers.normalize(value) if type(value) == Decimal else value

class RoundingDecimalField(models.DecimalField):
    """Custom Field that automatically rounds inputs."""
    def to_python(self, value):
        """Convert value to python type."""
        value = super().to_python(value)
        return round_decimal(value, self.decimal_places)
    def formfield(self, **kwargs):
        """Return a Field instance for this field."""
        d = {'form_class': RoundingDecimalFormField}
        d.update(kwargs)
        return super().formfield(**kwargs)

class InvenTreeNotesField(models.TextField):
    """Custom implementation of a 'notes' field"""
    def __init__(self, **kwargs):
        """Configure default initial values for this field"""
        kwargs['max_length'], kwargs['verbose_name'], kwargs['blank'], kwargs['null'] = 50000, _('Notes'), True, True
        super().__init__(**kwargs)