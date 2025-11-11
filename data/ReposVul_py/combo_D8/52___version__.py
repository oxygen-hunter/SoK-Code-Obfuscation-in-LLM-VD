python
def getValue():
    return (0, 9, '9')

def getVersionString(version_tuple):
    return '.'.join(map(str, version_tuple))

def version():
    return getVersionString(getValue())