import string
import hashlib
import requests
import secrets

class passeo:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []

        def VM():
            while self.pc < len(self.program):
                op, *args = self.program[self.pc]
                if op == 'PUSH':
                    self.stack.append(args[0])
                elif op == 'POP':
                    self.stack.pop()
                elif op == 'ADD':
                    self.stack.append(self.stack.pop() + self.stack.pop())
                elif op == 'SUB':
                    self.stack.append(self.stack.pop() - self.stack.pop())
                elif op == 'JMP':
                    self.pc = args[0] - 1
                elif op == 'JZ':
                    if self.stack.pop() == 0:
                        self.pc = args[0] - 1
                elif op == 'LOAD':
                    self.stack.append(vars[self.stack.pop()])
                elif op == 'STORE':
                    vars[self.stack.pop()] = self.stack.pop()
                self.pc += 1

        def compile_generate(length, numbers, symbols, uppercase, lowercase, space, save):
            self.program = [
                ('PUSH', ''), 
                ('STORE', 0),
                ('PUSH', numbers),
                ('JZ', 2),
                ('PUSH', string.digits),
                ('PUSH', 0),
                ('LOAD',),
                ('PUSH', secrets.choice),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', symbols),
                ('JZ', 4),
                ('PUSH', string.punctuation),
                ('PUSH', 0),
                ('LOAD',),
                ('PUSH', secrets.choice),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', lowercase),
                ('PUSH', uppercase),
                ('ADD',),
                ('PUSH', 2),
                ('SUB',),
                ('JZ', 6),
                ('PUSH', ValueError),
                ('PUSH', 'Uppercase and lowercase are both true, please make one of them false.'),
                ('PUSH', 0),
                ('LOAD',),
                ('PUSH', raise_),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', uppercase),
                ('JZ', 8),
                ('PUSH', string.ascii_uppercase),
                ('PUSH', 0),
                ('LOAD',),
                ('PUSH', secrets.choice),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', lowercase),
                ('JZ', 10),
                ('PUSH', string.ascii_lowercase),
                ('PUSH', 0),
                ('LOAD',),
                ('PUSH', secrets.choice),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', space),
                ('JZ', 12),
                ('PUSH', ' '),
                ('PUSH', 0),
                ('LOAD',),
                ('PUSH', secrets.choice),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', length),
                ('PUSH', 0),
                ('LOAD',),
                ('PUSH', (lambda x: ''.join(secrets.choice(x) for _ in range(length)))),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', save),
                ('JZ', 14),
                ('PUSH', 'passeo_passwords.txt'),
                ('PUSH', 'a'),
                ('PUSH', 0),
                ('LOAD',),
                ('PUSH', (lambda f, p: f.write(p + '\n'))),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', 0),
                ('LOAD',),
                ('RETURN',),
            ]
            VM()
            return self.stack.pop()

        def compile_strengthcheck(password):
            sha1password = hashlib.sha1(password.encode('utf-8')).hexdigest().upper()
            first5, tail = sha1password[:5], sha1password[5:]
            response = requests.get('https://api.pwnedpasswords.com/range/' + first5)
            y = tail in response.text
            length = len(password)
            StrengthCheckQuiz = {'Pwned': '', 'Length': '', 'Case': ''}
            self.program = [
                ('PUSH', y),
                ('JZ', 2),
                ('PUSH', '1/3: PASS: password has been pwned, please change it.'),
                ('PUSH', 'Pwned'),
                ('STORE', 0),
                ('JMP', 8),
                ('PUSH', '1/3: PASS: Your password has not been pwned, you are safe.'),
                ('PUSH', 'Pwned'),
                ('STORE', 0),
                ('PUSH', length),
                ('PUSH', 8),
                ('SUB',),
                ('JZ', 10),
                ('PUSH', '2/3: FAIL: Your password is too short, it is recommended to make it longer.'),
                ('PUSH', 'Length'),
                ('STORE', 0),
                ('JMP', 16),
                ('PUSH', length),
                ('PUSH', 8),
                ('SUB',),
                ('PUSH', length),
                ('PUSH', 16),
                ('SUB',),
                ('JZ', 14),
                ('PUSH', '2/3: PASS: Your password is long enough! It could be longer, but is great.'),
                ('PUSH', 'Length'),
                ('STORE', 0),
                ('JMP', 20),
                ('PUSH', '2/3: PASS: Your password is very long, good job!'),
                ('PUSH', 'Length'),
                ('STORE', 0),
                ('PUSH', password.lower()),
                ('JZ', 22),
                ('PUSH', '3/3: FAIL: Your password has lowercase letters, but not uppercase letters, it is recommended to add uppercase letters.'),
                ('PUSH', 'Case'),
                ('STORE', 0),
                ('JMP', 28),
                ('PUSH', password.upper()),
                ('JZ', 24),
                ('PUSH', '3/3: FAIL: Your password has uppercase letters, however it is also recommended to add lowercase letters.'),
                ('PUSH', 'Case'),
                ('STORE', 0),
                ('JMP', 32),
                ('PUSH', password.lower()),
                ('PUSH', password.upper()),
                ('ADD',),
                ('JZ', 26),
                ('PUSH', '3/3: PASS: Your password has both uppercase and lowercase letters, good job!'),
                ('PUSH', 'Case'),
                ('STORE', 0),
                ('PUSH', 0),
                ('LOAD',),
                ('ADD',),
                ('RETURN',)
            ]
            VM()
            return str(StrengthCheckQuiz['Pwned']) + '\n' + str(StrengthCheckQuiz['Length'] + '\n' + str(
                StrengthCheckQuiz[
                    'Case']) + '\n' + 'The Passeo password strength test has ended. Any questions/bugs? Raise a issue on https://github.com/ArjunSharda/Passeo/issue.')

        def compile_quickgenerate(length, save, bulk):
            PASSEO_QUICKGEN_PASSWORD = ''.join(
                secrets.choice(string.ascii_letters + string.digits) for i in range(length))
            self.program = [
                ('PUSH', save),
                ('JZ', 2),
                ('PUSH', 'passeo_quickgen_passwords.txt'),
                ('PUSH', 'a'),
                ('PUSH', 0),
                ('PUSH', PASSEO_QUICKGEN_PASSWORD),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', bulk),
                ('PUSH', 1),
                ('SUB',),
                ('JZ', 10),
                ('PUSH', 'passeo_quickgen_bulk_passwords.txt'),
                ('PUSH', 'a'),
                ('PUSH', 0),
                ('PUSH', bulk),
                ('ADD',),
                ('STORE', 0),
                ('PUSH', 0),
                ('LOAD',),
                ('ADD',),
                ('RETURN',)
            ]
            VM()
            return PASSEO_QUICKGEN_PASSWORD

        self.generate = compile_generate
        self.strengthcheck = compile_strengthcheck
        self.quickgenerate = compile_quickgenerate