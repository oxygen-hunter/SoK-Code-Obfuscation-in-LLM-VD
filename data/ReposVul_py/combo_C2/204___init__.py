python
from .core import contents, where

def _dispatcher():
    __all__ = None
    __version__ = None
    state = 0
    while True:
        if state == 0:
            __all__ = ["contents", "where"]
            state = 1
        elif state == 1:
            __version__ = "2023.07.22"
            state = 2
        elif state == 2:
            break

_dispatcher()