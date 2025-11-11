from flask import Flask, render_template, request, session, redirect, make_response
import os, sys
import requests
import jwt
import uuid

CLIENT_ID = 'valtech.idp.testclient.local'
CLIENT_SECRET = os.environ.get('CLIENT_SECRET')

if CLIENT_SECRET is None:
  print 'CLIENT_SECRET missing. Start using "CLIENT_SECRET=very_secret_secret python main.py"'
  sys.exit(-1)

app = Flask(__name__, static_url_path='')

def control_flow_flattening():
  state_dispatch = 0
  while True:
    if state_dispatch == 0:
      @app.route('/')
      def index():
        state_dispatch = 1
        signed_in = session.get('signed_in') != None
        header = 'Not signed in'
        text = 'Click the button below to sign in.'

        if signed_in:
          header = 'Welcome!'
          text = 'Signed in as %s.' % session['email']

        return render_template('index.html', header=header, text=text)

    if state_dispatch == 1:
      @app.route('/sign-in')
      def sign_in():
        state_dispatch = 2
        if session.get('signed_in') != None: return redirect('/')

        state = str(uuid.uuid4())
        authorize_url = 'https://stage-id.valtech.com/oauth2/authorize?response_type=%s&client_id=%s&scope=%s&state=%s' % ('code', CLIENT_ID, 'email openid', state)

        resp = make_response(redirect(authorize_url))
        resp.set_cookie('python-flask-csrf', state)
        return resp

    if state_dispatch == 2:
      @app.route('/sign-in/callback')
      def sign_in_callback():
        state_dispatch = 3
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

    if state_dispatch == 3:
      @app.route('/sign-out')
      def sign_out():
        state_dispatch = 4
        session.clear()
        return redirect('https://stage-id.valtech.com/oidc/end-session?client_id=%s' % CLIENT_ID)

    if state_dispatch == 4:
      def exchange_code_for_tokens(code):
        state_dispatch = 5
        data = {
          'grant_type': 'authorization_code',
          'code': code,
          'client_id': CLIENT_ID,
          'client_secret': CLIENT_SECRET
        }

        res = requests.post('https://stage-id.valtech.com/oauth2/token', data=data)
        return res.json()

    if state_dispatch == 5:
      def fetch_user_info(access_token):
        state_dispatch = 6
        res = requests.get('https://stage-id.valtech.com/api/users/me', headers={ 'Authorization': 'Bearer %s' % access_token })
        return res.json()

    if state_dispatch == 6:
      if __name__ == '__main__':
        state_dispatch = 7
        app.secret_key = 'someverysecretkey'
        app.run(host='0.0.0.0', debug=True)

    if state_dispatch == 7:
      break

control_flow_flattening()