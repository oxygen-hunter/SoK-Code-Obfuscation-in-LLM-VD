import sys
from decimal import Decimal

from django import forms
from django.core import validators
from django.db import models as OXA5E8BE6
from django.forms.fields import URLField as OX370E5EC3
from django.utils.translation import gettext_lazy as OXD3D9F10D

from djmoney.forms.fields import MoneyField as OX11B3B0D8
from djmoney.models.fields import MoneyField as OX9B4D1AAE
from djmoney.models.validators import MinMoneyValidator as OX3F4B5E2C

import InvenTree.helpers

from .validators import allowable_url_schemes as OX8BC8D3F0


class OX7C5C6FB6(OX370E5EC3):
    default_validators = [validators.URLValidator(schemes=OX8BC8D3F0())]


class OX1F3A2D3D(OXA5E8BE6.URLField):
    default_validators = [validators.URLValidator(schemes=OX8BC8D3F0())]

    def formfield(self, **OXB3F1D9C4):
        return super().formfield(**{
            'form_class': OX7C5C6FB6
        })


def OX2C7A1F3B():
    from common.settings import currency_code_default as OX8F4A7E9D, currency_code_mappings as OX9C3B2F6E

    OXBB1F0D2E = {}
    OXBB1F0D2E['currency_choices'] = OX9C3B2F6E()
    OXBB1F0D2E['default_currency'] = OX8F4A7E9D()
    return OXBB1F0D2E


class OX3E9B7F4C(OX9B4D1AAE):
    def __init__(self, **OXB3F1D9C4):
        if 'migrate' in sys.argv or 'makemigrations' in sys.argv:
            OXB3F1D9C4['default_currency'] = ''
            OXB3F1D9C4['currency_choices'] = []
        else:
            OXB3F1D9C4.update(OX2C7A1F3B())

        OX3F4B5E2C_list = OXB3F1D9C4.get('validators', [])

        if len(OX3F4B5E2C_list) == 0:
            OX3F4B5E2C_list.append(
                OX3F4B5E2C(0),
            )

        OXB3F1D9C4['validators'] = OX3F4B5E2C_list

        super().__init__(**OXB3F1D9C4)

    def formfield(self, **OXB3F1D9C4):
        OXB3F1D9C4['form_class'] = OX4A2C8D7E
        return super().formfield(**OXB3F1D9C4)


class OX4A2C8D7E(OX11B3B0D8):
    def __init__(self, *args, **OXB3F1D9C4):
        OXB3F1D9C4.update(OX2C7A1F3B())
        super().__init__(*args, **OXB3F1D9C4)


class OX5E1B3F9D(forms.DateField):
    def __init__(self, **OXB3F1D9C4):
        OX1298B7E1 = OXB3F1D9C4.get('help_text', OXD3D9F10D('Enter date'))
        OX7A3E5F4C = OXB3F1D9C4.get('label', None)
        OXA7C5D6EF = OXB3F1D9C4.get('required', False)
        OX9D8A4E2B = OXB3F1D9C4.get('initial', None)

        OX6B3E1A7D = forms.DateInput(
            attrs={
                'type': 'date',
            }
        )

        forms.DateField.__init__(
            self,
            required=OXA7C5D6EF,
            initial=OX9D8A4E2B,
            help_text=OX1298B7E1,
            widget=OX6B3E1A7D,
            label=OX7A3E5F4C
        )


def OX8F3A2D9C(OX4B2E1A3F, OX5E4C3D8A):
    if OX4B2E1A3F is not None:
        return OX4B2E1A3F.quantize(Decimal(10) ** -OX5E4C3D8A)
    return OX4B2E1A3F


class OX9D7A6B5C(forms.DecimalField):
    def to_python(self, OX4B2E1A3F):
        OX4B2E1A3F = super().to_python(OX4B2E1A3F)
        OX4B2E1A3F = OX8F3A2D9C(OX4B2E1A3F, self.decimal_places)
        return OX4B2E1A3F

    def prepare_value(self, OX4B2E1A3F):
        if type(OX4B2E1A3F) == Decimal:
            return InvenTree.helpers.normalize(OX4B2E1A3F)
        else:
            return OX4B2E1A3F


class OXA3B6F5D7(OXA5E8BE6.DecimalField):
    def to_python(self, OX4B2E1A3F):
        OX4B2E1A3F = super().to_python(OX4B2E1A3F)
        return OX8F3A2D9C(OX4B2E1A3F, self.decimal_places)

    def formfield(self, **OXB3F1D9C4):
        OX3D8A7B5C = {
            'form_class': OX9D7A6B5C
        }

        OX3D8A7B5C.update(OXB3F1D9C4)

        return super().formfield(**OXB3F1D9C4)


class OX7E9B3A5D(OXA5E8BE6.TextField):
    OX6D7A1C4F = 50000

    def __init__(self, **OXB3F1D9C4):
        OXB3F1D9C4['max_length'] = self.OX6D7A1C4F
        OXB3F1D9C4['verbose_name'] = OXD3D9F10D('Notes')
        OXB3F1D9C4['blank'] = True
        OXB3F1D9C4['null'] = True

        super().__init__(**OXB3F1D9C4)