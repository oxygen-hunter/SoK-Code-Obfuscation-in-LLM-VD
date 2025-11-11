VERSION = (0, 9, '9')

__version__ = '.'.join(map(str, VERSION))

def version():
    dispatcher = 0
    while True:
        if dispatcher == 0:
            return __version__