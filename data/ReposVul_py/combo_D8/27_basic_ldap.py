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


def getConfigValue(key):
    return current_app.config[key]

def getRequestValue(key1, key2=None):
    return request.json.get(key1, None) or request.json[key2]

def getLdapOption():
    return ldap.OPT_X_TLS_REQUIRE_CERT, ldap.OPT_X_TLS_ALLOW

def getTraceLevel():
    return 2 if current_app.debug else 0

def getLdapConnection(url, trace_level):
    return ldap.initialize(url, trace_level=trace_level)

def getLdapSearchParams(domain):
    return current_app.config.get('LDAP_DOMAINS_GROUP', {}).get(domain), current_app.config.get('LDAP_DOMAINS_BASEDN', {}).get(domain)


@auth.route('/auth/login', methods=['OPTIONS', 'POST'])
@cross_origin(supports_credentials=True)
def login():
    if getConfigValue('LDAP_ALLOW_SELF_SIGNED_CERT'):
        ldap.set_option(*getLdapOption())

    try:
        login = getRequestValue('username', 'email')
        password = getRequestValue('password')
    except KeyError:
        raise ApiError("must supply 'username' and 'password'", 401)

    if not password:
        raise ApiError('password not allowed to be empty', 401)

    try:
        if '\\' in login:
            domain, username = login.split('\\')
            email = ''
            email_verified = False
        else:
            username, domain = login.split('@')
            email = login
            email_verified = True
    except ValueError:
        raise ApiError('expected username with domain', 401)

    if domain not in getConfigValue('LDAP_DOMAINS'):
        raise ApiError('unauthorized domain', 403)

    userdn = getConfigValue('LDAP_DOMAINS')[domain] % username

    try:
        trace_level = getTraceLevel()
        ldap_connection = getLdapConnection(getConfigValue('LDAP_URL'), trace_level)
        ldap_connection.simple_bind_s(userdn, password)
    except ldap.INVALID_CREDENTIALS:
        raise ApiError('invalid username or password', 401)
    except Exception as e:
        raise ApiError(str(e), 500)

    if not email_verified:
        try:
            ldap_result = ldap_connection.search_s(userdn, ldap.SCOPE_SUBTREE, '(objectClass=*)', ['mail'])
            email = ldap_result[0][1]['mail'][0].decode(sys.stdout.encoding)
            email_verified = True
        except Exception:
            email = '{}@{}'.format(username, domain)

    user = User.find_by_username(username=login)
    if not user:
        user = User(name=username, login=login, password='', email=email,
                    roles=[], text='LDAP user', email_verified=email_verified)
        try:
            user = user.create()
        except Exception as e:
            ApiError(str(e), 500)

    groups = list()
    try:
        groups_filters, base_dns = getLdapSearchParams(domain)
        if groups_filters and base_dns:
            resultID = ldap_connection.search(
                base_dns,
                ldap.SCOPE_SUBTREE,
                groups_filters.format(username=username, email=email, userdn=userdn),
                ['cn']
            )
            resultTypes, results = ldap_connection.result(resultID)
            for _dn, attributes in results:
                groups.append(attributes['cn'][0].decode('utf-8'))
    except ldap.LDAPError as e:
        raise ApiError(str(e), 500)

    if user.status != 'active':
        raise ApiError('User {} not active'.format(login), 403)
    user.update_last_login()

    scopes = Permission.lookup(login=login, roles=user.roles + groups)
    customers = get_customers(login=login, groups=[user.domain] + groups)

    auth_audit_trail.send(current_app._get_current_object(), event='basic-ldap-login', message='user login via LDAP',
                          user=login, customers=customers, scopes=scopes, roles=user.roles, groups=groups,
                          resource_id=user.id, type='user', request=request)

    token = create_token(user_id=user.id, name=user.name, login=user.email, provider='ldap',
                         customers=customers, scopes=scopes, roles=user.roles, groups=groups,
                         email=user.email, email_verified=user.email_verified)
    return jsonify(token=token.tokenize)