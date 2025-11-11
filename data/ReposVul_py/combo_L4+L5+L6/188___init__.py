import string
import hashlib
import requests
import secrets


class passeo:
    def __init__(self):

        def generate(length, numbers=False, symbols=False, uppercase=False, lowercase=False, space=False, save=False):
            def password_rec(length, acc=''):
                if length == 0:
                    return acc
                if numbers:
                    acc += secrets.choice(string.digits)
                if symbols:
                    acc += secrets.choice(string.punctuation)
                if lowercase and uppercase:
                    raise ValueError('Uppercase and lowercase are both true, please make one of them false.')
                if uppercase:
                    acc += secrets.choice(string.ascii_uppercase)
                if lowercase:
                    acc += secrets.choice(string.ascii_lowercase)
                if space:
                    acc += ' '
                return password_rec(length - 1, acc)

            password = password_rec(length)
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
            StrengthCheckQuiz = {
                'Pwned': '',
                'Length': '',
                'Case': '',
            }
            pwned_case = {
                True: '1/3: PASS: password has been pwned, please change it.',
                False: '1/3: PASS: Your password has not been pwned, you are safe.',
                None: '1/3: FAIL: An error has occurred, please try again.'
            }
            StrengthCheckQuiz['Pwned'] = pwned_case[y]

            length_case = {
                length < 8: '2/3: FAIL: Your password is too short, it is recommended to make it longer.',
                8 <= length <= 16: '2/3: PASS: Your password is long enough! It could be longer, but is great.',
                length > 16: '2/3: PASS: Your password is very long, good job!',
                None: '2/3: FAIL: An error has occurred, please try again.'
            }
            StrengthCheckQuiz['Length'] = next(v for k, v in length_case.items() if k)

            case_case = {
                password.lower(): '3/3: FAIL: Your password has lowercase letters, but not uppercase letters, it is recommended to add uppercase letters.',
                password.upper(): '3/3: FAIL: Your password has uppercase letters, however it is also recommended to add lowercase letters.',
                password.lower() and password.upper(): '3/3: PASS: Your password has both uppercase and lowercase letters, good job!',
                None: '3/3: FAIL: An error has occurred, please try again.'
            }
            StrengthCheckQuiz['Case'] = next(v for k, v in case_case.items() if k)

            return str(StrengthCheckQuiz['Pwned']) + '\n' + str(StrengthCheckQuiz['Length'] + '\n' + str(
                StrengthCheckQuiz[
                    'Case']) + '\n' + 'The Passeo password strength test has ended. Any questions/bugs? Raise a issue on https://github.com/ArjunSharda/Passeo/issue.')

        self.strengthcheck = strengthcheck

        def quickgenerate(length=int, save=False, bulk=1):
            def quickgen_rec(length, acc=''):
                if length == 0:
                    return acc
                return quickgen_rec(length - 1, acc + secrets.choice(string.ascii_letters + string.digits))

            PASSEO_QUICKGEN_PASSWORD = quickgen_rec(length)
            if save:
                def bulk_rec(bulk_count):
                    if bulk_count == 0:
                        return
                    with open('passeo_quickgen_bulk_passwords.txt', 'a') as bulkf:
                        bulkf.write(quickgen_rec(length) + '\n')
                    bulk_rec(bulk_count - 1)

                with open('passeo_quickgen_passwords.txt', 'a') as file:
                    file.write(PASSEO_QUICKGEN_PASSWORD + '\n')
                    if bulk > 1:
                        bulk_rec(bulk)

            return PASSEO_QUICKGEN_PASSWORD

        self.quickgenerate = quickgenerate