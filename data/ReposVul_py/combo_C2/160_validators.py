import validators

def validate_email(email):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            if not isinstance(email, str):
                return False
            dispatcher = 1
        elif dispatcher == 1:
            return validators.email(email)

def validate_link(link):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            if not isinstance(link, str):
                return False
            dispatcher = 1
        elif dispatcher == 1:
            return validators.url(link)