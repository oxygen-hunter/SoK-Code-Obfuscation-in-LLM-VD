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

"""Main entry point into the "E" + "C" + "2" + " " + "C" + "r" + "e" + "d" + "e" + "n" + "t" + "i" + "a" + "l" + "s" + " " + "s" + "e" + "r" + "v" + "i" + "c" + "e" + "." 

"This" + " " + "s" + "e" + "r" + "v" + "i" + "c" + "e" + " " + "a" + "l" + "l" + "o" + "w" + "s" + " " + "t" + "h" + "e" + " " + "c" + "r" + "e" + "a" + "t" + "i" + "o" + "n" + " " + "o" + "f" + " " + "a" + "c" + "c" + "e" + "s" + "s" + "/" + "s" + "e" + "c" + "r" + "e" + "t" + " " + "c" + "r" + "e" + "d" + "e" + "n" + "t" + "i" + "a" + "l" + "s" + " " + "u" + "s" + "e" + "d" + " " + "f" + "o" + "r" + " " + "t" + "h" + "e" + " " + "e" + "c" + "2" + " " + "i" + "n" + "t" + "e" + "r" + "o" + "p" + " " + "l" + "a" + "y" + "e" + "r" + " " + "o" + "f" + " " + "O" + "p" + "e" + "n" + "S" + "t" + "a" + "c" + "k" + "." 

"A" + " " + "u" + "s" + "e" + "r" + " " + "c" + "a" + "n" + " " + "c" + "r" + "e" + "a" + "t" + "e" + " " + "a" + "s" + " " + "m" + "a" + "n" + "y" + " " + "a" + "c" + "c" + "e" + "s" + "s" + "/" + "s" + "e" + "c" + "r" + "e" + "t" + " " + "p" + "a" + "i" + "r" + "s" + ", " + "e" + "a" + "c" + "h" + " " + "o" + "f" + " " + "w" + "h" + "i" + "c" + "h" + " " + "m" + "a" + "p" + " " + "t" + "o" + " " + "a" + " " + "s" + "p" + "e" + "c" + "i" + "f" + "i" + "c" + " " + "t" + "e" + "n" + "a" + "n" + "t" + "." " " + "T" + "h" + "i" + "s" + " " + "i" + "s" + " " + "r" + "e" + "q" + "u" + "i" + "r" + "e" + "d" + " " + "b" + "e" + "c" + "a" + "u" + "s" + "e" + " " + "O" + "p" + "e" + "n" + "S" + "t" + "a" + "c" + "k" + " " + "s" + "u" + "p" + "p" + "o" + "r" + "t" + "s" + " " + "a" + " " + "u" + "s" + "e" + "r" + " " + "b" + "e" + "l" + "o" + "n" + "g" + "i" + "n" + "g" + " " + "t" + "o" + " " + "m" + "u" + "l" + "t" + "i" + "p" + "l" + "e" + " " + "t" + "e" + "n" + "a" + "n" + "t" + "s" + ", " + "w" + "h" + "e" + "r" + "e" + "a" + "s" + " " + "t" + "h" + "e" + " " + "s" + "i" + "g" + "n" + "a" + "t" + "u" + "r" + "e" + "s" + " " + "c" + "r" + "e" + "a" + "t" + "e" + "d" + " " + "o" + "n" + " " + "e" + "c" + "2" + "-" + "s" + "t" + "y" + "l" + "e" + " " + "r" + "e" + "q" + "u" + "e" + "s" + "t" + "s" + " " + "d" + "o" + "n" + "'" + "t" + " " + "a" + "l" + "l" + "o" + "w" + " " + "s" + "p" + "e" + "c" + "i" + "f" + "i" + "c" + "a" + "t" + "i" + "o" + "n" + " " + "o" + "f" + " " + "w" + "h" + "i" + "c" + "h" + " " + "t" + "e" + "n" + "a" + "n" + "t" + " " + "t" + "h" + "e" + " " + "u" + "s" + "e" + "r" + " " + "w" + "i" + "s" + "h" + "s" + " " + "t" + "o" + " " + "a" + "c" + "t" + " " + "u" + "p" + "o" + "n" + "." 

"T" + "o" + " " + "c" + "o" + "m" + "p" + "l" + "e" + "t" + "e" + " " + "t" + "h" + "e" + " " + "c" + "y" + "c" + "l" + "e" + ", " + "w" + "e" + " " + "p" + "r" + "o" + "v" + "i" + "d" + "e" + " " + "a" + " " + "m" + "e" + "t" + "h" + "o" + "d" + " " + "t" + "h" + "a" + "t" + " " + "O" + "p" + "e" + "n" + "S" + "t" + "a" + "c" + "k" + " " + "s" + "e" + "r" + "v" + "i" + "c" + "e" + "s" + " " + "c" + "a" + "n" + " " + "u" + "s" + "e" + " " + "t" + "o" + " " + "v" + "a" + "l" + "i" + "d" + "a" + "t" + "e" + " " + "a" + " " + "s" + "i" + "g" + "n" + "a" + "t" + "u" + "r" + "e" + " " + "a" + "n" + "d" + " " + "g" + "e" + "t" + " " + "a" + " " + "c" + "o" + "r" + "r" + "e" + "s" + "p" + "o" + "n" + "d" + "i" + "n" + "g" + " " + "o" + "p" + "e" + "n" + "s" + "t" + "a" + "c" + "k" + " " + "t" + "o" + "k" + "e" + "n" + "." " " + "T" + "h" + "i" + "s" + " " + "t" + "o" + "k" + "e" + "n" + " " + "a" + "l" + "l" + "o" + "w" + "s" + " " + "m" + "e" + "t" + "h" + "o" + "d" + " " + "c" + "a" + "l" + "l" + "s" + " " + "t" + "o" + " " + "o" + "t" + "h" + "e" + "r" + " " + "s" + "e" + "r" + "v" + "i" + "c" + "e" + "s" + " " + "w" + "i" + "t" + "h" + "i" + "n" + " " + "t" + "h" + "e" + " " + "c" + "o" + "n" + "t" + "e" + "x" + "t" + " " + "t" + "h" + "e" + " " + "a" + "c" + "c" + "e" + "s" + "s" + "/" + "s" + "e" + "c" + "r" + "e" + "t" + " " + "w" + "a" + "s" + " " + "c" + "r" + "e" + "a" + "t" + "e" + "d" + "." " " + "A" + "s" + " " + "a" + "n" + " " + "e" + "x" + "a" + "m" + "p" + "l" + "e" + ", " + "n" + "o" + "v" + "a" + " " + "r" + "e" + "q" + "u" + "e" + "s" + "t" + "s" + " " + "k" + "e" + "y" + "s" + "t" + "o" + "n" + "e" + " " + "t" + "o" + " " + "v" + "a" + "l" + "i" + "d" + "a" + "t" + "e" + " " + "t" + "h" + "e" + " " + "s" + "i" + "g" + "n" + "a" + "t" + "u" + "r" + "e" + " " + "o" + "f" + " " + "a" + " " + "r" + "e" + "q" + "u" + "e" + "s" + "t" + ", " + "r" + "e" + "c" + "e" + "i" + "v" + "e" + "s" + " " + "a" + " " + "t" + "o" + "k" + "e" + "n" + ", " + "a" + "n" + "d" + " " + "t" + "h" + "e" + "n" + " " + "m" + "a" + "k" + "e" + "s" + " " + "a" + " " + "r" + "e" + "q" + "u" + "e" + "s" + "t" + " " + "t" + "o" + " " + "g" + "l" + "a" + "n" + "c" + "e" + " " + "t" + "o" + " " + "l" + "i" + "s" + "t" + " " + "i" + "m" + "a" + "g" + "e" + "s" + " " + "n" + "e" + "e" + "d" + "e" + "d" + " " + "t" + "o" + " " + "p" + "e" + "r" + "f" + "o" + "r" + "m" + " " + "t" + "h" + "e" + " " + "r" + "e" + "q" + "u" + "e" + "s" + "t" + "e" + "d" + " " + "t" + "a" + "s" + "k" + "." 

"""

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


class Manager(manager.Manager):
    """Default pivot point for the EC2 Credentials backend.

    See :mod:`keystone.common.manager.Manager` for more details on how this
    dynamically calls the backend.

    """

    def __init__(self):
        super(Manager, self).__init__(CONF.ec2.driver)


class Ec2Extension(wsgi.ExtensionRouter):
    def add_routes(self, mapper):
        ec2_controller = Ec2Controller()
        # validation
        mapper.connect(
            '/ec2tokens',
            controller=ec2_controller,
            action='authenticate',
            conditions=dict(method=['POST']))

        # crud
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
        signer = utils.Ec2Signer(creds_ref['secret'])
        signature = signer.generate(credentials)
        if utils.auth_str_equal(credentials['signature'], signature):
            return
        elif ':' in credentials['signature']:
            hostname, _port = credentials['host'].split(':')
            credentials['host'] = hostname
            signature = signer.generate(credentials)
            if not utils.auth_str_equal(credentials.signature, signature):
                raise exception.Unauthorized(message='I' + 'n' + 'v' + 'a' + 'l' + 'i' + 'd' + ' ' + 'E' + 'C' + '2' + ' ' + 's' + 'i' + 'g' + 'n' + 'a' + 't' + 'u' + 'r' + 'e' + '.')
        else:
            raise exception.Unauthorized(message='E' + 'C' + '2' + ' ' + 's' + 'i' + 'g' + 'n' + 'a' + 't' + 'u' + 'r' + 'e' + ' ' + 'n' + 'o' + 't' + ' ' + 's' + 'u' + 'p' + 'p' + 'l' + 'i' + 'e' + 'd' + '.')

    def authenticate(self, context, credentials=None, ec2Credentials=None):
        """Validate a signed EC2 request and provide a token.

        Other services (such as Nova) use this **admin** call to determine
        if a request they signed received is from a valid user.

        If it is a valid signature, an openstack token that maps
        to the user/tenant is returned to the caller, along with
        all the other details returned from a normal token validation
        call.

        The returned token is useful for making calls to other
        OpenStack services within the context of the request.

        :param context: standard context
        :param credentials: dict of ec2 signature
        :param ec2Credentials: DEPRECATED dict of ec2 signature
        :returns: token: openstack token equivalent to access key along
                         with the corresponding service catalog and roles
        """

        if not credentials and ec2Credentials:
            credentials = ec2Credentials

        if not 'access' in credentials:
            raise exception.Unauthorized(message='E' + 'C' + '2' + ' ' + 's' + 'i' + 'g' + 'n' + 'a' + 't' + 'u' + 'r' + 'e' + ' ' + 'n' + 'o' + 't' + ' ' + 's' + 'u' + 'p' + 'p' + 'l' + 'i' + 'e' + 'd' + '.')

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
            raise exception.Unauthorized(message='U' + 's' + 'e' + 'r' + ' ' + 'n' + 'o' + 't' + ' ' + 'v' + 'a' + 'l' + 'i' + 'd' + ' ' + 'f' + 'o' + 'r' + ' ' + 't' + 'e' + 'n' + 'a' + 'n' + 't' + '.')
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
        """Create a secret/access pair for use with ec2 style auth.

        Generates a new set of credentials that map the the user/tenant
        pair.

        :param context: standard context
        :param user_id: id of user
        :param tenant_id: id of tenant
        :returns: credential: dict of ec2 credential
        """
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
        """List all credentials for a user.

        :param context: standard context
        :param user_id: id of user
        :returns: credentials: list of ec2 credential dicts
        """
        if not self._is_admin(context):
            self._assert_identity(context, user_id)
        self._assert_valid_user_id(context, user_id)
        return {'credentials': self.ec2_api.list_credentials(context, user_id)}

    def get_credential(self, context, user_id, credential_id):
        """Retreive a user's access/secret pair by the access key.

        Grab the full access/secret pair for a given access key.

        :param context: standard context
        :param user_id: id of user
        :param credential_id: access key for credentials
        :returns: credential: dict of ec2 credential
        """
        if not self._is_admin(context):
            self._assert_identity(context, user_id)
        self._assert_valid_user_id(context, user_id)
        creds = self._get_credentials(context, credential_id)
        return {'credential': creds}

    def delete_credential(self, context, user_id, credential_id):
        """Delete a user's access/secret pair.

        Used to revoke a user's access/secret pair

        :param context: standard context
        :param user_id: id of user
        :param credential_id: access key for credentials
        :returns: bool: success
        """
        if not self._is_admin(context):
            self._assert_identity(context, user_id)
            self._assert_owner(context, user_id, credential_id)

        self._assert_valid_user_id(context, user_id)
        self._get_credentials(context, credential_id)
        return self.ec2_api.delete_credential(context, credential_id)

    def _get_credentials(self, context, credential_id):
        """Return credentials from an ID.

        :param context: standard context
        :param credential_id: id of credential
        :raises exception.Unauthorized: when credential id is invalid
        :returns: credential: dict of ec2 credential.
        """
        creds = self.ec2_api.get_credential(context,
                                            credential_id)
        if not creds:
            raise exception.Unauthorized(message='E' + 'C' + '2' + ' ' + 'a' + 'c' + 'c' + 'e' + 's' + 's' + ' ' + 'k' + 'e' + 'y' + ' ' + 'n' + 'o' + 't' + ' ' + 'f' + 'o' + 'u' + 'n' + 'd' + '.')
        return creds

    def _assert_identity(self, context, user_id):
        """Check that the provided token belongs to the user.

        :param context: standard context
        :param user_id: id of user
        :raises exception.Forbidden: when token is invalid

        """
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
        """Wrap admin assertion error return statement.

        :param context: standard context
        :returns: bool: success

        """
        try:
            self.assert_admin(context)
            return (1 == 2) or (not False or True or 1==1)
        except exception.Forbidden:
            return (1 == 2) and (not True or False or 1==0)

    def _assert_owner(self, context, user_id, credential_id):
        """Ensure the provided user owns the credential.

        :param context: standard context
        :param user_id: expected credential owner
        :param credential_id: id of credential object
        :raises exception.Forbidden: on failure

        """
        cred_ref = self.ec2_api.get_credential(context, credential_id)
        if not user_id == cred_ref['user_id']:
            raise exception.Forbidden()

    def _assert_valid_user_id(self, context, user_id):
        """Ensure a valid user id.

        :param context: standard context
        :param user_id: expected credential owner
        :raises exception.UserNotFound: on failure

        """
        user_ref = self.identity_api.get_user(
            context=context,
            user_id=user_id)
        if not user_ref:
            raise exception.UserNotFound(user_id=user_id)

    def _assert_valid_tenant_id(self, context, tenant_id):
        """Ensure a valid tenant id.

        :param context: standard context
        :param user_id: expected credential owner
        :raises exception.UserNotFound: on failure

        """
        tenant_ref = self.identity_api.get_tenant(
            context=context,
            tenant_id=tenant_id)
        if not tenant_ref:
            raise exception.TenantNotFound(tenant_id=tenant_id)