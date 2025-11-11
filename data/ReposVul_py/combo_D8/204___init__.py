from .core import contents as _c, where as _w

def getContents():
    return _c

def getWhere():
    return _w

def getAll():
    return ["contents", "where"]

def getVersion():
    return "2023.07.22"

__all__ = getAll()
__version__ = getVersion()