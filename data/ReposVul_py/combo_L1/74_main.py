from flask import Flask as OX2DAF3E1A, render_template as OX5E2F8C3D, request as OX8F2B1C4D, session as OX4E7A9D3B, redirect as OX1D6F4B3D, make_response as OX7C3A6B2F
import os as OXA1B2C3D, sys as OX3E4D5F6A
import requests as OX9B8A7C6D
import jwt as OX6C5D4E3F
import uuid as OX3C2D1E0F

OX5F4E3D2C = 'valtech.idp.testclient.local'
OX7E6D5C4B = OXA1B2C3D.environ.get('CLIENT_SECRET')

if OX7E6D5C4B is None:
  print 'CLIENT_SECRET missing. Start using "CLIENT_SECRET=very_secret_secret python main.py"'
  OX3E4D5F6A.exit(-1)

OX2E1F0D3C = OX2DAF3E1A(__name__, static_url_path='')

@OX2E1F0D3C.route('/')
def OX3D4C5B6A():
  OX6F5E4D3C = OX4E7A9D3B.get('signed_in') != None
  OX1F0E9D8C = 'Not signed in'
  OX2E7D6C5B = 'Click the button below to sign in.'

  if OX6F5E4D3C:
    OX1F0E9D8C = 'Welcome!'
    OX2E7D6C5B = 'Signed in as %s.' % OX4E7A9D3B['email']

  return OX5E2F8C3D('index.html', header=OX1F0E9D8C, text=OX2E7D6C5B)

@OX2E1F0D3C.route('/sign-in')
def OX5A6B7C8D():
  if OX4E7A9D3B.get('signed_in') != None: return OX1D6F4B3D('/')

  OX1A2B3C4D = str(OX3C2D1E0F.uuid4())

  OX9A8B7C6D = 'https://stage-id.valtech.com/oauth2/authorize?response_type=%s&client_id=%s&scope=%s&state=%s' % ('code', OX5F4E3D2C, 'email openid', OX1A2B3C4D)

  OX0A9B8C7D = OX7C3A6B2F(OX1D6F4B3D(OX9A8B7C6D))
  OX0A9B8C7D.set_cookie('python-flask-csrf', OX1A2B3C4D)
  return OX0A9B8C7D

@OX2E1F0D3C.route('/sign-in/callback')
def OX1B2C3D4E():
  OX3E4F5A6B = OX8F2B1C4D.args.get('code')
  OX4D5E6F7A = OX8F2B1C4D.args.get('state')

  if OX4D5E6F7A != OX8F2B1C4D.cookies.get('python-flask-csrf'):
    raise Exception("Possible CSRF detected (state does not match stored state)")

  OX7B8A9C0D = OX2C3D4E5F(OX3E4F5A6B)

  OX6B5A4C3D = OX6C5D4E3F.decode(OX7B8A9C0D["id_token"], verify=False)

  OX4E7A9D3B['signed_in'] = True
  OX4E7A9D3B['email'] = OX6B5A4C3D['email']

  OX0D9C8B7A = OX7C3A6B2F(OX1D6F4B3D('/'))
  OX0D9C8B7A.set_cookie('python-flask-csrf', '', expires=0)
  return OX0D9C8B7A

@OX2E1F0D3C.route('/sign-out')
def OX8C7B6A5D():
  OX4E7A9D3B.clear()
  return OX1D6F4B3D('https://stage-id.valtech.com/oidc/end-session?client_id=%s' % OX5F4E3D2C)

def OX2C3D4E5F(OX0E9D8C7B):
  OX1F0E1D2C = {
    'grant_type': 'authorization_code',
    'code': OX0E9D8C7B,
    'client_id': OX5F4E3D2C,
    'client_secret': OX7E6D5C4B
  }

  OX8A7B6C5D = OX9B8A7C6D.post('https://stage-id.valtech.com/oauth2/token', data=OX1F0E1D2C)
  return OX8A7B6C5D.json()

def OX4F5E6A7B(OX7A6B5C4D):
  OX3C2D1E0F = OX9B8A7C6D.get('https://stage-id.valtech.com/api/users/me', headers={ 'Authorization': 'Bearer %s' % OX7A6B5C4D })
  return OX3C2D1E0F.json()

if __name__ == '__main__':
  OX2E1F0D3C.secret_key = 'someverysecretkey'
  OX2E1F0D3C.run(host='0.0.0.0', debug=True)