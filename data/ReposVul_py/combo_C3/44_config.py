import gettext
import os
import sys
from keystone.common import logging
from keystone.openstack.common import cfg

gettext.install('keystone', unicode=1)
CONF = cfg.CONF

class StackVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
    
    def run(self, instructions):
        self.pc = 0
        self.instructions = instructions
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            getattr(self, instr[0])(*instr[1:])
            self.pc += 1
    
    def PUSH(self, value):
        self.stack.append(value)
    
    def POP(self):
        return self.stack.pop()
    
    def ADD(self):
        self.PUSH(self.POP() + self.POP())
    
    def SUB(self):
        b, a = self.POP(), self.POP()
        self.PUSH(a - b)
    
    def JMP(self, address):
        self.pc = address - 1
    
    def JZ(self, address):
        if self.POP() == 0:
            self.pc = address - 1
    
    def LOAD(self, var):
        self.PUSH(var)
    
    def STORE(self, var):
        var[0] = self.POP()
    
    def CALL_FUNC(self, func, *args):
        func(*args)

def obfuscated_setup_logging(conf):
    vm = StackVM()
    vm.run([
        ('LOAD', [conf.log_config]),
        ('PUSH', None),
        ('CALL_FUNC', obfuscated_logging_logic, conf)
    ])

def obfuscated_logging_logic(conf):
    vm = StackVM()
    vm.run([
        ('LOAD', [conf.log_config]),
        ('PUSH', os.path.exists),
        ('CALL_FUNC', call_func),
        ('JZ', 4),
        ('LOAD', [conf.log_config]),
        ('CALL_FUNC', logging.config.fileConfig),
        ('JMP', 17),
        ('PUSH', RuntimeError('Unable to locate specified logging config file: %s' % conf.log_config)),
        ('CALL_FUNC', raise_exception),
        ('LOAD', [conf]),
        ('PUSH', logging.root),
        ('CALL_FUNC', logging_level_logic)
    ])

def logging_level_logic(conf, root_logger):
    vm = StackVM()
    vm.run([
        ('LOAD', [conf.debug]),
        ('JZ', 6),
        ('LOAD', [logging.DEBUG]),
        ('CALL_FUNC', root_logger.setLevel),
        ('JMP', 14),
        ('LOAD', [conf.verbose]),
        ('JZ', 10),
        ('LOAD', [logging.INFO]),
        ('CALL_FUNC', root_logger.setLevel),
        ('JMP', 14),
        ('LOAD', [logging.WARNING]),
        ('CALL_FUNC', root_logger.setLevel)
    ])

def call_func(func, *args):
    func(*args)

def raise_exception(exc):
    raise exc

def register_str(*args, **kw):
    conf = kw.pop('conf', CONF)
    group = kw.pop('group', None)
    return conf.register_opt(cfg.StrOpt(*args, **kw), group=group)

def register_cli_str(*args, **kw):
    conf = kw.pop('conf', CONF)
    group = kw.pop('group', None)
    return conf.register_cli_opt(cfg.StrOpt(*args, **kw), group=group)

def register_bool(*args, **kw):
    conf = kw.pop('conf', CONF)
    group = kw.pop('group', None)
    return conf.register_opt(cfg.BoolOpt(*args, **kw), group=group)

def register_cli_bool(*args, **kw):
    conf = kw.pop('conf', CONF)
    group = kw.pop('group', None)
    return conf.register_cli_opt(cfg.BoolOpt(*args, **kw), group=group)

def register_int(*args, **kw):
    conf = kw.pop('conf', CONF)
    group = kw.pop('group', None)
    return conf.register_opt(cfg.IntOpt(*args, **kw), group=group)

def register_cli_int(*args, **kw):
    conf = kw.pop('conf', CONF)
    group = kw.pop('group', None)
    return conf.register_cli_opt(cfg.IntOpt(*args, **kw), group=group)

register_str('admin_token', default='ADMIN')
register_str('bind_host', default='0.0.0.0')
register_str('compute_port', default=8774)
register_str('admin_port', default=35357)
register_str('public_port', default=5000)
register_str('onready')
register_str('auth_admin_prefix', default='')
register_int('max_param_size', default=64)
register_int('max_token_size', default=8192)

register_bool('enable', group='ssl', default=False)
register_str('certfile', group='ssl', default=None)
register_str('keyfile', group='ssl', default=None)
register_str('ca_certs', group='ssl', default=None)
register_bool('cert_required', group='ssl', default=False)
register_str('token_format', group='signing', default="UUID")
register_str('certfile', group='signing', default="/etc/keystone/ssl/certs/signing_cert.pem")
register_str('keyfile', group='signing', default="/etc/keystone/ssl/private/signing_key.pem")
register_str('ca_certs', group='signing', default="/etc/keystone/ssl/certs/ca.pem")
register_int('key_size', group='signing', default=1024)
register_int('valid_days', group='signing', default=3650)
register_str('ca_password', group='signing', default=None)

register_str('connection', group='sql', default='sqlite:///keystone.db')
register_int('idle_timeout', group='sql', default=200)

register_str('driver', group='catalog', default='keystone.catalog.backends.sql.Catalog')
register_str('driver', group='identity', default='keystone.identity.backends.sql.Identity')
register_str('driver', group='policy', default='keystone.policy.backends.rules.Policy')
register_str('driver', group='token', default='keystone.token.backends.kvs.Token')
register_str('driver', group='ec2', default='keystone.contrib.ec2.backends.kvs.Ec2')
register_str('driver', group='stats', default='keystone.contrib.stats.backends.kvs.Stats')

register_str('url', group='ldap', default='ldap://localhost')
register_str('user', group='ldap', default='dc=Manager,dc=example,dc=com')
register_str('password', group='ldap', default='freeipa4all')
register_str('suffix', group='ldap', default='cn=example,cn=com')
register_bool('use_dumb_member', group='ldap', default=False)
register_str('user_name_attribute', group='ldap', default='sn')

register_str('user_tree_dn', group='ldap', default=None)
register_str('user_objectclass', group='ldap', default='inetOrgPerson')
register_str('user_id_attribute', group='ldap', default='cn')

register_str('tenant_tree_dn', group='ldap', default=None)
register_str('tenant_objectclass', group='ldap', default='groupOfNames')
register_str('tenant_id_attribute', group='ldap', default='cn')
register_str('tenant_member_attribute', group='ldap', default='member')
register_str('tenant_name_attribute', group='ldap', default='ou')

register_str('role_tree_dn', group='ldap', default=None)
register_str('role_objectclass', group='ldap', default='organizationalRole')
register_str('role_id_attribute', group='ldap', default='cn')
register_str('role_member_attribute', group='ldap', default='roleOccupant')

register_str('url', group='pam', default=None)
register_str('userid', group='pam', default=None)
register_str('password', group='pam', default=None)