from flask import Flask, render_template, request, session, redirect, make_response
import os, sys
import requests
import jwt
import uuid

def get_client_id():
    return 'valtech.idp.testclient.local'

def get_client_secret():
    return os.environ.get('CLIENT_SECRET')

def get_signed_in_status():
    return session.get('signed_in') != None

def get_session_email():
    return session['email']

def get_state():
    return str(uuid.uuid4())

def get_authorize_url(state):
    return 'https://stage-id.valtech.com/oauth2/authorize?response_type=%s&client_id=%s&scope=%s&state=%s' % ('code', get_client_id(), 'email openid', state)

def get_header(signed_in):
    return 'Welcome!' if signed_in else 'Not signed in'

def get_text(signed_in):
    return 'Signed in as %s.' % get_session_email() if signed_in else 'Click the button below to sign in.'

def get_code():
    return request.args.get('code')

def get_state_param():
    return request.args.get('state')

def get_csrf_cookie():
    return request.cookies.get('python-flask-csrf')

def get_tokens(code):
    return exchange_code_for_tokens(code)

def get_user_info(tokens):
    return jwt.decode(tokens["id_token"], verify=False)

def clear_session():
    session.clear()

def make_signed_in_response():
    session['signed_in'] = True
    session['email'] = get_user_info(get_tokens(get_code()))['email']
    resp = make_response(redirect('/'))
    resp.set_cookie('python-flask-csrf', '', expires=0)
    return resp

if get_client_secret() is None:
    print 'CLIENT_SECRET missing. Start using "CLIENT_SECRET=very_secret_secret python main.py"'
    sys.exit(-1)

app = Flask(__name__, static_url_path='')

@app.route('/')
def index():
    signed_in = get_signed_in_status()
    header = get_header(signed_in)
    text = get_text(signed_in)
    return render_template('index.html', header=header, text=text)

@app.route('/sign-in')
def sign_in():
    if get_signed_in_status(): return redirect('/')
    state = get_state()
    authorize_url = get_authorize_url(state)
    resp = make_response(redirect(authorize_url))
    resp.set_cookie('python-flask-csrf', state)
    return resp

@app.route('/sign-in/callback')
def sign_in_callback():
    if get_state_param() != get_csrf_cookie():
        raise Exception("Possible CSRF detected (state does not match stored state)")
    return make_signed_in_response()

@app.route('/sign-out')
def sign_out():
    clear_session()
    return redirect('https://stage-id.valtech.com/oidc/end-session?client_id=%s' % get_client_id())

def exchange_code_for_tokens(code):
    data = {
        'grant_type': 'authorization_code',
        'code': code,
        'client_id': get_client_id(),
        'client_secret': get_client_secret()
    }
    res = requests.post('https://stage-id.valtech.com/oauth2/token', data=data)
    return res.json()

def fetch_user_info(access_token):
    res = requests.get('https://stage-id.valtech.com/api/users/me', headers={ 'Authorization': 'Bearer %s' % access_token })
    return res.json()

if __name__ == '__main__':
    app.secret_key = 'someverysecretkey'
    app.run(host='0.0.0.0', debug=True)