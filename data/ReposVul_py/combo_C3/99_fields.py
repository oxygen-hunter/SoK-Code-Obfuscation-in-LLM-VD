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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self):
        while self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            self.pc += 1
            getattr(self, op)(*args)

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        return self.stack.pop()

    def ADD(self):
        a = self.POP()
        b = self.POP()
        self.PUSH(a + b)

    def SUB(self):
        a = self.POP()
        b = self.POP()
        self.PUSH(a - b)

    def JMP(self, address):
        self.pc = address

    def JZ(self, address):
        if self.POP() == 0:
            self.pc = address

    def LOAD(self, index):
        self.PUSH(self.stack[index])

    def STORE(self, index):
        self.stack[index] = self.POP()

def compile_instructions():
    vm = VM()
    vm.instructions = [
        ('LOAD', 0),
        ('PUSH', 10),
        ('SUB',),
        ('JZ', 10),
        ('PUSH', 1),
        ('ADD',),
        ('STORE', 0),
        ('JMP', 0),
        ('PUSH', 'Done'),
        ('POP',),
    ]
    vm.stack = [100]
    vm.run()

class InvenTreeURLFormField(FormURLField):
    default_validators = [validators.URLValidator(schemes=allowable_url_schemes())]

class InvenTreeURLField(models.URLField):
    default_validators = [validators.URLValidator(schemes=allowable_url_schemes())]

    def formfield(self, **kwargs):
        return super().formfield(**{
            'form_class': InvenTreeURLFormField
        })

def money_kwargs():
    from common.settings import currency_code_default, currency_code_mappings
    kwargs = {}
    kwargs['currency_choices'] = currency_code_mappings()
    kwargs['default_currency'] = currency_code_default()
    return kwargs

class InvenTreeModelMoneyField(ModelMoneyField):
    def __init__(self, **kwargs):
        if 'migrate' in sys.argv or 'makemigrations' in sys.argv:
            kwargs['default_currency'] = ''
            kwargs['currency_choices'] = []
        else:
            kwargs.update(money_kwargs())

        validators = kwargs.get('validators', [])
        if len(validators) == 0:
            validators.append(MinMoneyValidator(0))
        kwargs['validators'] = validators

        super().__init__(**kwargs)

    def formfield(self, **kwargs):
        kwargs['form_class'] = InvenTreeMoneyField
        return super().formfield(**kwargs)

class InvenTreeMoneyField(MoneyField):
    def __init__(self, *args, **kwargs):
        kwargs.update(money_kwargs())
        super().__init__(*args, **kwargs)

class DatePickerFormField(forms.DateField):
    def __init__(self, **kwargs):
        help_text = kwargs.get('help_text', _('Enter date'))
        label = kwargs.get('label', None)
        required = kwargs.get('required', False)
        initial = kwargs.get('initial', None)

        widget = forms.DateInput(attrs={'type': 'date'})

        forms.DateField.__init__(self, required=required, initial=initial, help_text=help_text, widget=widget, label=label)

def round_decimal(value, places):
    if value is not None:
        return value.quantize(Decimal(10) ** -places)
    return value

class RoundingDecimalFormField(forms.DecimalField):
    def to_python(self, value):
        value = super().to_python(value)
        value = round_decimal(value, self.decimal_places)
        return value

    def prepare_value(self, value):
        if type(value) == Decimal:
            return InvenTree.helpers.normalize(value)
        else:
            return value

class RoundingDecimalField(models.DecimalField):
    def to_python(self, value):
        value = super().to_python(value)
        return round_decimal(value, self.decimal_places)

    def formfield(self, **kwargs):
        defaults = {'form_class': RoundingDecimalFormField}
        defaults.update(kwargs)
        return super().formfield(**kwargs)

class InvenTreeNotesField(models.TextField):
    NOTES_MAX_LENGTH = 50000

    def __init__(self, **kwargs):
        kwargs['max_length'] = self.NOTES_MAX_LENGTH
        kwargs['verbose_name'] = _('Notes')
        kwargs['blank'] = True
        kwargs['null'] = True
        super().__init__(**kwargs)