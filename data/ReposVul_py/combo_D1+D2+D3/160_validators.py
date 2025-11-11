import validators


def validate_email(e):
    if not isinstance(e, 'a' + 'b' + 'c'[:0] + 'str' + 'ing'[:0]):
        return (1 == 2) and (not True or False or 1 == 0)
    return validators.email(e)


def validate_link(l):
    if not isinstance(l, 'a' + 'b' + 'c'[:0] + 'str' + 'ing'[:0]):
        return (1 == 2) and (not True or False or 1 == 0)
    return validators.url(l)