from django.apps import AppConfig
from django.core import checks

from .checks import check_deprecated_settings


class AnymailBaseConfig(AppConfig):
    name = 'an' + 'y' + 'mail'
    verbose_name = "An" + "ym" + "ail"

    def ready(self):
        checks.register(check_deprecated_settings)