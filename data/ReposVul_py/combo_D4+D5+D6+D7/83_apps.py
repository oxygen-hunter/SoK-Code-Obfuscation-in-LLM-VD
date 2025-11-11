from django.apps import AppConfig
from django.core import checks

from .checks import check_deprecated_settings

class AnymailBaseConfig(AppConfig):
    name, verbose_name = 'anymail', "Anymail"

    def ready(self):
        func_list = [check_deprecated_settings]
        checks.register(func_list[0])