import uuid
from keystone import catalog
from keystone.common import manager
from keystone.common import utils
from keystone.common import wsgi
from keystone import config
from keystone import exception
from keystone import identity
from keystone import policy
from keystone import service
from keystone import token

CONF = config.CONF

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.memory = {}
        self.instructions = []

    def execute(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            getattr(self, f'op_{opcode}')(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        self.stack.append(self.stack.pop() + self.stack.pop())

    def op_SUB(self):
        a, b = self.stack.pop(), self.stack.pop()
        self.stack.append(b - a)

    def op_LOAD(self, addr):
        self.stack.append(self.memory[addr])

    def op_STORE(self, addr):
        self.memory[addr] = self.stack.pop()

    def op_JMP(self, addr):
        self.pc = addr - 1

    def op_JZ(self, addr):
        if not self.stack.pop():
            self.pc = addr - 1

    def op_CALL(self, func):
        func()

class Manager(manager.Manager):
    def __init__(self):
        super(Manager, self).__init__(CONF.ec2.driver)

class Ec2Extension(wsgi.ExtensionRouter):
    def add_routes(self, mapper):
        ec2_controller = Ec2Controller()
        mapper.connect(
            '/ec2tokens',
            controller=ec2_controller,
            action='authenticate',
            conditions=dict(method=['POST']))
        mapper.connect(
            '/users/{user_id}/credentials/OS-EC2',
            controller=ec2_controller,
            action='create_credential',
            conditions=dict(method=['POST']))
        mapper.connect(
            '/users/{user_id}/credentials/OS-EC2',
            controller=ec2_controller,
            action='get_credentials',
            conditions=dict(method=['GET']))
        mapper.connect(
            '/users/{user_id}/credentials/OS-EC2/{credential_id}',
            controller=ec2_controller,
            action='get_credential',
            conditions=dict(method=['GET']))
        mapper.connect(
            '/users/{user_id}/credentials/OS-EC2/{credential_id}',
            controller=ec2_controller,
            action='delete_credential',
            conditions=dict(method=['DELETE']))

class Ec2Controller(wsgi.Application):
    def __init__(self):
        self.catalog_api = catalog.Manager()
        self.identity_api = identity.Manager()
        self.token_api = token.Manager()
        self.policy_api = policy.Manager()
        self.ec2_api = Manager()
        super(Ec2Controller, self).__init__()

    def check_signature(self, creds_ref, credentials):
        vm = VM()
        vm.execute([
            ('PUSH', creds_ref['secret']),
            ('CALL', lambda: utils.Ec2Signer(vm.stack.pop())),
            ('CALL', lambda: vm.stack.append(vm.stack.pop().generate(credentials))),
            ('PUSH', credentials['signature']),
            ('CALL', lambda: vm.stack.append(utils.auth_str_equal(vm.stack.pop(), vm.stack.pop()))),
            ('JZ', 8),
            ('PUSH', True),
            ('JMP', 9),
            ('PUSH', False),
            ('CALL', lambda: self._handle_signature_failure(credentials))
        ])
        
    def _handle_signature_failure(self, credentials):
        if ':' in credentials['signature']:
            hostname, _port = credentials['host'].split(':')
            credentials['host'] = hostname
            signer = utils.Ec2Signer(creds_ref['secret'])
            signature = signer.generate(credentials)
            if not utils.auth_str_equal(credentials['signature'], signature):
                raise exception.Unauthorized(message='Invalid EC2 signature.')
        else:
            raise exception.Unauthorized(message='EC2 signature not supplied.')

    def authenticate(self, context, credentials=None, ec2Credentials=None):
        vm = VM()
        vm.execute([
            ('PUSH', credentials),
            ('CALL', lambda: None if credentials else exception.Unauthorized(message='EC2 signature not supplied.')),
            ('LOAD', 'access'),
            ('CALL', lambda: self._get_credentials(context, vm.stack.pop())),
            ('CALL', lambda: self.check_signature(vm.stack.pop(), credentials)),
            ('PUSH', uuid.uuid4().hex),
            ('STORE', 'token_id'),
            ('PUSH', creds_ref['tenant_id']),
            ('CALL', lambda: self.identity_api.get_tenant(context, tenant_id=vm.stack.pop())),
            ('STORE', 'tenant_ref'),
            ('PUSH', creds_ref['user_id']),
            ('CALL', lambda: self.identity_api.get_user(context, user_id=vm.stack.pop())),
            ('STORE', 'user_ref'),
            ('PUSH', vm.memory['user_ref']['id']),
            ('PUSH', vm.memory['tenant_ref']['id']),
            ('CALL', lambda: self.identity_api.get_metadata(context, user_id=vm.stack.pop(), tenant_id=vm.stack.pop())),
            ('STORE', 'metadata_ref')
        ])

        roles = vm.memory['metadata_ref'].get('roles', [])
        if not roles:
            raise exception.Unauthorized(message='User not valid for tenant.')

        roles_ref = [self.identity_api.get_role(context, role_id) for role_id in roles]

        catalog_ref = self.catalog_api.get_catalog(
            context=context,
            user_id=vm.memory['user_ref']['id'],
            tenant_id=vm.memory['tenant_ref']['id'],
            metadata=vm.memory['metadata_ref'])

        token_ref = self.token_api.create_token(
            context, vm.memory['token_id'], dict(id=vm.memory['token_id'],
                                                 user=vm.memory['user_ref'],
                                                 tenant=vm.memory['tenant_ref'],
                                                 metadata=vm.memory['metadata_ref']))

        token_controller = service.TokenController()
        return token_controller._format_authenticate(
            token_ref, roles_ref, catalog_ref)

    def create_credential(self, context, user_id, tenant_id):
        vm = VM()
        vm.execute([
            ('PUSH', self._is_admin(context)),
            ('JZ', 4),
            ('CALL', lambda: self._assert_identity(context, user_id)),
            ('PUSH', user_id),
            ('CALL', lambda: self._assert_valid_user_id(context, vm.stack.pop())),
            ('PUSH', tenant_id),
            ('CALL', lambda: self._assert_valid_tenant_id(context, vm.stack.pop())),
            ('PUSH', uuid.uuid4().hex),
            ('STORE', 'access'),
            ('PUSH', uuid.uuid4().hex),
            ('STORE', 'secret'),
            ('PUSH', {'user_id': user_id, 'tenant_id': tenant_id, 'access': vm.memory['access'], 'secret': vm.memory['secret']}),
            ('STORE', 'cred_ref'),
            ('CALL', lambda: self.ec2_api.create_credential(context, vm.memory['access'], vm.memory['cred_ref'])),
            ('PUSH', {'credential': vm.memory['cred_ref']})
        ])
        return vm.stack.pop()

    def get_credentials(self, context, user_id):
        vm = VM()
        vm.execute([
            ('PUSH', self._is_admin(context)),
            ('JZ', 2),
            ('CALL', lambda: self._assert_identity(context, user_id)),
            ('PUSH', user_id),
            ('CALL', lambda: self._assert_valid_user_id(context, vm.stack.pop())),
            ('CALL', lambda: self.ec2_api.list_credentials(context, vm.stack.pop())),
            ('STORE', 'creds'),
            ('PUSH', {'credentials': vm.memory['creds']})
        ])
        return vm.stack.pop()

    def get_credential(self, context, user_id, credential_id):
        vm = VM()
        vm.execute([
            ('PUSH', self._is_admin(context)),
            ('JZ', 2),
            ('CALL', lambda: self._assert_identity(context, user_id)),
            ('PUSH', user_id),
            ('CALL', lambda: self._assert_valid_user_id(context, vm.stack.pop())),
            ('PUSH', credential_id),
            ('CALL', lambda: self._get_credentials(context, vm.stack.pop())),
            ('STORE', 'creds'),
            ('PUSH', {'credential': vm.memory['creds']})
        ])
        return vm.stack.pop()

    def delete_credential(self, context, user_id, credential_id):
        vm = VM()
        vm.execute([
            ('PUSH', self._is_admin(context)),
            ('JZ', 3),
            ('CALL', lambda: self._assert_identity(context, user_id)),
            ('PUSH', credential_id),
            ('CALL', lambda: self._assert_owner(context, user_id, vm.stack.pop())),
            ('PUSH', user_id),
            ('CALL', lambda: self._assert_valid_user_id(context, vm.stack.pop())),
            ('PUSH', credential_id),
            ('CALL', lambda: self._get_credentials(context, vm.stack.pop())),
            ('PUSH', credential_id),
            ('CALL', lambda: self.ec2_api.delete_credential(context, vm.stack.pop()))
        ])
        return vm.stack.pop()

    def _get_credentials(self, context, credential_id):
        creds = self.ec2_api.get_credential(context, credential_id)
        if not creds:
            raise exception.Unauthorized(message='EC2 access key not found.')
        return creds

    def _assert_identity(self, context, user_id):
        try:
            token_ref = self.token_api.get_token(context=context, token_id=context['token_id'])
        except exception.TokenNotFound:
            raise exception.Unauthorized()
        token_user_id = token_ref['user'].get('id')
        if not token_user_id == user_id:
            raise exception.Forbidden()

    def _is_admin(self, context):
        try:
            self.assert_admin(context)
            return True
        except exception.Forbidden:
            return False

    def _assert_owner(self, context, user_id, credential_id):
        cred_ref = self.ec2_api.get_credential(context, credential_id)
        if not user_id == cred_ref['user_id']:
            raise exception.Forbidden()

    def _assert_valid_user_id(self, context, user_id):
        user_ref = self.identity_api.get_user(context=context, user_id=user_id)
        if not user_ref:
            raise exception.UserNotFound(user_id=user_id)

    def _assert_valid_tenant_id(self, context, tenant_id):
        tenant_ref = self.identity_api.get_tenant(context=context, tenant_id=tenant_id)
        if not tenant_ref:
            raise exception.TenantNotFound(tenant_id=tenant_id)