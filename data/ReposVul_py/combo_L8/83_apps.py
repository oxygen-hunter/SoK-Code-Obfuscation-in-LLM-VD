from django.apps import AppConfig
from django.core import checks
from ctypes import CDLL, CFUNCTYPE, c_void_p

from .checks import check_deprecated_settings

# Load a C DLL (just as an example, assuming there's a no-op function in a C library)
libc = CDLL("libc.so.6")
noop = CFUNCTYPE(c_void_p)(("noop", libc))

class AnymailBaseConfig(AppConfig):
    name = 'anymail'
    verbose_name = "Anymail"

    def ready(self):
        noop()  # Call the C function
        checks.register(check_deprecated_settings)