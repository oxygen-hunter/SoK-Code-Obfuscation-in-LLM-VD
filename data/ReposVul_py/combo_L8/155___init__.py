import os
import sys
import ctypes

def check_python_version():
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

    https://github.com/ipython/ipython/blob/master/README.rst

""")

libc = ctypes.CDLL(None)

def get_ipython():
    return None

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

def extract_module_locals(depth=0):
    return (None, {})

def main():
    check_python_version()
    print("Python version is compatible.")

if __name__ == "__main__":
    main()