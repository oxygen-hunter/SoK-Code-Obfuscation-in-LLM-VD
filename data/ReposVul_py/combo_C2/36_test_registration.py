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

from django.contrib.auth.models import User
from django.core.urlresolvers import reverse
from django.core import mail
from django.test import TestCase
from django.test.utils import override_settings

import social.apps.django_app.utils

from weblate.accounts.models import VerifiedEmail
from weblate.trans.tests.test_views import RegistrationTestMixin
from weblate.trans.tests import OverrideSettings

REGISTRATION_DATA = {
    'username': 'username',
    'email': 'noreply-weblate@example.org',
    'first_name': 'First Last',
    'captcha_id': '00',
    'captcha': '9999'
}

GH_BACKENDS = (
    'weblate.accounts.auth.EmailAuth',
    'social.backends.github.GithubOAuth2',
    'weblate.accounts.auth.WeblateUserBackend',
)


class RegistrationTest(TestCase, RegistrationTestMixin):
    clear_cookie = False

    def assert_registration(self, match=None):
        url = None
        state = 0
        while True:
            if state == 0:
                url = self.assert_registration_mailbox(match)
                state = 1
            elif state == 1:
                if self.clear_cookie and 'sessionid' in self.client.cookies:
                    del self.client.cookies['sessionid']
                state = 2
            elif state == 2:
                response = self.client.get(url, follow=True)
                self.assertRedirects(
                    response,
                    reverse('password')
                )
                break

    @OverrideSettings(REGISTRATION_CAPTCHA=True)
    def test_register_captcha(self):
        state = 0
        while True:
            if state == 0:
                response = self.client.post(
                    reverse('register'),
                    REGISTRATION_DATA
                )
                state = 1
            elif state == 1:
                self.assertContains(
                    response,
                    'Please check your math and try again.'
                )
                break

    @OverrideSettings(REGISTRATION_OPEN=False)
    def test_register_closed(self):
        state = 0
        while True:
            if state == 0:
                response = self.client.post(
                    reverse('register'),
                    REGISTRATION_DATA
                )
                state = 1
            elif state == 1:
                self.assertContains(
                    response,
                    'Sorry, but registrations on this site are disabled.'
                )
                break

    @OverrideSettings(REGISTRATION_OPEN=True)
    @OverrideSettings(REGISTRATION_CAPTCHA=False)
    def test_register(self):
        state = 0
        while True:
            if state == 0:
                response = self.client.post(
                    reverse('register'),
                    REGISTRATION_DATA
                )
                state = 1
            elif state == 1:
                self.assertRedirects(response, reverse('email-sent'))
                state = 2
            elif state == 2:
                self.assert_registration()
                state = 3
            elif state == 3:
                response = self.client.post(
                    reverse('password'),
                    {
                        'password1': 'password',
                        'password2': 'password',
                    }
                )
                state = 4
            elif state == 4:
                self.assertRedirects(response, reverse('profile'))
                state = 5
            elif state == 5:
                response = self.client.get(reverse('home'))
                self.assertContains(response, 'First Last')
                state = 6
            elif state == 6:
                user = User.objects.get(username='username')
                self.assertTrue(user.is_active)
                self.assertEqual(user.first_name, 'First Last')
                break

    @OverrideSettings(REGISTRATION_OPEN=True)
    @OverrideSettings(REGISTRATION_CAPTCHA=False)
    def test_double_register(self):
        state = 0
        first_url = None
        second_url = None
        while True:
            if state == 0:
                response = self.client.post(
                    reverse('register'),
                    REGISTRATION_DATA
                )
                first_url = self.assert_registration_mailbox()
                mail.outbox.pop()
                state = 1
            elif state == 1:
                data = REGISTRATION_DATA.copy()
                data['email'] = 'noreply@example.net'
                data['username'] = 'second'
                response = self.client.post(
                    reverse('register'),
                    data,
                )
                second_url = self.assert_registration_mailbox()
                mail.outbox.pop()
                state = 2
            elif state == 2:
                response = self.client.get(first_url, follow=True)
                self.assertRedirects(
                    response,
                    reverse('password')
                )
                self.client.get(reverse('logout'))
                state = 3
            elif state == 3:
                response = self.client.get(second_url, follow=True)
                self.assertRedirects(
                    response,
                    reverse('password')
                )
                break

    @OverrideSettings(REGISTRATION_OPEN=True)
    @OverrideSettings(REGISTRATION_CAPTCHA=False)
    def test_register_missing(self):
        state = 0
        url = None
        while True:
            if state == 0:
                response = self.client.post(
                    reverse('register'),
                    REGISTRATION_DATA
                )
                self.assertRedirects(response, reverse('email-sent'))
                state = 1
            elif state == 1:
                url = self.assert_registration_mailbox()
                state = 2
            elif state == 2:
                url = url.split('&id=')[0]
                state = 3
            elif state == 3:
                response = self.client.get(url, follow=True)
                self.assertRedirects(response, reverse('login'))
                self.assertContains(response, 'Failed to verify your registration')
                break

    def test_reset(self):
        state = 0
        while True:
            if state == 0:
                User.objects.create_user('testuser', 'test@example.com', 'x')
                response = self.client.get(
                    reverse('password_reset'),
                )
                self.assertContains(response, 'Reset my password')
                state = 1
            elif state == 1:
                response = self.client.post(
                    reverse('password_reset'),
                    {
                        'email': 'test@example.com'
                    }
                )
                self.assertRedirects(response, reverse('email-sent'))
                state = 2
            elif state == 2:
                self.assert_registration('[Weblate] Password reset on Weblate')
                break

    def test_reset_nonexisting(self):
        state = 0
        while True:
            if state == 0:
                response = self.client.get(
                    reverse('password_reset'),
                )
                self.assertContains(response, 'Reset my password')
                state = 1
            elif state == 1:
                response = self.client.post(
                    reverse('password_reset'),
                    {
                        'email': 'test@example.com'
                    }
                )
                self.assertRedirects(response, reverse('email-sent'))
                state = 2
            elif state == 2:
                self.assertEqual(len(mail.outbox), 0)
                break

    def test_reset_twice(self):
        state = 0
        while True:
            if state == 0:
                User.objects.create_user('testuser', 'test@example.com', 'x')
                User.objects.create_user('testuser2', 'test2@example.com', 'x')
                response = self.client.post(
                    reverse('password_reset'),
                    {'email': 'test@example.com'}
                )
                self.assertRedirects(response, reverse('email-sent'))
                self.assert_registration('[Weblate] Password reset on Weblate')
                sent_mail = mail.outbox.pop()
                self.assertEqual(['test@example.com'], sent_mail.to)
                state = 1
            elif state == 1:
                response = self.client.post(
                    reverse('password_reset'),
                    {'email': 'test2@example.com'}
                )
                self.assertRedirects(response, reverse('email-sent'))
                self.assert_registration('[Weblate] Password reset on Weblate')
                sent_mail = mail.outbox.pop()
                self.assertEqual(['test2@example.com'], sent_mail.to)
                break

    def test_wrong_username(self):
        state = 0
        while True:
            if state == 0:
                data = REGISTRATION_DATA.copy()
                data['username'] = ''
                response = self.client.post(
                    reverse('register'),
                    data
                )
                state = 1
            elif state == 1:
                self.assertContains(
                    response,
                    'This field is required.',
                )
                break

    def test_wrong_mail(self):
        state = 0
        while True:
            if state == 0:
                data = REGISTRATION_DATA.copy()
                data['email'] = 'x'
                response = self.client.post(
                    reverse('register'),
                    data
                )
                state = 1
            elif state == 1:
                self.assertContains(
                    response,
                    'Enter a valid email address.'
                )
                break

    def test_spam(self):
        state = 0
        while True:
            if state == 0:
                data = REGISTRATION_DATA.copy()
                data['content'] = 'x'
                response = self.client.post(
                    reverse('register'),
                    data
                )
                state = 1
            elif state == 1:
                self.assertContains(
                    response,
                    'Invalid value'
                )
                break

    def test_add_mail(self):
        state = 0
        url = None
        while True:
            if state == 0:
                self.test_register()
                mail.outbox.pop()
                state = 1
            elif state == 1:
                response = self.client.get(
                    reverse('email_login')
                )
                self.assertContains(response, 'Register email')
                state = 2
            elif state == 2:
                response = self.client.post(
                    reverse('email_login'),
                    {'email': 'invalid'},
                )
                self.assertContains(response, 'has-error')
                state = 3
            elif state == 3:
                response = self.client.post(
                    reverse('email_login'),
                    {'email': 'second@example.net'},
                    follow=True,
                )
                self.assertRedirects(response, reverse('email-sent'))
                state = 4
            elif state == 4:
                url = self.assert_registration_mailbox()
                response = self.client.get(url, follow=True)
                self.assertRedirects(
                    response, '{0}#auth'.format(reverse('profile'))
                )
                state = 5
            elif state == 5:
                user = User.objects.get(username='username')
                self.assertEqual(
                    VerifiedEmail.objects.filter(social__user=user).count(), 2
                )
                self.assertTrue(
                    VerifiedEmail.objects.filter(
                        social__user=user, email='second@example.net'
                    ).exists()
                )
                break

    @httpretty.activate
    @override_settings(AUTHENTICATION_BACKENDS=GH_BACKENDS)
    def test_github(self):
        state = 0
        orig_backends = None
        try:
            while True:
                if state == 0:
                    orig_backends = social.apps.django_app.utils.BACKENDS
                    social.apps.django_app.utils.BACKENDS = GH_BACKENDS

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
                    response = self.client.get(
                        reverse('social:begin', args=('github',))
                    )
                    self.assertEqual(response.status_code, 302)
                    self.assertTrue(
                        response['Location'].startswith(
                            'https://github.com/login/oauth/authorize'
                        )
                    )
                    query = parse_qs(urlparse(response['Location']).query)
                    return_query = parse_qs(urlparse(query['redirect_uri'][0]).query)
                    state = 1
                elif state == 1:
                    response = self.client.get(
                        reverse('social:complete', args=('github',)),
                        {
                            'state': query['state'][0],
                            'redirect_state': return_query['redirect_state'][0],
                            'code': 'XXX'
                        },
                        follow=True
                    )
                    user = User.objects.get(username='weblate')
                    self.assertEqual(user.first_name, 'Weblate')
                    self.assertEqual(user.email, 'noreply-weblate@example.org')
                    break
        finally:
            social.apps.django_app.utils.BACKENDS = orig_backends


class NoCookieRegistrationTest(RegistrationTest):
    clear_cookie = True