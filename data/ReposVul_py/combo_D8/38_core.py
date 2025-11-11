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


def getCONF():
    return config.CONF

CONF = getCONF()


def getEc2Driver():
    return CONF.ec2.driver


class Manager(manager.Manager):
    def __init__(self):
        super(Manager, self).__init__(getEc2Driver())


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


def getCatalogApi():
    return catalog.Manager()

def getIdentityApi():
    return identity.Manager()

def getTokenApi():
    return token.Manager()

def getPolicyApi():
    return policy.Manager()

def getEc2Api():
    return Manager()


class Ec2Controller(wsgi.Application):
    def __init__(self):
        self.catalog_api = getCatalogApi()
        self.identity_api = getIdentityApi()
        self.token_api = getTokenApi()
        self.policy_api = getPolicyApi()
        self.ec2_api = getEc2Api()
        super(Ec2Controller, self).__init__()

    def check_signature(self, creds_ref, credentials):
        signer = utils.Ec2Signer(creds_ref['secret'])
        signature = signer.generate(credentials)
        if utils.auth_str_equal(credentials['signature'], signature):
            return
        elif ':' in credentials['signature']:
            hostname, _port = credentials['host'].split(':')
            credentials['host'] = hostname
            signature = signer.generate(credentials)
            if not utils.auth_str_equal(credentials.signature, signature):
                raise exception.Unauthorized(message='Invalid EC2 signature.')
        else:
            raise exception.Unauthorized(message='EC2 signature not supplied.')

    def authenticate(self, context, credentials=None, ec2Credentials=None):
        if not credentials and ec2Credentials:
            credentials = ec2Credentials

        if not 'access' in credentials:
            raise exception.Unauthorized(message='EC2 signature not supplied.')

        creds_ref = self._get_credentials(context,
                                          credentials['access'])
        self.check_signature(creds_ref, credentials)

        token_id = uuid.uuid4().hex
        tenant_ref = self.identity_api.get_tenant(
            context=context,
            tenant_id=creds_ref['tenant_id'])
        user_ref = self.identity_api.get_user(
            context=context,
            user_id=creds_ref['user_id'])
        metadata_ref = self.identity_api.get_metadata(
            context=context,
            user_id=user_ref['id'],
            tenant_id=tenant_ref['id'])

        roles = metadata_ref.get('roles', [])
        if not roles:
            raise exception.Unauthorized(message='User not valid for tenant.')
        roles_ref = [self.identity_api.get_role(context, role_id)
                     for role_id in roles]

        catalog_ref = self.catalog_api.get_catalog(
            context=context,
            user_id=user_ref['id'],
            tenant_id=tenant_ref['id'],
            metadata=metadata_ref)

        token_ref = self.token_api.create_token(
            context, token_id, dict(id=token_id,
                                    user=user_ref,
                                    tenant=tenant_ref,
                                    metadata=metadata_ref))

        token_controller = service.TokenController()
        return token_controller._format_authenticate(
            token_ref, roles_ref, catalog_ref)

    def create_credential(self, context, user_id, tenant_id):
        if not self._is_admin(context):
            self._assert_identity(context, user_id)

        self._assert_valid_user_id(context, user_id)
        self._assert_valid_tenant_id(context, tenant_id)

        cred_ref = {'user_id': user_id,
                    'tenant_id': tenant_id,
                    'access': uuid.uuid4().hex,
                    'secret': uuid.uuid4().hex}
        self.ec2_api.create_credential(context, cred_ref['access'], cred_ref)
        return {'credential': cred_ref}

    def get_credentials(self, context, user_id):
        if not self._is_admin(context):
            self._assert_identity(context, user_id)
        self._assert_valid_user_id(context, user_id)
        return {'credentials': self.ec2_api.list_credentials(context, user_id)}

    def get_credential(self, context, user_id, credential_id):
        if not self._is_admin(context):
            self._assert_identity(context, user_id)
        self._assert_valid_user_id(context, user_id)
        creds = self._get_credentials(context, credential_id)
        return {'credential': creds}

    def delete_credential(self, context, user_id, credential_id):
        if not self._is_admin(context):
            self._assert_identity(context, user_id)
            self._assert_owner(context, user_id, credential_id)

        self._assert_valid_user_id(context, user_id)
        self._get_credentials(context, credential_id)
        return self.ec2_api.delete_credential(context, credential_id)

    def _get_credentials(self, context, credential_id):
        creds = self.ec2_api.get_credential(context,
                                            credential_id)
        if not creds:
            raise exception.Unauthorized(message='EC2 access key not found.')
        return creds

    def _assert_identity(self, context, user_id):
        try:
            token_ref = self.token_api.get_token(
                context=context,
                token_id=context['token_id'])
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
        user_ref = self.identity_api.get_user(
            context=context,
            user_id=user_id)
        if not user_ref:
            raise exception.UserNotFound(user_id=user_id)

    def _assert_valid_tenant_id(self, context, tenant_id):
        tenant_ref = self.identity_api.get_tenant(
            context=context,
            tenant_id=tenant_id)
        if not tenant_ref:
            raise exception.TenantNotFound(tenant_id=tenant_id)