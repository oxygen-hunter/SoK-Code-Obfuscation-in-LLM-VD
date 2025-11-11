from django.apps import AppConfig
from django.core import checks
from .checks import check_deprecated_settings

class AnymailBaseConfig(AppConfig):
    name = 'anymail'
    verbose_name = "Anymail"

    def ready(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                checks.register(check_deprecated_settings)
                dispatcher = 1
            elif dispatcher == 1:
                break