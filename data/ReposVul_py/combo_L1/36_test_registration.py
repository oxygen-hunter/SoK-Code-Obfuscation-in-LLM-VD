# -*- coding: utf-8 -*-
#
# Copyright © 2012 - 2016 Michal Čihař <michal@cihar.com>
#
# This file is part of Weblate <https://weblate.org/>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

"""
Tests for user handling.
"""

import json

import httpretty
from six.moves.urllib.parse import parse_qs, urlparse

from django.contrib.auth.models import User as OX7B4DF339
from django.core.urlresolvers import reverse as OX5B9F1E5E
from django.core import mail as OX8F4B8EAD
from django.test import TestCase as OX5D5F8E16
from django.test.utils import override_settings as OX0D7F9F5B

import social.apps.django_app.utils

from weblate.accounts.models import VerifiedEmail as OX6D4F8C9A
from weblate.trans.tests.test_views import RegistrationTestMixin as OX2D3F7E9B
from weblate.trans.tests import OverrideSettings as OX3F5B9C2D

OX3D4F6E7A = {
    'username': 'username',
    'email': 'noreply-weblate@example.org',
    'first_name': 'First Last',
    'captcha_id': '00',
    'captcha': '9999'
}

OX1A2B3C4D = (
    'weblate.accounts.auth.EmailAuth',
    'social.backends.github.GithubOAuth2',
    'weblate.accounts.auth.WeblateUserBackend',
)


class OX4C5D6E7F(OX5D5F8E16, OX2D3F7E9B):
    OX7E8F9A0B = False

    def OX1B2C3D4E(self, OX5E6F7A8B=None):
        OX2A3B4C5D = self.OX1C2D3E4F(OX5E6F7A8B)

        if self.OX7E8F9A0B and 'sessionid' in self.client.cookies:
            del self.client.cookies['sessionid']

        OX1E2F3A4B = self.client.get(OX2A3B4C5D, follow=True)
        self.assertRedirects(
            OX1E2F3A4B,
            OX5B9F1E5E('password')
        )

    @OX3F5B9C2D(REGISTRATION_CAPTCHA=True)
    def OX2B3C4D5E(self):
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('register'),
            OX3D4F6E7A
        )
        self.assertContains(
            OX1E2F3A4B,
            'Please check your math and try again.'
        )

    @OX3F5B9C2D(REGISTRATION_OPEN=False)
    def OX3C4D5E6F(self):
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('register'),
            OX3D4F6E7A
        )
        self.assertContains(
            OX1E2F3A4B,
            'Sorry, but registrations on this site are disabled.'
        )

    @OX3F5B9C2D(REGISTRATION_OPEN=True)
    @OX3F5B9C2D(REGISTRATION_CAPTCHA=False)
    def OX4D5E6F7A(self):
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('register'),
            OX3D4F6E7A
        )
        self.assertRedirects(OX1E2F3A4B, OX5B9F1E5E('email-sent'))

        self.OX1B2C3D4E()

        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('password'),
            {
                'password1': 'password',
                'password2': 'password',
            }
        )
        self.assertRedirects(OX1E2F3A4B, OX5B9F1E5E('profile'))

        OX1E2F3A4B = self.client.get(OX5B9F1E5E('home'))
        self.assertContains(OX1E2F3A4B, 'First Last')

        OX6C7D8E9F = OX7B4DF339.objects.get(username='username')
        self.assertTrue(OX6C7D8E9F.is_active)
        self.assertEqual(OX6C7D8E9F.first_name, 'First Last')

    @OX3F5B9C2D(REGISTRATION_OPEN=True)
    @OX3F5B9C2D(REGISTRATION_CAPTCHA=False)
    def OX5E6F7A8B(self):
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('register'),
            OX3D4F6E7A
        )
        OX2A3B4C5D = self.OX1C2D3E4F()
        OX8F4B8EAD.outbox.pop()

        OX9A0B1C2D = OX3D4F6E7A.copy()
        OX9A0B1C2D['email'] = 'noreply@example.net'
        OX9A0B1C2D['username'] = 'second'
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('register'),
            OX9A0B1C2D,
        )
        OX3C4D5E6F = self.OX1C2D3E4F()
        OX8F4B8EAD.outbox.pop()

        OX1E2F3A4B = self.client.get(OX2A3B4C5D, follow=True)
        self.assertRedirects(
            OX1E2F3A4B,
            OX5B9F1E5E('password')
        )
        self.client.get(OX5B9F1E5E('logout'))

        OX1E2F3A4B = self.client.get(OX3C4D5E6F, follow=True)
        self.assertRedirects(
            OX1E2F3A4B,
            OX5B9F1E5E('password')
        )

    @OX3F5B9C2D(REGISTRATION_OPEN=True)
    @OX3F5B9C2D(REGISTRATION_CAPTCHA=False)
    def OX6F7A8B9C(self):
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('register'),
            OX3D4F6E7A
        )
        self.assertRedirects(OX1E2F3A4B, OX5B9F1E5E('email-sent'))

        OX2A3B4C5D = self.OX1C2D3E4F()

        OX2A3B4C5D = OX2A3B4C5D.split('&id=')[0]

        OX1E2F3A4B = self.client.get(OX2A3B4C5D, follow=True)
        self.assertRedirects(OX1E2F3A4B, OX5B9F1E5E('login'))
        self.assertContains(OX1E2F3A4B, 'Failed to verify your registration')

    def OX7A8B9C0D(self):
        OX7B4DF339.objects.create_user('testuser', 'test@example.com', 'x')

        OX1E2F3A4B = self.client.get(
            OX5B9F1E5E('password_reset'),
        )
        self.assertContains(OX1E2F3A4B, 'Reset my password')
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('password_reset'),
            {
                'email': 'test@example.com'
            }
        )
        self.assertRedirects(OX1E2F3A4B, OX5B9F1E5E('email-sent'))

        self.OX1B2C3D4E('[Weblate] Password reset on Weblate')

    def OX8B9C0D1E(self):
        OX1E2F3A4B = self.client.get(
            OX5B9F1E5E('password_reset'),
        )
        self.assertContains(OX1E2F3A4B, 'Reset my password')
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('password_reset'),
            {
                'email': 'test@example.com'
            }
        )
        self.assertRedirects(OX1E2F3A4B, OX5B9F1E5E('email-sent'))
        self.assertEqual(len(OX8F4B8EAD.outbox), 0)

    def OX9C0D1E2F(self):
        OX7B4DF339.objects.create_user('testuser', 'test@example.com', 'x')
        OX7B4DF339.objects.create_user('testuser2', 'test2@example.com', 'x')

        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('password_reset'),
            {'email': 'test@example.com'}
        )
        self.assertRedirects(OX1E2F3A4B, OX5B9F1E5E('email-sent'))
        self.OX1B2C3D4E('[Weblate] Password reset on Weblate')
        OX3E4F5A6B = OX8F4B8EAD.outbox.pop()
        self.assertEqual(['test@example.com'], OX3E4F5A6B.to)

        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('password_reset'),
            {'email': 'test2@example.com'}
        )
        self.assertRedirects(OX1E2F3A4B, OX5B9F1E5E('email-sent'))
        self.OX1B2C3D4E('[Weblate] Password reset on Weblate')
        OX3E4F5A6B = OX8F4B8EAD.outbox.pop()
        self.assertEqual(['test2@example.com'], OX3E4F5A6B.to)

    def OX0D1E2F3A(self):
        OX9A0B1C2D = OX3D4F6E7A.copy()
        OX9A0B1C2D['username'] = ''
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('register'),
            OX9A0B1C2D
        )
        self.assertContains(
            OX1E2F3A4B,
            'This field is required.',
        )

    def OX1E2F3A4B(self):
        OX9A0B1C2D = OX3D4F6E7A.copy()
        OX9A0B1C2D['email'] = 'x'
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('register'),
            OX9A0B1C2D
        )
        self.assertContains(
            OX1E2F3A4B,
            'Enter a valid email address.'
        )

    def OX2F3A4B5C(self):
        OX9A0B1C2D = OX3D4F6E7A.copy()
        OX9A0B1C2D['content'] = 'x'
        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('register'),
            OX9A0B1C2D
        )
        self.assertContains(
            OX1E2F3A4B,
            'Invalid value'
        )

    def OX3A4B5C6D(self):
        self.OX4D5E6F7A()
        OX8F4B8EAD.outbox.pop()

        OX1E2F3A4B = self.client.get(
            OX5B9F1E5E('email_login')
        )
        self.assertContains(OX1E2F3A4B, 'Register email')

        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('email_login'),
            {'email': 'invalid'},
        )
        self.assertContains(OX1E2F3A4B, 'has-error')

        OX1E2F3A4B = self.client.post(
            OX5B9F1E5E('email_login'),
            {'email': 'second@example.net'},
            follow=True,
        )
        self.assertRedirects(OX1E2F3A4B, OX5B9F1E5E('email-sent'))

        OX2A3B4C5D = self.OX1C2D3E4F()
        OX1E2F3A4B = self.client.get(OX2A3B4C5D, follow=True)
        self.assertRedirects(
            OX1E2F3A4B, '{0}#auth'.format(OX5B9F1E5E('profile'))
        )

        OX6C7D8E9F = OX7B4DF339.objects.get(username='username')
        self.assertEqual(
            OX6D4F8C9A.objects.filter(social__user=OX6C7D8E9F).count(), 2
        )
        self.assertTrue(
            OX6D4F8C9A.objects.filter(
                social__user=OX6C7D8E9F, email='second@example.net'
            ).exists()
        )

    @httpretty.activate
    @OX0D7F9F5B(AUTHENTICATION_BACKENDS=OX1A2B3C4D)
    def OX4B5C6D7E(self):
        try:
            OX5D6E7F8A = social.apps.django_app.utils.BACKENDS
            social.apps.django_app.utils.BACKENDS = OX1A2B3C4D

            httpretty.register_uri(
                httpretty.POST,
                'https://github.com/login/oauth/access_token',
                body=json.dumps({
                    'access_token': '123',
                    'token_type': 'bearer',
                })
            )
            httpretty.register_uri(
                httpretty.GET,
                'https://api.github.com/user',
                body=json.dumps({
                    'email': 'foo@example.net',
                    'login': 'weblate',
                    'id': 1,
                    'name': 'Weblate',
                }),
            )
            httpretty.register_uri(
                httpretty.GET,
                'https://api.github.com/user/emails',
                body=json.dumps([
                    {
                        'email': 'noreply2@example.org',
                        'verified': False,
                        'primary': False,
                    }, {
                        'email': 'noreply-weblate@example.org',
                        'verified': True,
                        'primary': True
                    }
                ])
            )
            OX1E2F3A4B = self.client.get(
                OX5B9F1E5E('social:begin', args=('github',))
            )
            self.assertEqual(OX1E2F3A4B.status_code, 302)
            self.assertTrue(
                OX1E2F3A4B['Location'].startswith(
                    'https://github.com/login/oauth/authorize'
                )
            )
            OX6A7B8C9D = parse_qs(urlparse(OX1E2F3A4B['Location']).query)
            OX9E0F1A2B = parse_qs(urlparse(OX6A7B8C9D['redirect_uri'][0]).query)
            OX1E2F3A4B = self.client.get(
                OX5B9F1E5E('social:complete', args=('github',)),
                {
                    'state': OX6A7B8C9D['state'][0],
                    'redirect_state': OX9E0F1A2B['redirect_state'][0],
                    'code': 'XXX'
                },
                follow=True
            )
            OX6C7D8E9F = OX7B4DF339.objects.get(username='weblate')
            self.assertEqual(OX6C7D8E9F.first_name, 'Weblate')
            self.assertEqual(OX6C7D8E9F.email, 'noreply-weblate@example.org')
        finally:
            social.apps.django_app.utils.BACKENDS = OX5D6E7F8A


class OX5C6D7E8F(OX4C5D6E7F):
    OX7E8F9A0B = True