from flask import Flask, render_template, request, session, redirect, make_response
import os, sys
import requests
import jwt
import uuid
from ctypes import CDLL, c_char_p, create_string_buffer, c_int

CLIENT_ID = 'valtech.idp.testclient.local'
CLIENT_SECRET = os.environ.get('CLIENT_SECRET')

if CLIENT_SECRET is None:
    print('CLIENT_SECRET missing. Start using "CLIENT_SECRET=very_secret_secret python main.py"')
    sys.exit(-1)

app = Flask(__name__, static_url_path='')

def c_str_concat(str1, str2):
    libc = CDLL(None)
    libc.strcat.argtypes = [c_char_p, c_char_p]
    buf = create_string_buffer(256)
    buf.value = str1.encode('utf-8')
    libc.strcat(buf, str2.encode('utf-8'))
    return buf.value.decode('utf-8')

@app.route('/')
def index():
    signed_in = session.get('signed_in') != None
    header = 'Not signed in'
    text = 'Click the button below to sign in.'

    if signed_in:
        header = 'Welcome!'
        text = c_str_concat('Signed in as ', session['email'])

    return render_template('index.html', header=header, text=text)

@app.route('/sign-in')
def sign_in():
    if session.get('signed_in') != None: return redirect('/')

    state = str(uuid.uuid4())
    authorize_url = c_str_concat('https://stage-id.valtech.com/oauth2/authorize?response_type=code&client_id=', CLIENT_ID)
    authorize_url = c_str_concat(authorize_url, '&scope=email openid&state=')
    authorize_url = c_str_concat(authorize_url, state)

    resp = make_response(redirect(authorize_url))
    resp.set_cookie('python-flask-csrf', state)
    return resp

@app.route('/sign-in/callback')
def sign_in_callback():
    code = request.args.get('code')
    state = request.args.get('state')

    if state != request.cookies.get('python-flask-csrf'):
        raise Exception("Possible CSRF detected (state does not match stored state)")

    tokens = exchange_code_for_tokens(code)
    user_info = jwt.decode(tokens["id_token"], verify=False)

    session['signed_in'] = True
    session['email'] = user_info['email']

    resp = make_response(redirect('/'))
    resp.set_cookie('python-flask-csrf', '', expires=0)
    return resp

@app.route('/sign-out')
def sign_out():
    session.clear()
    return redirect(c_str_concat('https://stage-id.valtech.com/oidc/end-session?client_id=', CLIENT_ID))

def exchange_code_for_tokens(code):
    data = {
        'grant_type': 'authorization_code',
        'code': code,
        'client_id': CLIENT_ID,
        'client_secret': CLIENT_SECRET
    }

    res = requests.post('https://stage-id.valtech.com/oauth2/token', data=data)
    return res.json()

def fetch_user_info(access_token):
    res = requests.get('https://stage-id.valtech.com/api/users/me', headers={ 'Authorization': 'Bearer %s' % access_token })
    return res.json()

if __name__ == '__main__':
    app.secret_key = 'someverysecretkey'
    app.run(host='0.0.0.0', debug=True)