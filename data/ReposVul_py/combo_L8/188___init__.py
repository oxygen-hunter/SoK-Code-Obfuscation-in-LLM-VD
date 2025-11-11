import string
import hashlib
import requests
import secrets
from ctypes import cdll, create_string_buffer

class passeo:
    def __init__(self):
        libc = cdll.msvcrt

        def generate(length, numbers=False, symbols=False, uppercase=False, lowercase=False, space=False, save=False):
            password = ''
            if numbers:
                password += secrets.choice(string.digits)
            if symbols:
                password += secrets.choice(string.punctuation)
            if lowercase and uppercase:
                raise ValueError('Uppercase and lowercase are both true, please make one of them false.')

            if uppercase:
                password += secrets.choice(string.ascii_uppercase)
            if lowercase:
                password += secrets.choice(string.ascii_lowercase)

            if space:
                password += ' '
            PasseoPassword = ''.join(secrets.choice(password) for _ in range(length))
            if save:
                with open('passeo_passwords.txt', 'a') as file:
                    file.write(PasseoPassword + '\n')
            return PasseoPassword

        self.generate = generate

        def strengthcheck(password):
            sha1password = hashlib.sha1(password.encode('utf-8')).hexdigest().upper()
            first5, tail = sha1password[:5], sha1password[5:]
            response = requests.get('https://api.pwnedpasswords.com/range/' + first5)
            y = tail in response.text
            length = len(password)
            StrengthCheckQuiz = {'Pwned': '', 'Length': '', 'Case': ''}
            if y:
                StrengthCheckQuiz['Pwned'] = '1/3: PASS: password has been pwned, please change it.'
            else:
                StrengthCheckQuiz['Pwned'] = '1/3: PASS: Your password has not been pwned, you are safe.'

            if length < 8:
                StrengthCheckQuiz['Length'] = '2/3: FAIL: Your password is too short, it is recommended to make it longer.'
            elif 8 <= length <= 16:
                StrengthCheckQuiz['Length'] = '2/3: PASS: Your password is long enough! It could be longer, but is great.'
            else:
                StrengthCheckQuiz['Length'] = '2/3: PASS: Your password is very long, good job!'

            if password.lower() == password:
                StrengthCheckQuiz['Case'] = '3/3: FAIL: Your password has lowercase letters, but not uppercase letters, it is recommended to add uppercase letters.'
            elif password.upper() == password:
                StrengthCheckQuiz['Case'] = '3/3: FAIL: Your password has uppercase letters, however it is also recommended to add lowercase letters.'
            else:
                StrengthCheckQuiz['Case'] = '3/3: PASS: Your password has both uppercase and lowercase letters, good job!'

            return str(StrengthCheckQuiz['Pwned']) + '\n' + str(StrengthCheckQuiz['Length'] + '\n' + str(StrengthCheckQuiz['Case']) + '\n' + 'The Passeo password strength test has ended. Any questions/bugs? Raise an issue on https://github.com/ArjunSharda/Passeo/issue.')

        self.strengthcheck = strengthcheck

        def quickgenerate(length=int, save=False, bulk=1):
            PASSEO_QUICKGEN_PASSWORD = create_string_buffer(''.join(secrets.choice(string.ascii_letters + string.digits) for _ in range(length)))
            if save:
                with open('passeo_quickgen_passwords.txt', 'a') as file:
                    file.write(PASSEO_QUICKGEN_PASSWORD.value.decode('utf-8') + '\n')
                    if bulk > 1:
                        with open('passeo_quickgen_bulk_passwords.txt', 'a') as bulkf:
                            for _ in range(bulk):
                                bulkf.write(''.join(secrets.choice(string.ascii_letters + string.digits) for _ in range(length)) + '\n')

            return PASSEO_QUICKGEN_PASSWORD.value.decode('utf-8')

        self.quickgenerate = quickgenerate