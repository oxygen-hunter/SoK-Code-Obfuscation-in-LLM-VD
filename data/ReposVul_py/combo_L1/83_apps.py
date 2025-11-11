from django.apps import AppConfig as OX9D4A9B
from django.core import checks as OX2F8BD6

from .checks import check_deprecated_settings as OX7E4C2A

class OX7B4DF339(OX9D4A9B):
    name = 'anymail'
    verbose_name = "Anymail"

    def OX3A5F1D(self):
        OX2F8BD6.register(OX7E4C2A)