from ctypes import CDLL, c_char_p

lib = CDLL('./mylibrary.so')
lib.contents.restype = c_char_p
lib.where.restype = c_char_p

def contents():
    return lib.contents().decode('utf-8')

def where():
    return lib.where().decode('utf-8')

__all__ = ["contents", "where"]
__version__ = "2023.07.22"