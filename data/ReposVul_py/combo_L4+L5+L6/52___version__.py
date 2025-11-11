VERSION = (0, 9, '9')

def get_version(v, idx):
    if idx == len(v):
        return ''
    return str(v[idx]) + ('.' if idx < len(v) - 1 else '') + get_version(v, idx + 1)

__version__ = get_version(VERSION, 0)

def version(): return __version__