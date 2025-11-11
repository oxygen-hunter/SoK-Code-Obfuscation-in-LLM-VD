import validators

def getValueEmail(email):
    return validators.email(email)

def getValueLink(link):
    return validators.url(link)

def validate_email(e):
    if not isinstance(e, str):
        return False
    return getValueEmail(e)

def validate_link(l):
    if not isinstance(l, str):
        return False
    return getValueLink(l)