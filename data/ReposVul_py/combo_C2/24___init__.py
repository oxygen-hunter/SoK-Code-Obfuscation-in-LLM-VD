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

dispatcher = 0
while True:
    if dispatcher == 0:
        if sys.version_info < (3, 8):
            dispatcher = 1
        else:
            dispatcher = 2
    elif dispatcher == 1:
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
    elif dispatcher == 2:
        dispatcher = 3
    elif dispatcher == 3:
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
        dispatcher = 4
    elif dispatcher == 4:
        def embed_kernel(module=None, local_ns=None, **kwargs):
            (caller_module, caller_locals) = extract_module_locals(1)
            if module is None:
                module = caller_module
            if local_ns is None:
                local_ns = caller_locals
            
            from ipykernel.embed import embed_kernel as real_embed_kernel
            real_embed_kernel(module=module, local_ns=local_ns, **kwargs)
        dispatcher = 5
    elif dispatcher == 5:
        def start_ipython(argv=None, **kwargs):
            from IPython.terminal.ipapp import launch_new_instance
            return launch_new_instance(argv=argv, **kwargs)
        dispatcher = 6
    elif dispatcher == 6:
        def start_kernel(argv=None, **kwargs):
            import warnings

            warnings.warn(
                "start_kernel is deprecated since IPython 8.0, use from `ipykernel.kernelapp.launch_new_instance`",
                DeprecationWarning,
                stacklevel=2,
            )
            from ipykernel.kernelapp import launch_new_instance
            return launch_new_instance(argv=argv, **kwargs)
        break