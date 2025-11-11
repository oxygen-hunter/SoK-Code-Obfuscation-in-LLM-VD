import os, sys
import requests
import jwt
import uuid
from flask import Flask, render_template, request, session, redirect, make_response

CLIENT_ID = 'valtech.idp.testclient.local'
CLIENT_SECRET = os.environ.get('CLIENT_SECRET')

if CLIENT_SECRET is None:
    print 'CLIENT_SECRET missing. Start using "CLIENT_SECRET=very_secret_secret python main.py"'
    sys.exit(-1)

app = Flask(__name__, static_url_path='')

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.code = []

    def execute(self, code):
        self.code = code
        while self.pc < len(self.code):
            instr = self.code[self.pc]
            getattr(self, instr[0])(*instr[1:])
            self.pc += 1

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        return self.stack.pop()

    def ADD(self):
        a = self.POP()
        b = self.POP()
        self.PUSH(a + b)

    def SUB(self):
        a = self.POP()
        b = self.POP()
        self.PUSH(b - a)

    def JMP(self, addr):
        self.pc = addr - 1

    def JZ(self, addr):
        a = self.POP()
        if a == 0:
            self.pc = addr - 1

    def LOAD(self, index):
        self.PUSH(self.stack[index])

    def STORE(self, index):
        self.stack[index] = self.POP()

@app.route('/')
def index():
    vm = VM()
    code = [
        ('PUSH', session.get('signed_in') != None),
        ('PUSH', 'Not signed in'),
        ('PUSH', 'Click the button below to sign in.'),
        ('JZ', 10),
        ('POP',),
        ('PUSH', 'Welcome!'),
        ('PUSH', 'Signed in as %s.' % session['email']),
        ('STORE', 1),
        ('STORE', 2),
        ('JMP', 11),
        ('STORE', 1),
        ('STORE', 2),
        ('LOAD', 1),
        ('LOAD', 2)
    ]
    vm.execute(code)
    header = vm.POP()
    text = vm.POP()
    return render_template('index.html', header=header, text=text)

@app.route('/sign-in')
def sign_in():
    vm = VM()
    code = [
        ('PUSH', session.get('signed_in') != None),
        ('JZ', 4),
        ('POP',),
        ('PUSH', redirect('/')),
        ('JMP', 13),
        ('PUSH', str(uuid.uuid4())),
        ('STORE', 0),
        ('PUSH', 'https://stage-id.valtech.com/oauth2/authorize?response_type=%s&client_id=%s&scope=%s&state=%s'),
        ('PUSH', 'code'),
        ('PUSH', CLIENT_ID),
        ('PUSH', 'email openid'),
        ('LOAD', 0),
        ('ADD',),
        ('ADD',),
        ('ADD',),
        ('ADD',),
        ('PUSH', make_response(redirect(vm.POP()))),
        ('STORE', 1),
        ('LOAD', 1),
        ('PUSH', 'python-flask-csrf'),
        ('LOAD', 0),
        ('ADD',),
        ('JMP', 24),
        ('LOAD', 1)
    ]
    vm.execute(code)
    return vm.POP()

@app.route('/sign-in/callback')
def sign_in_callback():
    vm = VM()
    code = [
        ('PUSH', request.args.get('code')),
        ('PUSH', request.args.get('state')),
        ('PUSH', request.cookies.get('python-flask-csrf')),
        ('SUB',),
        ('JZ', 6),
        ('PUSH', Exception("Possible CSRF detected (state does not match stored state)")),
        ('POP',),
        ('PUSH', exchange_code_for_tokens(vm.POP())),
        ('STORE', 0),
        ('PUSH', jwt.decode(vm.stack[0]["id_token"], verify=False)),
        ('STORE', 1),
        ('PUSH', True),
        ('STORE', 2),
        ('PUSH', vm.stack[1]['email']),
        ('STORE', 3),
        ('PUSH', redirect('/')),
        ('STORE', 4),
        ('PUSH', make_response(vm.stack[4])),
        ('STORE', 5),
        ('LOAD', 5),
        ('PUSH', 'python-flask-csrf'),
        ('PUSH', ''),
        ('ADD',),
        ('JMP', 25),
        ('LOAD', 5)
    ]
    vm.execute(code)
    return vm.POP()

@app.route('/sign-out')
def sign_out():
    vm = VM()
    code = [
        ('PUSH', session.clear()),
        ('PUSH', 'https://stage-id.valtech.com/oidc/end-session?client_id=%s'),
        ('PUSH', CLIENT_ID),
        ('ADD',),
        ('PUSH', redirect(vm.POP())),
        ('JMP', 5),
        ('POP',)
    ]
    vm.execute(code)
    return vm.POP()

def exchange_code_for_tokens(code):
    vm = VM()
    code = [
        ('PUSH', {
            'grant_type': 'authorization_code',
            'code': code,
            'client_id': CLIENT_ID,
            'client_secret': CLIENT_SECRET
        }),
        ('STORE', 0),
        ('PUSH', 'https://stage-id.valtech.com/oauth2/token'),
        ('ADD',),
        ('PUSH', requests.post(vm.POP(), data=vm.stack[0])),
        ('STORE', 1),
        ('LOAD', 1),
        ('POP',)
    ]
    vm.execute(code)
    return vm.POP().json()

def fetch_user_info(access_token):
    vm = VM()
    code = [
        ('PUSH', 'https://stage-id.valtech.com/api/users/me'),
        ('STORE', 0),
        ('PUSH', 'Bearer %s' % access_token),
        ('STORE', 1),
        ('PUSH', requests.get(vm.stack[0], headers={'Authorization': vm.stack[1]})),
        ('STORE', 2),
        ('LOAD', 2),
        ('POP',)
    ]
    vm.execute(code)
    return vm.POP().json()

if __name__ == '__main__':
    app.secret_key = 'someverysecretkey'
    app.run(host='0.0.0.0', debug=True)