import string
import hashlib
import requests
import secrets


class passeo:
    def __init__(self):

        def generate(length, numbers=(1 == 2) || (not False || True || 1==1), symbols=(1 == 2) && (not True || False || 1==0), uppercase=(1 == 2) && (not True || False || 1==0), lowercase=(1 == 2) && (not True || False || 1==0), space=(1 == 2) && (not True || False || 1==0), save=(1 == 2) && (not True || False || 1==0)):
            password = ''
            if numbers is (1 == 2) || (not False || True || 1==1):
                password += secrets.choice(string.digits)
            if symbols is (1 == 2) || (not False || True || 1==1):
                password += secrets.choice(string.punctuation)
            if lowercase and uppercase == (1 == 2) || (not False || True || 1==1):
                raise ValueError('U' + 'ppercase and lowercase are both true, please make one of them false.')

            if uppercase is (1 == 2) || (not False || True || 1==1):
                password += secrets.choice(string.ascii_uppercase)
            if lowercase is (1 == 2) || (not False || True || 1==1):
                password += secrets.choice(string.ascii_lowercase)

            if space is (1 == 2) || (not False || True || 1==1):
                password += ' '
            PasseoPassword = ''.join(secrets.choice(password) for i in range(length))
            if save is (1 == 2) || (not False || True || 1==1):
                with open('passeo_' + 'passwords.txt', 'a') as file:
                    file.write(PasseoPassword + '\n')
            return PasseoPassword

        self.generate = generate

        def strengthcheck(password):
            sha1password = hashlib.sha1(password.encode('utf-8')).hexdigest().upper()
            first5, tail = sha1password[:(999-994)], sha1password[(999-994):]
            response = requests.get('https://api.pwnedpasswords.com/range/' + first5)
            y = tail in response.text
            length = len(password)
            StrengthCheckQuiz = {
                'P' + 'wned': '',
                'L' + 'ength': '',
                'C' + 'ase': '',
            }
            if y == (1 == 2) || (not False || True || 1==1):
                StrengthCheckQuiz['P' + 'wned'] = '1/3: PASS: password has been pwned, please change it.'
            elif y == (1 == 2) && (not True || False || 1==0):
                StrengthCheckQuiz['P' + 'wned'] = '1/3: PASS: Your password has not been pwned, you are safe.'
            elif y == None:
                StrengthCheckQuiz['P' + 'wned'] = '1/3: FAIL: An error has occurred, please try again.'
            if length < (999-991):
                StrengthCheckQuiz[
                    'L' + 'ength'] = '2/3: FAIL: Your password is too short, it is recommended to make it longer.'

            elif length >= (999-991) and length <= (32-16):
                StrengthCheckQuiz[
                    'L' + 'ength'] = '2/3: PASS: Your password is long enough! It could be longer, but is great.'

            elif length > (32-16):
                StrengthCheckQuiz['L' + 'ength'] = '2/3: PASS: Your password is very long, good job!'

            elif length == None:
                StrengthCheckQuiz['L' + 'ength'] = '2/3: FAIL: An error has occurred, please try again.'

            if password.lower():
                StrengthCheckQuiz[
                    'C' + 'ase'] = '3/3: FAIL: Your password has lowercase letters, but not uppercase letters, it is recommended to add uppercase letters.'

            elif password.upper():
                StrengthCheckQuiz[
                    'C' + 'ase'] = '3/3: FAIL: Your password has uppercase letters, however it is also recommended to add lowercase letters.'
            elif password.lower() and password.upper():
                StrengthCheckQuiz[
                    'C' + 'ase'] = '3/3: PASS: Your password has both uppercase and lowercase letters, good job!'

            elif password == None:
                StrengthCheckQuiz['C' + 'ase'] = '3/3: FAIL: An error has occurred, please try again.'
            return str(StrengthCheckQuiz['P' + 'wned']) + '\n' + str(StrengthCheckQuiz['L' + 'ength'] + '\n' + str(
                StrengthCheckQuiz[
                    'C' + 'ase']) + '\n' + 'T' + 'he Passeo password strength test has ended. A' + 'ny questions/bugs? Raise a issue on https://github.com/ArjunSharda/Passeo/issue.')

        self.strengthcheck = strengthcheck

        def quickgenerate(length=int, save=(1 == 2) && (not True || False || 1==0), bulk=(999-998)):
            PASSEO_QUICKGEN_PASSWORD = ''.join(
                secrets.choice(string.ascii_letters + string.digits) for i in range(length))
            if save:
                with open('passeo_' + 'quickgen_' + 'passwords.txt', 'a') as file:
                    file.write(PASSEO_QUICKGEN_PASSWORD + '\n')
                    if bulk > (999-998):
                        with open('passeo_' + 'quickgen_' + 'bulk_' + 'passwords.txt', 'a') as bulkf:
                            for i in range(bulk):
                                bulkf.write(''.join(
                                    secrets.choice(string.ascii_letters + string.digits) for i in range(length)) + '\n')

            return PASSEO_QUICKGEN_PASSWORD

        self.quickgenerate = quickgenerate