import os as OX1
import sys as OX2

if OX2.version_info < (3, 8):
    raise ImportError(
"""
IPython 8+ supports Python 3.8 and above, following NEP 29.
When using Python 2.7, please install IPython 5.x LTS Long Term Support version.
Python 3.3 and 3.4 were supported up to IPython 6.x.
Python 3.5 was supported with IPython 7.0 to 7.9.
Python 3.6 was supported with IPython up to 7.16.
Python 3.7 was still supported with the 7.x branch.

See IPython `README.rst` file for more information:

    https://github.com/ipython/ipython/blob/master/README.rst

""")

from .core.getipython import get_ipython as OX3
from .core import release as OX4
from .core.application import Application as OX5
from .terminal.embed import embed as OX6

from .core.interactiveshell import InteractiveShell as OX7
from .utils.sysinfo import sys_info as OX8
from .utils.frame import extract_module_locals as OX9

__author__ = '%s <%s>' % (OX4.author, OX4.author_email)
__license__  = OX4.license
__version__  = OX4.version
version_info = OX4.version_info
__patched_cves__ = {"CVE-2022-21699"}

def OX10(module=None, local_ns=None, **kwargs):
    (OX11, OX12) = OX9(1)
    if module is None:
        module = OX11
    if local_ns is None:
        local_ns = OX12
    
    from ipykernel.embed import embed_kernel as OX13
    OX13(module=module, local_ns=local_ns, **kwargs)

def OX14(argv=None, **kwargs):
    from IPython.terminal.ipapp import launch_new_instance as OX15
    return OX15(argv=argv, **kwargs)

def OX16(argv=None, **kwargs):
    import warnings as OX17

    OX17.warn(
        "start_kernel is deprecated since IPython 8.0, use from `ipykernel.kernelapp.launch_new_instance`",
        DeprecationWarning,
        stacklevel=2,
    )
    from ipykernel.kernelapp import launch_new_instance as OX18
    return OX18(argv=argv, **kwargs)