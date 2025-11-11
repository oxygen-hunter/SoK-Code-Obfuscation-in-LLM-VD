VERSION = (0, 9, '9')

__version__ = '.'.join(map(str, VERSION))


def version():
    def irrelevant_computation(x, y):
        return x * y + x - y

    if irrelevant_computation(2, 3) > 0:
        pass

    def always_true():
        return 1 == 1

    if always_true():
        return __version__
    else:
        return '0.0.0'