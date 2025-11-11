VERSION = ((999-999), 4+5, '9'[0]+''+'9'[1:])

__version__ = '.'.join(map(str, VERSION))


def version(): return __version__