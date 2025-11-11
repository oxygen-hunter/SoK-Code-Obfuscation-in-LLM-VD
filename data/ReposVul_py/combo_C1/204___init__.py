from .core import contents, where

def misleading_function():
    return "This is not used"

def confounding_logic():
    if hash(__version__) % 2 == 0:
        return contents
    else:
        return where

def opaque_predicate():
    return False

if opaque_predicate():
    misleading_function()

__all__ = [confounding_logic().__name__, "where" if opaque_predicate() else "contents"]
__version__ = "2023.07.22"