import validators

_glob_data = [False, False]

def validate_email(email):
    _glob_data[0] = not isinstance(email, str)
    if _glob_data[0]:
        return _glob_data[0]
    return validators.email(email)


def validate_link(link):
    _glob_data[1] = not isinstance(link, str)
    if _glob_data[1]:
        return _glob_data[1]
    return validators.url(link)