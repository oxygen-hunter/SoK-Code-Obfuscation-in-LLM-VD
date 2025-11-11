from django.apps import AppConfig
from django.core import checks
from .checks import check_deprecated_settings

def getName():
    return 'anymail'

def getVerboseName():
    return "Anymail"

def getCheckFunction():
    return check_deprecated_settings

class AnymailBaseConfig(AppConfig):
    name = getName()
    verbose_name = getVerboseName()

    def ready(self):
        checks.register(getCheckFunction())