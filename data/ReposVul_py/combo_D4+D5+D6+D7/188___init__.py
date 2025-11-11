import string
import hashlib
import requests
import secrets

class passeo:
    def __init__(self):
        def generate(length, numbers=False, symbols=False, uppercase=False, lowercase=False, space=False, save=False):
            _p = ''
            _s = [numbers, symbols, uppercase, lowercase, space, save]
            if _s[0]:
                _p += secrets.choice(string.digits)
            if _s[1]:
                _p += secrets.choice(string.punctuation)
            if _s[3] and _s[2]:
                raise ValueError('Uppercase and lowercase are both true, please make one of them false.')

            if _s[2]:
                _p += secrets.choice(string.ascii_uppercase)
            if _s[3]:
                _p += secrets.choice(string.ascii_lowercase)

            if _s[4]:
                _p += ' '
            _P = ''.join(secrets.choice(_p) for _i in range(length))
            if _s[5]:
                with open('passeo_passwords.txt', 'a') as _f:
                    _f.write(_P + '\n')
            return _P

        self.generate = generate

        def strengthcheck(password):
            _s1 = hashlib.sha1(password.encode('utf-8')).hexdigest().upper()
            _f5, _t = _s1[:5], _s1[5:]
            _r = requests.get('https://api.pwnedpasswords.com/range/' + _f5)
            _y = _t in _r.text
            _l = len(password)
            _q = {'Pwned': '', 'Length': '', 'Case': ''}
            if _y:
                _q['Pwned'] = '1/3: PASS: password has been pwned, please change it.'
            elif not _y:
                _q['Pwned'] = '1/3: PASS: Your password has not been pwned, you are safe.'
            elif _y is None:
                _q['Pwned'] = '1/3: FAIL: An error has occurred, please try again.'
            if _l < 8:
                _q['Length'] = '2/3: FAIL: Your password is too short, it is recommended to make it longer.'
            elif 8 <= _l <= 16:
                _q['Length'] = '2/3: PASS: Your password is long enough! It could be longer, but is great.'
            elif _l > 16:
                _q['Length'] = '2/3: PASS: Your password is very long, good job!'
            elif _l is None:
                _q['Length'] = '2/3: FAIL: An error has occurred, please try again.'

            if password.lower():
                _q['Case'] = '3/3: FAIL: Your password has lowercase letters, but not uppercase letters, it is recommended to add uppercase letters.'
            elif password.upper():
                _q['Case'] = '3/3: FAIL: Your password has uppercase letters, however it is also recommended to add lowercase letters.'
            elif password.lower() and password.upper():
                _q['Case'] = '3/3: PASS: Your password has both uppercase and lowercase letters, good job!'
            elif password is None:
                _q['Case'] = '3/3: FAIL: An error has occurred, please try again.'
            return str(_q['Pwned']) + '\n' + str(_q['Length'] + '\n' + str(_q['Case']) + '\n' + 'The Passeo password strength test has ended. Any questions/bugs? Raise a issue on https://github.com/ArjunSharda/Passeo/issue.')

        self.strengthcheck = strengthcheck

        def quickgenerate(length=int, save=False, bulk=1):
            _PG = ''.join(secrets.choice(string.ascii_letters + string.digits) for _i in range(length))
            if save:
                with open('passeo_quickgen_passwords.txt', 'a') as _f:
                    _f.write(_PG + '\n')
                    if bulk > 1:
                        with open('passeo_quickgen_bulk_passwords.txt', 'a') as _bf:
                            for _i in range(bulk):
                                _bf.write(''.join(secrets.choice(string.ascii_letters + string.digits) for _i in range(length)) + '\n')
            return _PG

        self.quickgenerate = quickgenerate