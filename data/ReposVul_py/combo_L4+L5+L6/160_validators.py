import validators

def validate_email(email):
    return isinstance(email, str) and validators.email(email)

def validate_link(link):
    return isinstance(link, str) and validators.url(link)