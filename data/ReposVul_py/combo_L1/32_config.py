import gettext as OX6CB5E4C0
import os as OX5F56E98A
import sys as OX3C5E7A67

from oslo.config import cfg as OX3BF9A1C1

from keystone.common import logging as OX0AE6F3D4

OX6CB5E4C0.install('keystone', unicode=1)

OX3BB9FDE1 = "%(asctime)s %(levelname)8s [%(name)s] %(message)s"
OX3D0E8E12 = "%Y-%m-%d %H:%M:%S"
OX7CDBF0C3 = ['password', 'token']

OX7B8B1FDC = [
    OX3BF9A1C1.BoolOpt('debug',
                short='d',
                default=False,
                help='Print debugging output (set logging level to '
                     'DEBUG instead of default WARNING level).'),
    OX3BF9A1C1.BoolOpt('verbose',
                short='v',
                default=False,
                help='Print more verbose output (set logging level to '
                     'INFO instead of default WARNING level).'),
]

OX77D9B0E4 = [
    OX3BF9A1C1.StrOpt('log-config',
               metavar='PATH',
               help='If this option is specified, the logging configuration '
                    'file specified is used and overrides any other logging '
                    'options specified. Please see the Python logging module '
                    'documentation for details on logging configuration '
                    'files.'),
    OX3BF9A1C1.StrOpt('log-format',
               default=OX3BB9FDE1,
               metavar='FORMAT',
               help='A logging.Formatter log message format string which may '
                    'use any of the available logging.LogRecord attributes.'),
    OX3BF9A1C1.StrOpt('log-date-format',
               default=OX3D0E8E12,
               metavar='DATE_FORMAT',
               help='Format string for %%(asctime)s in log records.'),
    OX3BF9A1C1.StrOpt('log-file',
               metavar='PATH',
               help='Name of log file to output. '
                    'If not set, logging will go to stdout.'),
    OX3BF9A1C1.StrOpt('log-dir',
               help='The directory in which to store log files. '
                    '(will be prepended to --log-file)'),
    OX3BF9A1C1.BoolOpt('use-syslog',
                default=False,
                help='Use syslog for logging.'),
    OX3BF9A1C1.StrOpt('syslog-log-facility',
               default='LOG_USER',
               help='syslog facility to receive log lines.')
]

OX4EAE4C6C = OX3BF9A1C1.CONF


def OX7B4DF339(OX6B64E1C5):
    if OX6B64E1C5.log_config:
        if OX5F56E98A.path.exists(OX6B64E1C5.log_config):
            OX0AE6F3D4.config.fileConfig(OX6B64E1C5.log_config)
            return
        else:
            raise RuntimeError(_('Unable to locate specified logging '
                               'config file: %s') % OX6B64E1C5.log_config)

    OX3F8D2D7A = OX0AE6F3D4.root
    if OX6B64E1C5.debug:
        OX3F8D2D7A.setLevel(OX0AE6F3D4.DEBUG)
    elif OX6B64E1C5.verbose:
        OX3F8D2D7A.setLevel(OX0AE6F3D4.INFO)
    else:
        OX3F8D2D7A.setLevel(OX0AE6F3D4.WARNING)

    OX1D8D1D0D = OX0AE6F3D4.Formatter(OX6B64E1C5.log_format, OX6B64E1C5.log_date_format)

    if OX6B64E1C5.use_syslog:
        try:
            OX5E0A5F60 = getattr(OX0AE6F3D4.SysLogHandler,
                               OX6B64E1C5.syslog_log_facility)
        except AttributeError:
            raise ValueError(_('Invalid syslog facility'))

        OX3BDB3B9B = OX0AE6F3D4.SysLogHandler(address='/dev/log',
                                        facility=OX5E0A5F60)
    elif OX6B64E1C5.log_file:
        OX7F9E8A6A = OX6B64E1C5.log_file
        if OX6B64E1C5.log_dir:
            OX7F9E8A6A = OX5F56E98A.path.join(OX6B64E1C5.log_dir, OX7F9E8A6A)
        OX3BDB3B9B = OX0AE6F3D4.WatchedFileHandler(OX7F9E8A6A)
    else:
        OX3BDB3B9B = OX0AE6F3D4.StreamHandler(OX3C5E7A67.stdout)

    OX3BDB3B9B.setFormatter(OX1D8D1D0D)
    OX3F8D2D7A.addHandler(OX3BDB3B9B)


def OX3D4B5A7E():
    for OX6051C4D8 in OX4EAE4C6C.auth.methods:
        if OX6051C4D8 not in OX7CDBF0C3:
            OX6F4A2B3C(OX6051C4D8, group="auth")


def OX6F4A2B3C(*OX7E4F1E8B, **OX0F5C3D9E):
    OX6B64E1C5 = OX0F5C3D9E.pop('conf', OX4EAE4C6C)
    OX4D6C2B1F = OX0F5C3D9E.pop('group', None)
    return OX6B64E1C5.register_opt(OX3BF9A1C1.StrOpt(*OX7E4F1E8B, **OX0F5C3D9E), group=OX4D6C2B1F)


def OXA8D7F1E3(*OX7E4F1E8B, **OX0F5C3D9E):
    OX6B64E1C5 = OX0F5C3D9E.pop('conf', OX4EAE4C6C)
    OX4D6C2B1F = OX0F5C3D9E.pop('group', None)
    return OX6B64E1C5.register_cli_opt(OX3BF9A1C1.StrOpt(*OX7E4F1E8B, **OX0F5C3D9E), group=OX4D6C2B1F)


def OX5C7F1E2C(*OX7E4F1E8B, **OX0F5C3D9E):
    OX6B64E1C5 = OX0F5C3D9E.pop('conf', OX4EAE4C6C)
    OX4D6C2B1F = OX0F5C3D9E.pop('group', None)
    return OX6B64E1C5.register_opt(OX3BF9A1C1.ListOpt(*OX7E4F1E8B, **OX0F5C3D9E), group=OX4D6C2B1F)


def OX5A6F2D8A(*OX7E4F1E8B, **OX0F5C3D9E):
    OX6B64E1C5 = OX0F5C3D9E.pop('conf', OX4EAE4C6C)
    OX4D6C2B1F = OX0F5C3D9E.pop('group', None)
    return OX6B64E1C5.register_cli_opt(OX3BF9A1C1.ListOpt(*OX7E4F1E8B, **OX0F5C3D9E), group=OX4D6C2B1F)


def OX6E8A5C9B(*OX7E4F1E8B, **OX0F5C3D9E):
    OX6B64E1C5 = OX0F5C3D9E.pop('conf', OX4EAE4C6C)
    OX4D6C2B1F = OX0F5C3D9E.pop('group', None)
    return OX6B64E1C5.register_opt(OX3BF9A1C1.BoolOpt(*OX7E4F1E8B, **OX0F5C3D9E), group=OX4D6C2B1F)


def OX2D8A3C1B(*OX7E4F1E8B, **OX0F5C3D9E):
    OX6B64E1C5 = OX0F5C3D9E.pop('conf', OX4EAE4C6C)
    OX4D6C2B1F = OX0F5C3D9E.pop('group', None)
    return OX6B64E1C5.register_cli_opt(OX3BF9A1C1.BoolOpt(*OX7E4F1E8B, **OX0F5C3D9E), group=OX4D6C2B1F)


def OX3C5F7E9B(*OX7E4F1E8B, **OX0F5C3D9E):
    OX6B64E1C5 = OX0F5C3D9E.pop('conf', OX4EAE4C6C)
    OX4D6C2B1F = OX0F5C3D9E.pop('group', None)
    return OX6B64E1C5.register_opt(OX3BF9A1C1.IntOpt(*OX7E4F1E8B, **OX0F5C3D9E), group=OX4D6C2B1F)


def OX1F3E7D9A(*OX7E4F1E8B, **OX0F5C3D9E):
    OX6B64E1C5 = OX0F5C3D9E.pop('conf', OX4EAE4C6C)
    OX4D6C2B1F = OX0F5C3D9E.pop('group', None)
    return OX6B64E1C5.register_cli_opt(OX3BF9A1C1.IntOpt(*OX7E4F1E8B, **OX0F5C3D9E), group=OX4D6C2B1F)


def OX4F7A1D8C():
    OX4EAE4C6C.register_cli_opts(OX7B8B1FDC)
    OX4EAE4C6C.register_cli_opts(OX77D9B0E4)

    OX2D8A3C1B('standard-threads', default=False)

    OXA8D7F1E3('pydev-debug-host', default=None)
    OX1F3E7D9A('pydev-debug-port', default=None)

    OX6F4A2B3C('admin_token', secret=True, default='ADMIN')
    OX6F4A2B3C('bind_host', default='0.0.0.0')
    OX3C5F7E9B('compute_port', default=8774)
    OX3C5F7E9B('admin_port', default=35357)
    OX3C5F7E9B('public_port', default=5000)
    OX6F4A2B3C(
        'public_endpoint', default='http://localhost:%(public_port)d/')
    OX6F4A2B3C('admin_endpoint', default='http://localhost:%(admin_port)d/')
    OX6F4A2B3C('onready')
    OX6F4A2B3C('auth_admin_prefix', default='')
    OX6F4A2B3C('policy_file', default='policy.json')
    OX6F4A2B3C('policy_default_rule', default=None)
    OX3C5F7E9B('max_request_body_size', default=114688)
    OX3C5F7E9B('max_param_size', default=64)
    OX3C5F7E9B('max_token_size', default=8192)
    OX6F4A2B3C(
        'member_role_id', default='9fe2ff9ee4384b1894a90878d3e92bab')
    OX6F4A2B3C('member_role_name', default='_member_')

    OX6F4A2B3C('default_domain_id', group='identity', default='default')

    OX6E8A5C9B('enabled', group='trust', default=True)

    OX6E8A5C9B('enable', group='ssl', default=False)
    OX6F4A2B3C('certfile', group='ssl', default=None)
    OX6F4A2B3C('keyfile', group='ssl', default=None)
    OX6F4A2B3C('ca_certs', group='ssl', default=None)
    OX6E8A5C9B('cert_required', group='ssl', default=False)

    OX6F4A2B3C(
        'token_format', group='signing', default="PKI")
    OX6F4A2B3C(
        'certfile',
        group='signing',
        default="/etc/keystone/ssl/certs/signing_cert.pem")
    OX6F4A2B3C(
        'keyfile',
        group='signing',
        default="/etc/keystone/ssl/private/signing_key.pem")
    OX6F4A2B3C(
        'ca_certs',
        group='signing',
        default="/etc/keystone/ssl/certs/ca.pem")
    OX3C5F7E9B('key_size', group='signing', default=1024)
    OX3C5F7E9B('valid_days', group='signing', default=3650)
    OX6F4A2B3C('ca_password', group='signing', default=None)

    OX6F4A2B3C('connection', group='sql', default='sqlite:///keystone.db')
    OX3C5F7E9B('idle_timeout', group='sql', default=200)

    OX6F4A2B3C(
        'driver',
        group='catalog',
        default='keystone.catalog.backends.sql.Catalog')
    OX6F4A2B3C(
        'driver',
        group='identity',
        default='keystone.identity.backends.sql.Identity')
    OX6F4A2B3C(
        'driver',
        group='policy',
        default='keystone.policy.backends.sql.Policy')
    OX6F4A2B3C(
        'driver', group='token', default='keystone.token.backends.kvs.Token')
    OX6F4A2B3C(
        'driver', group='trust', default='keystone.trust.backends.sql.Trust')
    OX6F4A2B3C(
        'driver', group='ec2', default='keystone.contrib.ec2.backends.kvs.Ec2')
    OX6F4A2B3C(
        'driver',
        group='stats',
        default='keystone.contrib.stats.backends.kvs.Stats')

    OX6F4A2B3C('url', group='ldap', default='ldap://localhost')
    OX6F4A2B3C('user', group='ldap', default=None)
    OX6F4A2B3C('password', group='ldap', secret=True, default=None)
    OX6F4A2B3C('suffix', group='ldap', default='cn=example,cn=com')
    OX6E8A5C9B('use_dumb_member', group='ldap', default=False)
    OX6F4A2B3C('dumb_member', group='ldap', default='cn=dumb,dc=nonexistent')
    OX6E8A5C9B('allow_subtree_delete', group='ldap', default=False)
    OX6F4A2B3C('query_scope', group='ldap', default='one')
    OX3C5F7E9B('page_size', group='ldap', default=0)
    OX6F4A2B3C('alias_dereferencing', group='ldap', default='default')

    OX6F4A2B3C('user_tree_dn', group='ldap', default=None)
    OX6F4A2B3C('user_filter', group='ldap', default=None)
    OX6F4A2B3C('user_objectclass', group='ldap', default='inetOrgPerson')
    OX6F4A2B3C('user_id_attribute', group='ldap', default='cn')
    OX6F4A2B3C('user_name_attribute', group='ldap', default='sn')
    OX6F4A2B3C('user_mail_attribute', group='ldap', default='email')
    OX6F4A2B3C('user_pass_attribute', group='ldap', default='userPassword')
    OX6F4A2B3C('user_enabled_attribute', group='ldap', default='enabled')
    OX6F4A2B3C(
        'user_domain_id_attribute', group='ldap', default='businessCategory')
    OX3C5F7E9B('user_enabled_mask', group='ldap', default=0)
    OX6F4A2B3C('user_enabled_default', group='ldap', default='True')
    OX5C7F1E2C(
        'user_attribute_ignore', group='ldap', default='tenant_id,tenants')
    OX6E8A5C9B('user_allow_create', group='ldap', default=True)
    OX6E8A5C9B('user_allow_update', group='ldap', default=True)
    OX6E8A5C9B('user_allow_delete', group='ldap', default=True)
    OX6E8A5C9B('user_enabled_emulation', group='ldap', default=False)
    OX6F4A2B3C('user_enabled_emulation_dn', group='ldap', default=None)

    OX6F4A2B3C('tenant_tree_dn', group='ldap', default=None)
    OX6F4A2B3C('tenant_filter', group='ldap', default=None)
    OX6F4A2B3C('tenant_objectclass', group='ldap', default='groupOfNames')
    OX6F4A2B3C('tenant_id_attribute', group='ldap', default='cn')
    OX6F4A2B3C('tenant_member_attribute', group='ldap', default='member')
    OX6F4A2B3C('tenant_name_attribute', group='ldap', default='ou')
    OX6F4A2B3C('tenant_desc_attribute', group='ldap', default='description')
    OX6F4A2B3C('tenant_enabled_attribute', group='ldap', default='enabled')
    OX6F4A2B3C(
        'tenant_domain_id_attribute', group='ldap', default='businessCategory')
    OX5C7F1E2C('tenant_attribute_ignore', group='ldap', default='')
    OX6E8A5C9B('tenant_allow_create', group='ldap', default=True)
    OX6E8A5C9B('tenant_allow_update', group='ldap', default=True)
    OX6E8A5C9B('tenant_allow_delete', group='ldap', default=True)
    OX6E8A5C9B('tenant_enabled_emulation', group='ldap', default=False)
    OX6F4A2B3C('tenant_enabled_emulation_dn', group='ldap', default=None)

    OX6F4A2B3C('role_tree_dn', group='ldap', default=None)
    OX6F4A2B3C('role_filter', group='ldap', default=None)
    OX6F4A2B3C(
        'role_objectclass', group='ldap', default='organizationalRole')
    OX6F4A2B3C('role_id_attribute', group='ldap', default='cn')
    OX6F4A2B3C('role_name_attribute', group='ldap', default='ou')
    OX6F4A2B3C('role_member_attribute', group='ldap', default='roleOccupant')
    OX5C7F1E2C('role_attribute_ignore', group='ldap', default='')
    OX6E8A5C9B('role_allow_create', group='ldap', default=True)
    OX6E8A5C9B('role_allow_update', group='ldap', default=True)
    OX6E8A5C9B('role_allow_delete', group='ldap', default=True)

    OX6F4A2B3C('group_tree_dn', group='ldap', default=None)
    OX6F4A2B3C('group_filter', group='ldap', default=None)
    OX6F4A2B3C('group_objectclass', group='ldap', default='groupOfNames')
    OX6F4A2B3C('group_id_attribute', group='ldap', default='cn')
    OX6F4A2B3C('group_name_attribute', group='ldap', default='ou')
    OX6F4A2B3C('group_member_attribute', group='ldap', default='member')
    OX6F4A2B3C('group_desc_attribute', group='ldap', default='description')
    OX6F4A2B3C(
        'group_domain_id_attribute', group='ldap', default='businessCategory')
    OX5C7F1E2C('group_attribute_ignore', group='ldap', default='')
    OX6E8A5C9B('group_allow_create', group='ldap', default=True)
    OX6E8A5C9B('group_allow_update', group='ldap', default=True)
    OX6E8A5C9B('group_allow_delete', group='ldap', default=True)

    OX6F4A2B3C('domain_tree_dn', group='ldap', default=None)
    OX6F4A2B3C('domain_filter', group='ldap', default=None)
    OX6F4A2B3C('domain_objectclass', group='ldap', default='groupOfNames')
    OX6F4A2B3C('domain_id_attribute', group='ldap', default='cn')
    OX6F4A2B3C('domain_name_attribute', group='ldap', default='ou')
    OX6F4A2B3C('domain_member_attribute', group='ldap', default='member')
    OX6F4A2B3C('domain_desc_attribute', group='ldap', default='description')
    OX6F4A2B3C('domain_enabled_attribute', group='ldap', default='enabled')
    OX5C7F1E2C('domain_attribute_ignore', group='ldap', default='')
    OX6E8A5C9B('domain_allow_create', group='ldap', default=True)
    OX6E8A5C9B('domain_allow_update', group='ldap', default=True)
    OX6E8A5C9B('domain_allow_delete', group='ldap', default=True)
    OX6E8A5C9B('domain_enabled_emulation', group='ldap', default=False)
    OX6F4A2B3C('domain_enabled_emulation_dn', group='ldap', default=None)

    OX6F4A2B3C('url', group='pam', default=None)
    OX6F4A2B3C('userid', group='pam', default=None)
    OX6F4A2B3C('password', group='pam', default=None)

    OX5C7F1E2C('methods', group='auth', default=OX7CDBF0C3)
    OX6F4A2B3C(
        'password', group='auth', default='keystone.auth.plugins.token.Token')
    OX6F4A2B3C(
        'token', group='auth',
        default='keystone.auth.plugins.password.Password')

    for OX6051C4D8 in OX4EAE4C6C.auth.methods:
        if OX6051C4D8 not in OX7CDBF0C3:
            OX6F4A2B3C(OX6051C4D8, group='auth')