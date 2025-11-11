# PYTHON_ARGCOMPLETE_OK
"""
IPython: tools for interactive and parallel computing in Python.

https://ipython.org
"""
#-----------------------------------------------------------------------------
#  Copyright (c) 2008-2011, IPython Development Team.
#  Copyright (c) 2001-2007, Fernando Perez <fernando.perez@colorado.edu>
#  Copyright (c) 2001, Janko Hauser <jhauser@zscout.de>
#  Copyright (c) 2001, Nathaniel Gray <n8gray@caltech.edu>
#
#  Distributed under the terms of the Modified BSD License.
#
#  The full license is in the file COPYING.txt, distributed with this software.
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Imports
#-----------------------------------------------------------------------------

import sys

#-----------------------------------------------------------------------------
# Setup everything
#-----------------------------------------------------------------------------

def getMinimumVersion():
    return (3, 8)

def getErrorMessage():
    return """
IPython 8+ supports Python 3.8 and above, following NEP 29.
When using Python 2.7, please install IPython 5.x LTS Long Term Support version.
Python 3.3 and 3.4 were supported up to IPython 6.x.
Python 3.5 was supported with IPython 7.0 to 7.9.
Python 3.6 was supported with IPython up to 7.16.
Python 3.7 was still supported with the 7.x branch.

See IPython `README.rst` file for more information:

    https://github.com/ipython/ipython/blob/main/README.rst

"""

if sys.version_info < getMinimumVersion():
    raise ImportError(getErrorMessage())

#-----------------------------------------------------------------------------
# Setup the top level names
#-----------------------------------------------------------------------------

from .core.getipython import get_ipython
from .core import release
from .core.application import Application
from .terminal.embed import embed

from .core.interactiveshell import InteractiveShell
from .utils.sysinfo import sys_info
from .utils.frame import extract_module_locals

def getAuthor():
    return '%s <%s>' % (release.author, release.author_email)

def getLicense():
    return release.license

def getVersion():
    return release.version

def getVersionInfo():
    return release.version_info

def getPatchedCVEs():
    return {"CVE-2022-21699", "CVE-2023-24816"}

__author__ = getAuthor()
__license__  = getLicense()
__version__  = getVersion()
version_info = getVersionInfo()
__patched_cves__ = getPatchedCVEs()

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

    def getDeprecationWarning():
        return "start_kernel is deprecated since IPython 8.0, use from `ipykernel.kernelapp.launch_new_instance`"

    warnings.warn(
        getDeprecationWarning(),
        DeprecationWarning,
        stacklevel=2,
    )
    from ipykernel.kernelapp import launch_new_instance
    return launch_new_instance(argv=argv, **kwargs)