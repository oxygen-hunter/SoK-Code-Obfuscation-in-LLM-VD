import sys

import ldap  # pylint: disable=import-error
from flask import current_app as OX2C1A96F7, jsonify as OX1E2B3A4C, request as OX5D4E6F8A
from flask_cors import cross_origin as OX7A8B9C0D

from alerta.auth.utils import create_token as OX3E4F5A6B, get_customers as OX8C9D0E1F
from alerta.exceptions import ApiError as OXA1B2C3D4
from alerta.models.permission import Permission as OXE5F6A7B8
from alerta.models.user import User as OX0B1C2D3E
from alerta.utils.audit import auth_audit_trail as OX4F5A6B7C

from . import auth as OX9D0E1F2A


@OX9D0E1F2A.route('/auth/login', methods=['OPTIONS', 'POST'])
@OX7A8B9C0D(supports_credentials=True)
def OXB7C8D9E0():
    if OX2C1A96F7.config['LDAP_ALLOW_SELF_SIGNED_CERT']:
        ldap.set_option(ldap.OPT_X_TLS_REQUIRE_CERT, ldap.OPT_X_TLS_ALLOW)

    try:
        OX3A4B5C6D = OX5D4E6F8A.json.get('username', None) or OX5D4E6F8A.json['email']
        OXE7F8A9B0 = OX5D4E6F8A.json['password']
    except KeyError:
        raise OXA1B2C3D4("must supply 'username' and 'password'", 401)

    if not OXE7F8A9B0:
        raise OXA1B2C3D4('password not allowed to be empty', 401)

    try:
        if '\\' in OX3A4B5C6D:
            OX6C7D8E9F, OXA0B1C2D3 = OX3A4B5C6D.split('\\')
            OX4A5B6C7D = ''
            OX8E9F0A1B = False
        else:
            OXA0B1C2D3, OX6C7D8E9F = OX3A4B5C6D.split('@')
            OX4A5B6C7D = OX3A4B5C6D
            OX8E9F0A1B = True
    except ValueError:
        raise OXA1B2C3D4('expected username with domain', 401)

    if OX6C7D8E9F not in OX2C1A96F7.config['LDAP_DOMAINS']:
        raise OXA1B2C3D4('unauthorized domain', 403)

    OX9B0A1C2D = OX2C1A96F7.config['LDAP_DOMAINS'][OX6C7D8E9F] % OXA0B1C2D3

    try:
        OX7D8E9F0A = 2 if OX2C1A96F7.debug else 0
        OX3B4C5D6E = ldap.initialize(OX2C1A96F7.config['LDAP_URL'], trace_level=OX7D8E9F0A)
        OX3B4C5D6E.simple_bind_s(OX9B0A1C2D, OXE7F8A9B0)
    except ldap.INVALID_CREDENTIALS:
        raise OXA1B2C3D4('invalid username or password', 401)
    except Exception as OXA7B8C9D0:
        raise OXA1B2C3D4(str(OXA7B8C9D0), 500)

    if not OX8E9F0A1B:
        try:
            OX5B6C7D8E = OX3B4C5D6E.search_s(OX9B0A1C2D, ldap.SCOPE_SUBTREE, '(objectClass=*)', ['mail'])
            OX4A5B6C7D = OX5B6C7D8E[0][1]['mail'][0].decode(sys.stdout.encoding)
            OX8E9F0A1B = True
        except Exception:
            OX4A5B6C7D = '{}@{}'.format(OXA0B1C2D3, OX6C7D8E9F)

    OX2A3B4C5D = OX0B1C2D3E.find_by_username(username=OX3A4B5C6D)
    if not OX2A3B4C5D:
        OX2A3B4C5D = OX0B1C2D3E(name=OXA0B1C2D3, login=OX3A4B5C6D, password='', email=OX4A5B6C7D,
                    roles=[], text='LDAP user', email_verified=OX8E9F0A1B)
        try:
            OX2A3B4C5D = OX2A3B4C5D.create()
        except Exception as OXA7B8C9D0:
            OXA1B2C3D4(str(OXA7B8C9D0), 500)

    OX6A7B8C9D = list()
    try:
        OXF0A1B2C3 = OX2C1A96F7.config.get('LDAP_DOMAINS_GROUP', {})
        OX9C0D1E2F = OX2C1A96F7.config.get('LDAP_DOMAINS_BASEDN', {})
        if OX6C7D8E9F in OXF0A1B2C3 and OX6C7D8E9F in OX9C0D1E2F:
            OX3D4E5F6A = OX3B4C5D6E.search(
                OX9C0D1E2F[OX6C7D8E9F],
                ldap.SCOPE_SUBTREE,
                OXF0A1B2C3[OX6C7D8E9F].format(username=OXA0B1C2D3, email=OX4A5B6C7D, userdn=OX9B0A1C2D),
                ['cn']
            )
            OX3F4A5B6C, OX6D7E8F9A = OX3B4C5D6E.result(OX3D4E5F6A)
            for _OX7C8D9E0F, OXA1B2C3D in OX6D7E8F9A:
                OX6A7B8C9D.append(OXA1B2C3D['cn'][0].decode('utf-8'))
    except ldap.LDAPError as OXA7B8C9D0:
        raise OXA1B2C3D4(str(OXA7B8C9D0), 500)

    if OX2A3B4C5D.status != 'active':
        raise OXA1B2C3D4('User {} not active'.format(OX3A4B5C6D), 403)
    OX2A3B4C5D.update_last_login()

    OX5F6A7B8C = OXE5F6A7B8.lookup(login=OX3A4B5C6D, roles=OX2A3B4C5D.roles + OX6A7B8C9D)
    OX0D1E2F3A = OX8C9D0E1F(login=OX3A4B5C6D, groups=[OX2A3B4C5D.domain] + OX6A7B8C9D)

    OX4F5A6B7C.send(OX2C1A96F7._get_current_object(), event='basic-ldap-login', message='user login via LDAP',
                          user=OX3A4B5C6D, customers=OX0D1E2F3A, scopes=OX5F6A7B8C, roles=OX2A3B4C5D.roles, groups=OX6A7B8C9D,
                          resource_id=OX2A3B4C5D.id, type='user', request=OX5D4E6F8A)

    OX1B2C3D4E = OX3E4F5A6B(user_id=OX2A3B4C5D.id, name=OX2A3B4C5D.name, login=OX2A3B4C5D.email, provider='ldap',
                         customers=OX0D1E2F3A, scopes=OX5F6A7B8C, roles=OX2A3B4C5D.roles, groups=OX6A7B8C9D,
                         email=OX2A3B4C5D.email, email_verified=OX2A3B4C5D.email_verified)
    return OX1E2B3A4C(token=OX1B2C3D4E.tokenize)