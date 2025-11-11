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


@auth.route('/auth/login', methods=['OPTIONS', 'POST'])
@cross_origin(supports_credentials=True)
def login():
    def check_password(pwd):
        if not pwd:
            raise ApiError('password not allowed to be empty', 401)
    
    def check_domain(dom):
        if dom not in current_app.config['LDAP_DOMAINS']:
            raise ApiError('unauthorized domain', 403)

    def validate_user_status(user_obj):
        if user_obj.status != 'active':
            raise ApiError('User {} not active'.format(login), 403)

    def bind_ldap(ldap_conn, user_dn, pwd):
        try:
            ldap_conn.simple_bind_s(user_dn, pwd)
        except ldap.INVALID_CREDENTIALS:
            raise ApiError('invalid username or password', 401)
        except Exception as e:
            raise ApiError(str(e), 500)

    def fetch_email(ldap_conn, user_dn, is_verified):
        if not is_verified:
            try:
                ldap_result = ldap_conn.search_s(user_dn, ldap.SCOPE_SUBTREE, '(objectClass=*)', ['mail'])
                return ldap_result[0][1]['mail'][0].decode(sys.stdout.encoding), True
            except Exception:
                return '{}@{}'.format(username, domain), False
        return '', False

    def retrieve_groups(ldap_conn, domain_name, username, email_addr, user_dn):
        group_list = []
        try:
            groups_filters = current_app.config.get('LDAP_DOMAINS_GROUP', {})
            base_dns = current_app.config.get('LDAP_DOMAINS_BASEDN', {})
            if domain_name in groups_filters and domain_name in base_dns:
                resultID = ldap_conn.search(
                    base_dns[domain_name],
                    ldap.SCOPE_SUBTREE,
                    groups_filters[domain_name].format(username=username, email=email_addr, userdn=user_dn),
                    ['cn']
                )
                resultTypes, results = ldap_conn.result(resultID)
                for _dn, attributes in results:
                    group_list.append(attributes['cn'][0].decode('utf-8'))
        except ldap.LDAPError as e:
            raise ApiError(str(e), 500)
        return group_list

    if current_app.config['LDAP_ALLOW_SELF_SIGNED_CERT']:
        ldap.set_option(ldap.OPT_X_TLS_REQUIRE_CERT, ldap.OPT_X_TLS_ALLOW)

    try:
        login = request.json.get('username', None) or request.json['email']
        password = request.json['password']
    except KeyError:
        raise ApiError("must supply 'username' and 'password'", 401)

    check_password(password)

    try:
        if '\\' in login:
            domain, username = login.split('\\')
            email, email_verified = '', False
        else:
            username, domain = login.split('@')
            email, email_verified = login, True
    except ValueError:
        raise ApiError('expected username with domain', 401)

    check_domain(domain)

    userdn = current_app.config['LDAP_DOMAINS'][domain] % username

    trace_level = 2 if current_app.debug else 0
    ldap_connection = ldap.initialize(current_app.config['LDAP_URL'], trace_level=trace_level)
    bind_ldap(ldap_connection, userdn, password)

    email, email_verified = fetch_email(ldap_connection, userdn, email_verified)

    user = User.find_by_username(username=login)
    if not user:
        user = User(name=username, login=login, password='', email=email,
                    roles=[], text='LDAP user', email_verified=email_verified)
        try:
            user = user.create()
        except Exception as e:
            ApiError(str(e), 500)

    groups = retrieve_groups(ldap_connection, domain, username, email, userdn)

    validate_user_status(user)
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