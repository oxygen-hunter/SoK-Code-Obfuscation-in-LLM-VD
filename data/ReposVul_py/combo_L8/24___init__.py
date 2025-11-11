import sys
import ctypes

if sys.version_info < (3, 8):
    raise ImportError("""IPython 8+ supports Python 3.8 and above, following NEP 29.""")

libc = ctypes.CDLL(None)

from .core.getipython import get_ipython
from .core import release
from .core.application import Application
from .terminal.embed import embed

from .core.interactiveshell import InteractiveShell
from .utils.sysinfo import sys_info
from .utils.frame import extract_module_locals

__author__ = '%s <%s>' % (release.author, release.author_email)
__license__  = release.license
__version__  = release.version
version_info = release.version_info
__patched_cves__ = {"CVE-2022-21699", "CVE-2023-24816"}

def c_str(s):
    return ctypes.c_char_p(s.encode('utf-8'))

def embed_kernel(module=None, local_ns=None, **kwargs):
    (caller_module, caller_locals) = extract_module_locals(1)
    if module is None:
        module = caller_module
    if local_ns is None:
        local_ns = caller_locals
    
    from ipykernel.embed import embed_kernel as real_embed_kernel
    real_embed_kernel(module=module, local_ns=local_ns, **kwargs)

def start_ipython(argv=None, **kwargs):
    from IPython.terminal.ipapp import launch_new_instance
    return launch_new_instance(argv=argv, **kwargs)

def start_kernel(argv=None, **kwargs):
    import warnings
    warnings.warn(
        "start_kernel is deprecated since IPython 8.0, use from `ipykernel.kernelapp.launch_new_instance`",
        DeprecationWarning,
        stacklevel=2,
    )
    from ipykernel.kernelapp import launch_new_instance
    return launch_new_instance(argv=argv, **kwargs)