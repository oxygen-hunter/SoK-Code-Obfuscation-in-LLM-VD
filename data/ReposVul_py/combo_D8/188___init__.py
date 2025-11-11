import string
import hashlib
import requests
import secrets

class passeo:
    def __init__(self):
        
        def generate(*args, **kwargs):
            length = kwargs.get('length')
            numbers = kwargs.get('numbers', False)
            symbols = kwargs.get('symbols', False)
            uppercase = kwargs.get('uppercase', False)
            lowercase = kwargs.get('lowercase', False)
            space = kwargs.get('space', False)
            save = kwargs.get('save', False)
            
            def getChoice(characters):
                return secrets.choice(characters)
            
            def getPassword(pw, ln):
                return ''.join(getChoice(pw) for _ in range(ln))
            
            def writePassword(file_name, password):
                with open(file_name, 'a') as file:
                    file.write(password + '\n')

            A = ''
            if numbers:
                A += getChoice(string.digits)
            if symbols:
                A += getChoice(string.punctuation)
            if lowercase and uppercase:
                raise ValueError('Uppercase and lowercase are both true, please make one of them false.')
            if uppercase:
                A += getChoice(string.ascii_uppercase)
            if lowercase:
                A += getChoice(string.ascii_lowercase)
            if space:
                A += ' '
            B = getPassword(A, length)
            if save:
                writePassword('passeo_passwords.txt', B)
            return B

        self.generate = generate

        def strengthcheck(password):
            def request_api_data(query_char):
                url = 'https://api.pwnedpasswords.com/range/' + query_char
                res = requests.get(url)
                return res

            def get_password_leak_count(hashes, hash_to_check):
                hashes = (line.split(':') for line in hashes.text.splitlines())
                return any(hash_to_check == h for h, count in hashes)

            def createStrengthMessage(check, msg):
                return f"{check}: {msg}"

            hashed_password = hashlib.sha1(password.encode('utf-8')).hexdigest().upper()
            first5_char, tail = hashed_password[:5], hashed_password[5:]
            response = request_api_data(first5_char)
            y = get_password_leak_count(response, tail)
            length = len(password)
            StrengthCheckQuiz = {
                'Pwned': '',
                'Length': '',
                'Case': '',
            }
            StrengthCheckQuiz['Pwned'] = createStrengthMessage('1/3: PASS', 
                'password has been pwned, please change it.' if y else 'Your password has not been pwned, you are safe.')
            StrengthCheckQuiz['Length'] = createStrengthMessage('2/3: PASS', 
                'Your password is very long, good job!' if length > 16 else 
                'Your password is long enough! It could be longer, but is great.' if length >= 8 else 
                'Your password is too short, it is recommended to make it longer.')
            StrengthCheckQuiz['Case'] = createStrengthMessage('3/3: PASS', 
                'Your password has both uppercase and lowercase letters, good job!' if password.lower() and password.upper() 
                else 'Your password has lowercase letters, but not uppercase letters, it is recommended to add uppercase letters.' if password.lower() 
                else 'Your password has uppercase letters, however it is also recommended to add lowercase letters.')

            return '\n'.join([StrengthCheckQuiz['Pwned'], StrengthCheckQuiz['Length'], StrengthCheckQuiz['Case'], 
                              'The Passeo password strength test has ended. Any questions/bugs? Raise an issue on https://github.com/ArjunSharda/Passeo/issue.'])

        self.strengthcheck = strengthcheck

        def quickgenerate(*args, **kwargs):
            length = kwargs.get('length', int)
            save = kwargs.get('save', False)
            bulk = kwargs.get('bulk', 1)

            def generatePassword():
                return ''.join(secrets.choice(string.ascii_letters + string.digits) for _ in range(length))

            def savePassword(file_name, password):
                with open(file_name, 'a') as file:
                    file.write(password + '\n')

            A = generatePassword()
            if save:
                savePassword('passeo_quickgen_passwords.txt', A)
                if bulk > 1:
                    with open('passeo_quickgen_bulk_passwords.txt', 'a') as bulkf:
                        for _ in range(bulk):
                            bulkf.write(generatePassword() + '\n')

            return A

        self.quickgenerate = quickgenerate