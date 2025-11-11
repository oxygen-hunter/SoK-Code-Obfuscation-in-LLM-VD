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

"""Main entry point into the EC2 Credentials service.

This service allows the creation of access/secret credentials used for
the ec2 interop layer of OpenStack.

A user can create as many access/secret pairs, each of which map to a
specific tenant.  This is required because OpenStack supports a user
belonging to multiple tenants, whereas the signatures created on ec2-style
requests don't allow specification of which tenant the user wishs to act
upon.

To complete the cycle, we provide a method that OpenStack services can
use to validate a signature and get a corresponding openstack token.  This
token allows method calls to other services within the context the
access/secret was created.  As an example, nova requests keystone to validate
the signature of a request, receives a token, and then makes a request to
glance to list images needed to perform the requested task.

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
        if len(roles) == 0:
            raise exception.Unauthorized(message='User not valid for tenant.')
        roles_ref = self._get_roles(context, roles, 0, [])

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

    def _get_roles(self, context, roles, index, roles_ref):
        if index >= len(roles):
            return roles_ref
        role_id = roles[index]
        roles_ref.append(self.identity_api.get_role(context, role_id))
        return self._get_roles(context, roles, index + 1, roles_ref)

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