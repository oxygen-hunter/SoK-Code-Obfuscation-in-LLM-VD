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
if sys.version_info < (3, 8):
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

from .core.getipython import get_ipython as OX7B4DF339
from .core import release as OX1E4F7A9C
from .core.application import Application as OX3E8F9B1A
from .terminal.embed import embed as OX9A1B2C3D

from .core.interactiveshell import InteractiveShell as OX4C5D6E7F
from .utils.sysinfo import sys_info as OX8E9F0A1B
from .utils.frame import extract_module_locals as OX2B3C4D5E

__author__ = '%s <%s>' % (OX1E4F7A9C.author, OX1E4F7A9C.author_email)
__license__  = OX1E4F7A9C.license
__version__  = OX1E4F7A9C.version
OX6F7E8D9C = OX1E4F7A9C.version_info
__patched_cves__ = {"CVE-2022-21699", "CVE-2023-24816"}

def OX5E6F7A8B(OX8A9B0C1D=None, OX1D2E3F4G=None, **OX3F4G5H6I):
    (OX9B0C1D2E, OX1A2B3C4D) = OX2B3C4D5E(1)
    if OX8A9B0C1D is None:
        OX8A9B0C1D = OX9B0C1D2E
    if OX1D2E3F4G is None:
        OX1D2E3F4G = OX1A2B3C4D
    
    from ipykernel.embed import embed_kernel as OX7C8D9E0F
    OX7C8D9E0F(module=OX8A9B0C1D, local_ns=OX1D2E3F4G, **OX3F4G5H6I)

def OX7D8E9F0A(OX2F3G4H5I=None, **OX6I7J8K9L):
    from IPython.terminal.ipapp import launch_new_instance as OX0A1B2C3D
    return OX0A1B2C3D(argv=OX2F3G4H5I, **OX6I7J8K9L)

def OX8F9G0A1B(OX3H4I5J6K=None, **OX7K8L9M0N):
    import warnings as OX4G5H6I7J
    OX4G5H6I7J.warn(
        "start_kernel is deprecated since IPython 8.0, use from `ipykernel.kernelapp.launch_new_instance`",
        DeprecationWarning,
        stacklevel=2,
    )
    from ipykernel.kernelapp import launch_new_instance as OX5B6C7D8E
    return OX5B6C7D8E(argv=OX3H4I5J6K, **OX7K8L9M0N)