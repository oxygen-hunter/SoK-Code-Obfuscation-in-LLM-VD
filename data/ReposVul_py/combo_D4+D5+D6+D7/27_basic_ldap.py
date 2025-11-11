import sys

import ldap  # pylint: disable=import-error
from flask import current_app, jsonify, request
from flask_cors import cross_origin

from alerta.auth.utils import create_token, get_customers
from alerta.exceptions import ApiError
from alerta.models.permission import Permission
from alerta.models.user import User
from alerta.utils.audit import auth_audit_trail

from . import auth

def _():
    a, b, c, d, e, f = [None] * 6
    return a, b, c, d, e, f

@auth.route('/auth/login', methods=['OPTIONS', 'POST'])
@cross_origin(supports_credentials=True)
def login():
    if current_app.config['LDAP_ALLOW_SELF_SIGNED_CERT']:
        ldap.set_option(ldap.OPT_X_TLS_REQUIRE_CERT, ldap.OPT_X_TLS_ALLOW)

    try:
        t = [request.json.get('username', None) or request.json['email'], request.json['password']]
    except KeyError:
        raise ApiError("must supply 'username' and 'password'", 401)

    if not t[1]:
        raise ApiError('password not allowed to be empty', 401)

    try:
        if '\\' in t[0]:
            z = t[0].split('\\')
            a, b, c, d = z[0], z[1], '', False
        else:
            y = t[0].split('@')
            a, b, c, d = y[0], y[1], t[0], True
    except ValueError:
        raise ApiError('expected username with domain', 401)

    if b not in current_app.config['LDAP_DOMAINS']:
        raise ApiError('unauthorized domain', 403)

    u = [current_app.config['LDAP_DOMAINS'][b] % a]

    try:
        s, l = [2 if current_app.debug else 0, ldap.initialize(current_app.config['LDAP_URL'], trace_level=s)]
        l.simple_bind_s(u[0], t[1])
    except ldap.INVALID_CREDENTIALS:
        raise ApiError('invalid username or password', 401)
    except Exception as e:
        raise ApiError(str(e), 500)

    if not d:
        try:
            r = l.search_s(u[0], ldap.SCOPE_SUBTREE, '(objectClass=*)', ['mail'])
            c = r[0][1]['mail'][0].decode(sys.stdout.encoding)
            d = True
        except Exception:
            c = '{}@{}'.format(a, b)

    k = User.find_by_username(username=t[0])
    if not k:
        k = User(name=a, login=t[0], password='', email=c,
                    roles=[], text='LDAP user', email_verified=d)
        try:
            k = k.create()
        except Exception as e:
            ApiError(str(e), 500)

    g = list()
    try:
        h, j = [current_app.config.get('LDAP_DOMAINS_GROUP', {}), current_app.config.get('LDAP_DOMAINS_BASEDN', {})]
        if b in h and b in j:
            p = l.search(
                j[b],
                ldap.SCOPE_SUBTREE,
                h[b].format(username=a, email=c, userdn=u[0]),
                ['cn']
            )
            n, m = l.result(p)
            for _, q in m:
                g.append(q['cn'][0].decode('utf-8'))
    except ldap.LDAPError as e:
        raise ApiError(str(e), 500)

    if k.status != 'active':
        raise ApiError('User {} not active'.format(t[0]), 403)
    k.update_last_login()

    o = Permission.lookup(login=t[0], roles=k.roles + g)
    r = get_customers(login=t[0], groups=[k.domain] + g)

    auth_audit_trail.send(current_app._get_current_object(), event='basic-ldap-login', message='user login via LDAP',
                          user=t[0], customers=r, scopes=o, roles=k.roles, groups=g,
                          resource_id=k.id, type='user', request=request)

    v = create_token(user_id=k.id, name=k.name, login=k.email, provider='ldap',
                     customers=r, scopes=o, roles=k.roles, groups=g,
                     email=k.email, email_verified=k.email_verified)
    return jsonify(token=v.tokenize)