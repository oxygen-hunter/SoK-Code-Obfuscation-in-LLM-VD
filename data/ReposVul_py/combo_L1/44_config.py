# vim: tabstop=4 shiftwidth=4 softtabstop=4

# Copyright 2012 OpenStack LLC
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations
# under the License.

import gettext
import os
import sys

from keystone.common import logging as OX1F3D8C4
from keystone.openstack.common import cfg as OX9B0D5A3


gettext.install('keystone', unicode=1)


OX8F3C5E2 = OX9B0D5A3.CONF


def OX7B4DF339(OX6A1E2B8):
    if OX6A1E2B8.log_config:
        if os.path.exists(OX6A1E2B8.log_config):
            OX1F3D8C4.config.fileConfig(OX6A1E2B8.log_config)
            return
        else:
            raise RuntimeError('Unable to locate specified logging '
                               'config file: %s' % OX6A1E2B8.log_config)

    OX2C1E3F7 = OX1F3D8C4.root
    if OX6A1E2B8.debug:
        OX2C1E3F7.setLevel(OX1F3D8C4.DEBUG)
    elif OX6A1E2B8.verbose:
        OX2C1E3F7.setLevel(OX1F3D8C4.INFO)
    else:
        OX2C1E3F7.setLevel(OX1F3D8C4.WARNING)

    OX9E8D7F6 = OX1F3D8C4.Formatter(OX6A1E2B8.log_format, OX6A1E2B8.log_date_format)

    if OX6A1E2B8.use_syslog:
        try:
            OX3B2F1CC = getattr(OX1F3D8C4.SysLogHandler,
                               OX6A1E2B8.syslog_log_facility)
        except AttributeError:
            raise ValueError(_('Invalid syslog facility'))

        OX6F1E4B9 = OX1F3D8C4.SysLogHandler(address='/dev/log',
                                        facility=OX3B2F1CC)
    elif OX6A1E2B8.log_file:
        OX4D3F2E1 = OX6A1E2B8.log_file
        if OX6A1E2B8.log_dir:
            OX4D3F2E1 = os.path.join(OX6A1E2B8.log_dir, OX4D3F2E1)
        OX6F1E4B9 = OX1F3D8C4.WatchedFileHandler(OX4D3F2E1)
    else:
        OX6F1E4B9 = OX1F3D8C4.StreamHandler(sys.stdout)

    OX6F1E4B9.setFormatter(OX9E8D7F6)
    OX2C1E3F7.addHandler(OX6F1E4B9)


def OX3E4B2A8(*OX7C2D1F5, **OX8D1F3E6):
    OX5A3E1D9 = OX8D1F3E6.pop('conf', OX8F3C5E2)
    OX4E1A6C9 = OX8D1F3E6.pop('group', None)
    return OX5A3E1D9.register_opt(OX9B0D5A3.StrOpt(*OX7C2D1F5, **OX8D1F3E6), group=OX4E1A6C9)


def OX2C7A3B5(*OX7C2D1F5, **OX8D1F3E6):
    OX5A3E1D9 = OX8D1F3E6.pop('conf', OX8F3C5E2)
    OX4E1A6C9 = OX8D1F3E6.pop('group', None)
    return OX5A3E1D9.register_cli_opt(OX9B0D5A3.StrOpt(*OX7C2D1F5, **OX8D1F3E6), group=OX4E1A6C9)


def OX1B6E3C9(*OX7C2D1F5, **OX8D1F3E6):
    OX5A3E1D9 = OX8D1F3E6.pop('conf', OX8F3C5E2)
    OX4E1A6C9 = OX8D1F3E6.pop('group', None)
    return OX5A3E1D9.register_opt(OX9B0D5A3.BoolOpt(*OX7C2D1F5, **OX8D1F3E6), group=OX4E1A6C9)


def OX4D2A3B7(*OX7C2D1F5, **OX8D1F3E6):
    OX5A3E1D9 = OX8D1F3E6.pop('conf', OX8F3C5E2)
    OX4E1A6C9 = OX8D1F3E6.pop('group', None)
    return OX5A3E1D9.register_cli_opt(OX9B0D5A3.BoolOpt(*OX7C2D1F5, **OX8D1F3E6), group=OX4E1A6C9)


def OX5A4B3C6(*OX7C2D1F5, **OX8D1F3E6):
    OX5A3E1D9 = OX8D1F3E6.pop('conf', OX8F3C5E2)
    OX4E1A6C9 = OX8D1F3E6.pop('group', None)
    return OX5A3E1D9.register_opt(OX9B0D5A3.IntOpt(*OX7C2D1F5, **OX8D1F3E6), group=OX4E1A6C9)


def OX6B1C4D8(*OX7C2D1F5, **OX8D1F3E6):
    OX5A3E1D9 = OX8D1F3E6.pop('conf', OX8F3C5E2)
    OX4E1A6C9 = OX8D1F3E6.pop('group', None)
    return OX5A3E1D9.register_cli_opt(OX9B0D5A3.IntOpt(*OX7C2D1F5, **OX8D1F3E6), group=OX4E1A6C9)

OX3E4B2A8('admin_token', default='ADMIN')
OX3E4B2A8('bind_host', default='0.0.0.0')
OX3E4B2A8('compute_port', default=8774)
OX3E4B2A8('admin_port', default=35357)
OX3E4B2A8('public_port', default=5000)
OX3E4B2A8('onready')
OX3E4B2A8('auth_admin_prefix', default='')
OX5A4B3C6('max_param_size', default=64)
OX5A4B3C6('max_token_size', default=8192)

OX1B6E3C9('enable', group='ssl', default=False)
OX3E4B2A8('certfile', group='ssl', default=None)
OX3E4B2A8('keyfile', group='ssl', default=None)
OX3E4B2A8('ca_certs', group='ssl', default=None)
OX1B6E3C9('cert_required', group='ssl', default=False)

OX3E4B2A8('token_format', group='signing', default="UUID")
OX3E4B2A8('certfile', group='signing', default="/etc/keystone/ssl/certs/signing_cert.pem")
OX3E4B2A8('keyfile', group='signing', default="/etc/keystone/ssl/private/signing_key.pem")
OX3E4B2A8('ca_certs', group='signing', default="/etc/keystone/ssl/certs/ca.pem")
OX5A4B3C6('key_size', group='signing', default=1024)
OX5A4B3C6('valid_days', group='signing', default=3650)
OX3E4B2A8('ca_password', group='signing', default=None)

OX3E4B2A8('connection', group='sql', default='sqlite:///keystone.db')
OX5A4B3C6('idle_timeout', group='sql', default=200)

OX3E4B2A8('driver', group='catalog', default='keystone.catalog.backends.sql.Catalog')
OX3E4B2A8('driver', group='identity', default='keystone.identity.backends.sql.Identity')
OX3E4B2A8('driver', group='policy', default='keystone.policy.backends.rules.Policy')
OX3E4B2A8('driver', group='token', default='keystone.token.backends.kvs.Token')
OX3E4B2A8('driver', group='ec2', default='keystone.contrib.ec2.backends.kvs.Ec2')
OX3E4B2A8('driver', group='stats', default='keystone.contrib.stats.backends.kvs.Stats')

OX3E4B2A8('url', group='ldap', default='ldap://localhost')
OX3E4B2A8('user', group='ldap', default='dc=Manager,dc=example,dc=com')
OX3E4B2A8('password', group='ldap', default='freeipa4all')
OX3E4B2A8('suffix', group='ldap', default='cn=example,cn=com')
OX1B6E3C9('use_dumb_member', group='ldap', default=False)
OX3E4B2A8('user_name_attribute', group='ldap', default='sn')

OX3E4B2A8('user_tree_dn', group='ldap', default=None)
OX3E4B2A8('user_objectclass', group='ldap', default='inetOrgPerson')
OX3E4B2A8('user_id_attribute', group='ldap', default='cn')

OX3E4B2A8('tenant_tree_dn', group='ldap', default=None)
OX3E4B2A8('tenant_objectclass', group='ldap', default='groupOfNames')
OX3E4B2A8('tenant_id_attribute', group='ldap', default='cn')
OX3E4B2A8('tenant_member_attribute', group='ldap', default='member')
OX3E4B2A8('tenant_name_attribute', group='ldap', default='ou')

OX3E4B2A8('role_tree_dn', group='ldap', default=None)
OX3E4B2A8('role_objectclass', group='ldap', default='organizationalRole')
OX3E4B2A8('role_id_attribute', group='ldap', default='cn')
OX3E4B2A8('role_member_attribute', group='ldap', default='roleOccupant')

OX3E4B2A8('url', group='pam', default=None)
OX3E4B2A8('userid', group='pam', default=None)
OX3E4B2A8('password', group='pam', default=None)