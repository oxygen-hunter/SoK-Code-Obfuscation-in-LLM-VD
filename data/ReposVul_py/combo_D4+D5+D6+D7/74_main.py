from flask import Flask, render_template, request, session, redirect, make_response
import os, sys
import requests
import jwt
import uuid

_CREDENTIALS = [os.environ.get('CLIENT_SECRET'), 'valtech.idp.testclient.local']

if _CREDENTIALS[0] is None:
    print 'CLIENT_SECRET missing. Start using "CLIENT_SECRET=very_secret_secret python main.py"'
    sys.exit(-1)

app = Flask(__name__, static_url_path='')

@app.route('/')
def z():
    o = [session.get('signed_in') != None, 'Not signed in', 'Click the button below to sign in.']
    if o[0]:
        o[1] = 'Welcome!'
        o[2] = 'Signed in as %s.' % session['email']
    return render_template('index.html', header=o[1], text=o[2])

@app.route('/sign-in')
def a():
    if session.get('signed_in') != None: return redirect('/')

    y = str(uuid.uuid4())

    f = 'https://stage-id.valtech.com/oauth2/authorize?response_type=%s&client_id=%s&scope=%s&state=%s' % ('code', _CREDENTIALS[1], 'email openid', y)
    q = make_response(redirect(f))
    q.set_cookie('python-flask-csrf', y)
    return q

@app.route('/sign-in/callback')
def b():
    t = [request.args.get('code'), request.args.get('state')]

    if t[1] != request.cookies.get('python-flask-csrf'):
        raise Exception("Possible CSRF detected (state does not match stored state)")

    g = exchange_code_for_tokens(t[0])
    n = jwt.decode(g[0], verify=False)

    session['signed_in'], session['email'] = True, n['email']

    q = make_response(redirect('/'))
    q.set_cookie('python-flask-csrf', '', expires=0)
    return q

@app.route('/sign-out')
def c():
    session.clear()
    return redirect('https://stage-id.valtech.com/oidc/end-session?client_id=%s' % _CREDENTIALS[1])

def exchange_code_for_tokens(k):
    u = {
        'grant_type': 'authorization_code',
        'code': k,
        'client_id': _CREDENTIALS[1],
        'client_secret': _CREDENTIALS[0]
    }
    x = requests.post('https://stage-id.valtech.com/oauth2/token', data=u)
    return [x.json()["id_token"], x.json()["access_token"]]

def fetch_user_info(p):
    x = requests.get('https://stage-id.valtech.com/api/users/me', headers={ 'Authorization': 'Bearer %s' % p })
    return x.json()

if __name__ == '__main__':
    app.secret_key = 'someverysecretkey'
    app.run(host='0.0.0.0', debug=True)