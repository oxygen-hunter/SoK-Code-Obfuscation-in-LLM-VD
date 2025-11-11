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

# Don't forget to also update setup.py when this changes!
version_threshold = (3, 8)
if sys.version_info < version_threshold:
    raise ImportError(
        """
IPython 8+ supports Python 3.8 and above, following NEP 29.
When using Python 2.7, please install IPython 5.x LTS Long Term Support version.
Python 3.3 and 3.4 were supported up to IPython 6.x.
Python 3.5 was supported with IPython 7.0 to 7.9.
Python 3.6 was supported with IPython up to 7.16.
Python 3.7 was still supported with the 7.x branch.

See IPython `README.rst` file for more information:

    https://github.com/ipython/ipython/blob/main/README.rst

"""
    )

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

release_data = [release.author, release.author_email, release.license, release.version, release.version_info]

__author__ = '%s <%s>' % (release_data[0], release_data[1])
__license__  = release_data[2]
__version__  = release_data[3]
version_info = release_data[4]

__patched_cves__ = {"CVE-2022-21699", "CVE-2023-24816"}

def embed_kernel(mod=None, ns=None, **kwargs):
    (call_mod, call_ns) = extract_module_locals(1)
    if mod is None:
        mod = call_mod
    if ns is None:
        ns = call_ns
    
    from ipykernel.embed import embed_kernel as real_embed_kernel
    real_embed_kernel(module=mod, local_ns=ns, **kwargs)

def start_ipython(args=None, **kwargs):
    from IPython.terminal.ipapp import launch_new_instance
    return launch_new_instance(argv=args, **kwargs)

def start_kernel(args=None, **kwargs):
    import warnings

    warnings.warn(
        "start_kernel is deprecated since IPython 8.0, use from `ipykernel.kernelapp.launch_new_instance`",
        DeprecationWarning,
        stacklevel=2,
    )
    from ipykernel.kernelapp import launch_new_instance
    return launch_new_instance(argv=args, **kwargs)