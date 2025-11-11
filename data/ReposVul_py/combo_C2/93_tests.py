import sys
from datetime import datetime, timedelta
from unittest import skipUnless

import django
from django.conf import settings
from django.contrib import auth
from django.contrib.auth.models import User
from django.contrib.sessions.backends.base import CreateError
from django.core.management import call_command
from django.test import TestCase, TransactionTestCase
from django.test.utils import modify_settings, override_settings
from django.urls import reverse
from django.utils.timezone import now
from user_sessions.backends.db import SessionStore
from user_sessions.models import Session
from user_sessions.templatetags.user_sessions import device, location
from user_sessions.utils.tests import Client

try:
    from urllib.parse import urlencode
except ImportError:
    from urllib import urlencode
try:
    from unittest.mock import patch
except ImportError:
    from mock import patch

try:
    from django.contrib.gis.geoip2 import GeoIP2
    geoip = GeoIP2()
    geoip_msg = None
except Exception as error_geoip2:
    try:
        from django.contrib.gis.geoip import GeoIP
        geoip = GeoIP()
        geoip_msg = None
    except Exception as error_geoip:
        geoip = None
        geoip_msg = str(error_geoip2) + " and " + str(error_geoip)


class MiddlewareTest(TestCase):
    def test_unmodified_session(self):
        state = 0
        while state != -1:
            if state == 0:
                self.client.get('/', HTTP_USER_AGENT='Python/2.7')
                state = 1
            elif state == 1:
                self.assertNotIn(settings.SESSION_COOKIE_NAME, self.client.cookies)
                state = -1

    def test_modify_session(self):
        state = 0
        while state != -1:
            if state == 0:
                self.client.get('/modify_session/', HTTP_USER_AGENT='Python/2.7')
                state = 1
            elif state == 1:
                self.assertIn(settings.SESSION_COOKIE_NAME, self.client.cookies)
                state = 2
            elif state == 2:
                session = Session.objects.get(
                    pk=self.client.cookies[settings.SESSION_COOKIE_NAME].value
                )
                state = 3
            elif state == 3:
                self.assertEqual(session.user_agent, 'Python/2.7')
                state = 4
            elif state == 4:
                self.assertEqual(session.ip, '127.0.0.1')
                state = -1

    def test_login(self):
        state = 0
        while state != -1:
            if state == 0:
                admin_login_url = reverse('admin:login')
                state = 1
            elif state == 1:
                user = User.objects.create_superuser('bouke', '', 'secret')
                state = 2
            elif state == 2:
                response = self.client.post(admin_login_url,
                                            data={
                                                'username': 'bouke',
                                                'password': 'secret',
                                                'this_is_the_login_form': '1',
                                                'next': '/admin/'},
                                            HTTP_USER_AGENT='Python/2.7')
                state = 3
            elif state == 3:
                self.assertRedirects(response, '/admin/')
                session = Session.objects.get(
                    pk=self.client.cookies[settings.SESSION_COOKIE_NAME].value
                )
                state = 4
            elif state == 4:
                self.assertEqual(user, session.user)
                state = -1

    def test_long_ua(self):
        state = 0
        while state != -1:
            if state == 0:
                self.client.get('/modify_session/',
                                HTTP_USER_AGENT=''.join('a' for _ in range(400)))
                state = -1


class ViewsTest(TestCase):
    client_class = Client

    def setUp(self):
        self.user = User.objects.create_user('bouke', '', 'secret')
        assert self.client.login(username='bouke', password='secret')

    def test_list(self):
        state = 0
        while state != -1:
            if state == 0:
                self.user.session_set.create(session_key='ABC123', ip='127.0.0.1',
                                             expire_date=datetime.now() + timedelta(days=1),
                                             user_agent='Firefox')
                state = 1
            elif state == 1:
                response = self.client.get(reverse('user_sessions:session_list'))
                state = 2
            elif state == 2:
                self.assertContains(response, 'Active Sessions')
                state = 3
            elif state == 3:
                self.assertContains(response, 'Firefox')
                state = 4
            elif state == 4:
                self.assertNotContains(response, 'ABC123')
                state = -1

    def test_delete(self):
        state = 0
        while state != -1:
            if state == 0:
                session_key = self.client.cookies[settings.SESSION_COOKIE_NAME].value
                state = 1
            elif state == 1:
                response = self.client.post(reverse('user_sessions:session_delete',
                                                    args=[session_key]))
                state = 2
            elif state == 2:
                self.assertRedirects(response, '/')
                state = -1

    def test_delete_all_other(self):
        state = 0
        while state != -1:
            if state == 0:
                self.user.session_set.create(ip='127.0.0.1', expire_date=datetime.now() + timedelta(days=1))
                state = 1
            elif state == 1:
                self.assertEqual(self.user.session_set.count(), 2)
                state = 2
            elif state == 2:
                response = self.client.post(reverse('user_sessions:session_delete_other'))
                state = 3
            elif state == 3:
                self.assertRedirects(response, reverse('user_sessions:session_list'))
                state = 4
            elif state == 4:
                self.assertEqual(self.user.session_set.count(), 1)
                state = -1

    def test_delete_some_other(self):
        state = 0
        while state != -1:
            if state == 0:
                other = self.user.session_set.create(session_key='OTHER', ip='127.0.0.1',
                                                     expire_date=datetime.now() + timedelta(days=1))
                state = 1
            elif state == 1:
                self.assertEqual(self.user.session_set.count(), 2)
                state = 2
            elif state == 2:
                response = self.client.post(reverse('user_sessions:session_delete',
                                                    args=[other.session_key]))
                state = 3
            elif state == 3:
                self.assertRedirects(response, reverse('user_sessions:session_list'))
                state = 4
            elif state == 4:
                self.assertEqual(self.user.session_set.count(), 1)
                state = -1


class AdminTest(TestCase):
    client_class = Client

    def setUp(self):
        User.objects.create_superuser('bouke', '', 'secret')
        assert self.client.login(username='bouke', password='secret')

        expired = SessionStore(user_agent='Python/2.5', ip='20.13.1.1')
        expired.set_expiry(-365 * 86400)
        expired.save()
        unexpired = SessionStore(user_agent='Python/2.7', ip='1.1.1.1')
        unexpired.save()

        self.admin_url = reverse('admin:user_sessions_session_changelist')

    def test_list(self):
        state = 0
        while state != -1:
            if state == 0:
                response = self.client.get(self.admin_url)
                state = 1
            elif state == 1:
                self.assertContains(response, 'Select session to change')
                state = 2
            elif state == 2:
                self.assertContains(response, '127.0.0.1')
                state = 3
            elif state == 3:
                self.assertContains(response, '20.13.1.1')
                state = 4
            elif state == 4:
                self.assertContains(response, '1.1.1.1')
                state = -1

    def test_search(self):
        state = 0
        while state != -1:
            if state == 0:
                response = self.client.get(self.admin_url, {'q': 'bouke'})
                state = 1
            elif state == 1:
                self.assertContains(response, '127.0.0.1')
                state = 2
            elif state == 2:
                self.assertNotContains(response, '20.13.1.1')
                state = 3
            elif state == 3:
                self.assertNotContains(response, '1.1.1.1')
                state = -1

    def test_mine(self):
        state = 0
        while state != -1:
            if state == 0:
                my_sessions = '%s?%s' % (self.admin_url, urlencode({'owner': 'my'}))
                state = 1
            elif state == 1:
                response = self.client.get(my_sessions)
                state = 2
            elif state == 2:
                self.assertContains(response, '127.0.0.1')
                state = 3
            elif state == 3:
                self.assertNotContains(response, '1.1.1.1')
                state = -1

    def test_expired(self):
        state = 0
        while state != -1:
            if state == 0:
                expired = '%s?%s' % (self.admin_url, urlencode({'active': '0'}))
                state = 1
            elif state == 1:
                response = self.client.get(expired)
                state = 2
            elif state == 2:
                self.assertContains(response, '20.13.1.1')
                state = 3
            elif state == 3:
                self.assertNotContains(response, '1.1.1.1')
                state = -1

    def test_unexpired(self):
        state = 0
        while state != -1:
            if state == 0:
                unexpired = '%s?%s' % (self.admin_url, urlencode({'active': '1'}))
                state = 1
            elif state == 1:
                response = self.client.get(unexpired)
                state = 2
            elif state == 2:
                self.assertContains(response, '1.1.1.1')
                state = 3
            elif state == 3:
                self.assertNotContains(response, '20.13.1.1')
                state = -1


class SessionStoreTest(TestCase):
    def setUp(self):
        self.store = SessionStore(user_agent='Python/2.7', ip='127.0.0.1')
        User.objects.create_user('bouke', '', 'secret', id=1)

    def test_untouched_init(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertFalse(self.store.modified)
                state = 1
            elif state == 1:
                self.assertFalse(self.store.accessed)
                state = -1

    def test_auth_session_key(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertFalse(auth.SESSION_KEY in self.store)
                state = 1
            elif state == 1:
                self.assertFalse(self.store.modified)
                state = 2
            elif state == 2:
                self.assertTrue(self.store.accessed)
                state = 3
            elif state == 3:
                self.store.get(auth.SESSION_KEY)
                state = 4
            elif state == 4:
                self.assertFalse(self.store.modified)
                state = 5
            elif state == 5:
                self.store[auth.SESSION_KEY] = 1
                state = 6
            elif state == 6:
                self.assertTrue(self.store.modified)
                state = -1

    def test_save(self):
        state = 0
        while state != -1:
            if state == 0:
                self.store[auth.SESSION_KEY] = 1
                state = 1
            elif state == 1:
                self.store.save()
                state = 2
            elif state == 2:
                session = Session.objects.get(pk=self.store.session_key)
                state = 3
            elif state == 3:
                self.assertEqual(session.user_agent, 'Python/2.7')
                state = 4
            elif state == 4:
                self.assertEqual(session.ip, '127.0.0.1')
                state = 5
            elif state == 5:
                self.assertEqual(session.user_id, 1)
                state = 6
            elif state == 6:
                self.assertAlmostEqual(now(), session.last_activity,
                                       delta=timedelta(seconds=5))
                state = -1

    def test_load_unmodified(self):
        state = 0
        while state != -1:
            if state == 0:
                self.store[auth.SESSION_KEY] = 1
                state = 1
            elif state == 1:
                self.store.save()
                state = 2
            elif state == 2:
                store2 = SessionStore(session_key=self.store.session_key,
                                      user_agent='Python/2.7', ip='127.0.0.1')
                state = 3
            elif state == 3:
                store2.load()
                state = 4
            elif state == 4:
                self.assertEqual(store2.user_agent, 'Python/2.7')
                state = 5
            elif state == 5:
                self.assertEqual(store2.ip, '127.0.0.1')
                state = 6
            elif state == 6:
                self.assertEqual(store2.user_id, 1)
                state = 7
            elif state == 7:
                self.assertEqual(store2.modified, False)
                state = -1

    def test_load_modified(self):
        state = 0
        while state != -1:
            if state == 0:
                self.store[auth.SESSION_KEY] = 1
                state = 1
            elif state == 1:
                self.store.save()
                state = 2
            elif state == 2:
                store2 = SessionStore(session_key=self.store.session_key,
                                      user_agent='Python/3.3', ip='8.8.8.8')
                state = 3
            elif state == 3:
                store2.load()
                state = 4
            elif state == 4:
                self.assertEqual(store2.user_agent, 'Python/3.3')
                state = 5
            elif state == 5:
                self.assertEqual(store2.ip, '8.8.8.8')
                state = 6
            elif state == 6:
                self.assertEqual(store2.user_id, 1)
                state = 7
            elif state == 7:
                self.assertEqual(store2.modified, True)
                state = -1

    def test_duplicate_create(self):
        state = 0
        while state != -1:
            if state == 0:
                s1 = SessionStore(session_key='DUPLICATE', user_agent='Python/2.7', ip='127.0.0.1')
                state = 1
            elif state == 1:
                s1.create()
                state = 2
            elif state == 2:
                s2 = SessionStore(session_key='DUPLICATE', user_agent='Python/2.7', ip='127.0.0.1')
                state = 3
            elif state == 3:
                s2.create()
                state = 4
            elif state == 4:
                self.assertNotEqual(s1.session_key, s2.session_key)
                state = 5
            elif state == 5:
                s3 = SessionStore(session_key=s1.session_key, user_agent='Python/2.7', ip='127.0.0.1')
                state = 6
            elif state == 6:
                with self.assertRaises(CreateError):
                    s3.save(must_create=True)
                state = -1

    def test_delete(self):
        state = 0
        while state != -1:
            if state == 0:
                self.store.delete()
                state = 1
            elif state == 1:
                self.store.create()
                state = 2
            elif state == 2:
                session_key = self.store.session_key
                state = 3
            elif state == 3:
                self.store.delete()
                state = 4
            elif state == 4:
                self.store.delete()
                state = 5
            elif state == 5:
                self.store.delete(session_key)
                state = -1

    def test_clear(self):
        state = 0
        while state != -1:
            if state == 0:
                self.store[auth.SESSION_KEY] = 1
                state = 1
            elif state == 1:
                self.store.clear()
                state = 2
            elif state == 2:
                self.store.save()
                state = 3
            elif state == 3:
                session = Session.objects.get(pk=self.store.session_key)
                state = 4
            elif state == 4:
                self.assertEqual(session.user_id, None)
                state = -1


class ModelTest(TestCase):
    def test_get_decoded(self):
        state = 0
        while state != -1:
            if state == 0:
                User.objects.create_user('bouke', '', 'secret', id=1)
                state = 1
            elif state == 1:
                store = SessionStore(user_agent='Python/2.7', ip='127.0.0.1')
                state = 2
            elif state == 2:
                store[auth.SESSION_KEY] = 1
                state = 3
            elif state == 3:
                store['foo'] = 'bar'
                state = 4
            elif state == 4:
                store.save()
                state = 5
            elif state == 5:
                session = Session.objects.get(pk=store.session_key)
                state = 6
            elif state == 6:
                self.assertEqual(session.get_decoded(),
                                 {'foo': 'bar', auth.SESSION_KEY: 1})
                state = -1

    def test_very_long_ua(self):
        state = 0
        while state != -1:
            if state == 0:
                ua = 'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; WOW64; ' \
                     'Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; ' \
                     '.NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E; ' \
                     'InfoPath.3; ms-office; MSOffice 14)'
                state = 1
            elif state == 1:
                store = SessionStore(user_agent=ua, ip='127.0.0.1')
                state = 2
            elif state == 2:
                store.save()
                state = 3
            elif state == 3:
                session = Session.objects.get(pk=store.session_key)
                state = 4
            elif state == 4:
                self.assertEqual(session.user_agent, ua[:200])
                state = -1


class ClientTest(TestCase):
    def test_invalid_login(self):
        state = 0
        while state != -1:
            if state == 0:
                client = Client()
                state = 1
            elif state == 1:
                self.assertFalse(client.login())
                state = -1

    def test_restore_session(self):
        state = 0
        while state != -1:
            if state == 0:
                store = SessionStore(user_agent='Python/2.7', ip='127.0.0.1')
                state = 1
            elif state == 1:
                store['foo'] = 'bar'
                state = 2
            elif state == 2:
                store.save()
                state = 3
            elif state == 3:
                client = Client()
                state = 4
            elif state == 4:
                client.cookies[settings.SESSION_COOKIE_NAME] = store.session_key
                state = 5
            elif state == 5:
                User.objects.create_user('bouke', '', 'secret')
                state = 6
            elif state == 6:
                assert client.login(username='bouke', password='secret')
                state = 7
            elif state == 7:
                self.assertEqual(client.session['foo'], 'bar')
                state = -1

    def test_login_logout(self):
        state = 0
        while state != -1:
            if state == 0:
                client = Client()
                state = 1
            elif state == 1:
                User.objects.create_user('bouke', '', 'secret')
                state = 2
            elif state == 2:
                assert client.login(username='bouke', password='secret')
                state = 3
            elif state == 3:
                assert settings.SESSION_COOKIE_NAME in client.cookies
                state = 4
            elif state == 4:
                client.logout()
                state = 5
            elif state == 5:
                assert settings.SESSION_COOKIE_NAME not in client.cookies
                state = 6
            elif state == 6:
                client.logout()
                state = -1

    @patch('django.contrib.auth.signals.user_logged_in.send')
    def test_login_signal(self, mock_user_logged_in):
        state = 0
        while state != -1:
            if state == 0:
                client = Client()
                state = 1
            elif state == 1:
                User.objects.create_user('bouke', '', 'secret')
                state = 2
            elif state == 2:
                assert client.login(username='bouke', password='secret')
                state = 3
            elif state == 3:
                assert mock_user_logged_in.called
                state = 4
            elif state == 4:
                request = mock_user_logged_in.call_args[1]['request']
                state = 5
            elif state == 5:
                assert getattr(request, 'user', None) is not None
                state = -1

    @override_settings(INSTALLED_APPS=())
    def test_no_session(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertIsNone(Client().session)
                state = -1


class LocationTemplateFilterTest(TestCase):
    @override_settings(GEOIP_PATH=None)
    def test_no_location(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual(location('127.0.0.1'), None)
                state = -1

    @skipUnless(geoip, geoip_msg)
    def test_locations(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual('United States', location('8.8.8.8'))
                state = 1
            elif state == 1:
                self.assertEqual('San Diego, United States', location('44.55.66.77'))
                state = -1


class DeviceTemplateFilterTest(TestCase):
    def test_ie(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual(
                    'Internet Explorer on Windows XP',
                    device('Mozilla/4.0 (Windows; MSIE 6.0; Windows NT 5.1; SV1; '
                           '.NET CLR 2.0.50727)')
                )
                state = 1
            elif state == 1:
                self.assertEqual(
                    'Internet Explorer on Windows Vista',
                    device('Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; '
                           'Trident/4.0; SLCC1; .NET CLR 2.0.50727; .NET CLR 1.1.4322;'
                           ' InfoPath.2; .NET CLR 3.5.21022; .NET CLR 3.5.30729; '
                           'MS-RTC LM 8; OfficeLiveConnector.1.4; OfficeLivePatch.1.3;'
                           ' .NET CLR 3.0.30729)')
                )
                state = 2
            elif state == 2:
                self.assertEqual(
                    'Internet Explorer on Windows 7',
                    device('Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; '
                           'Trident/6.0)')
                )
                state = 3
            elif state == 3:
                self.assertEqual(
                    'Internet Explorer on Windows 8',
                    device('Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; '
                           'Win64; x64; Trident/6.0)')
                )
                state = 4
            elif state == 4:
                self.assertEqual(
                    'Internet Explorer on Windows 8.1',
                    device('Mozilla/5.0 (IE 11.0; Windows NT 6.3; Trident/7.0; '
                           '.NET4.0E; .NET4.0C; rv:11.0) like Gecko')
                )
                state = -1

    def test_apple(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual(
                    'Safari on iPad',
                    device('Mozilla/5.0 (iPad; U; CPU OS 4_2_1 like Mac OS X; ja-jp) '
                           'AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 '
                           'Mobile/8C148 Safari/6533.18.5')
                )
                state = 1
            elif state == 1:
                self.assertEqual(
                    'Safari on iPhone',
                    device('Mozilla/5.0 (iPhone; CPU iPhone OS 7_0 like Mac OS X) '
                           'AppleWebKit/537.51.1 (KHTML, like Gecko) Version/7.0 '
                           'Mobile/11A465 Safari/9537.53')
                )
                state = 2
            elif state == 2:
                self.assertEqual(
                    'Safari on OS X',
                    device('Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) '
                           'AppleWebKit/536.26.17 (KHTML, like Gecko) Version/6.0.2 '
                           'Safari/536.26.17')
                )
                state = -1

    def test_android(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual(
                    'Safari on Android',
                    device('Mozilla/5.0 (Linux; U; Android 1.5; de-de; HTC Magic '
                           'Build/CRB17) AppleWebKit/528.5+ (KHTML, like Gecko) '
                           'Version/3.1.2 Mobile Safari/525.20.1')
                )
                state = -1

    def test_firefox(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual(
                    'Firefox on Windows 7',
                    device('Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:22.0) '
                           'Gecko/20130328 Firefox/22.0')
                )
                state = -1

    def test_chrome(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual(
                    'Chrome on Windows 8.1',
                    device('Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 ('
                           'KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36')
                )
                state = -1

    def test_firefox_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Firefox", device("Not a legit OS Firefox/51.0"))
                state = -1

    def test_chrome_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Chrome", device("Not a legit OS Chrome/54.0.32"))
                state = -1

    def test_safari_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Safari", device("Not a legit OS Safari/5.2"))
                state = -1

    def test_linux_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Linux", device("Linux not a real browser/10.3"))
                state = -1

    def test_ipad_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("iPad", device("iPad not a real browser/10.3"))
                state = -1

    def test_iphone_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("iPhone", device("iPhone not a real browser/10.3"))
                state = -1

    def test_windowsxp_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Windows XP", device("NT 5.1 not a real browser/10.3"))
                state = -1

    def test_windowsvista_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Windows Vista", device("NT 6.0 not a real browser/10.3"))
                state = -1

    def test_windows7_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Windows 7", device("NT 6.1 not a real browser/10.3"))
                state = -1

    def test_windows8_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Windows 8", device("NT 6.2 not a real browser/10.3"))
                state = -1

    def test_windows81_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Windows 8.1", device("NT 6.3 not a real browser/10.3"))
                state = -1

    def test_windows_only(self):
        state = 0
        while state != -1:
            if state == 0:
                self.assertEqual("Windows", device("Windows not a real browser/10.3"))
                state = -1


class ClearsessionsCommandTest(TestCase):
    def test_can_call(self):
        state = 0
        while state != -1:
            if state == 0:
                Session.objects.create(expire_date=datetime.now() - timedelta(days=1),
                                       ip='127.0.0.1')
                state = 1
            elif state == 1:
                call_command('clearsessions')
                state = 2
            elif state == 2:
                self.assertEqual(Session.objects.count(), 0)
                state = -1


class MigratesessionsCommandTest(TransactionTestCase):
    @modify_settings(INSTALLED_APPS={'append': 'django.contrib.sessions'})
    def test_migrate_from_login(self):
        state = 0
        while state != -1:
            if state == 0:
                from django.contrib.sessions.models import Session as DjangoSession
                from django.contrib.sessions.backends.db import SessionStore as DjangoSessionStore
                state = 1
            elif state == 1:
                try:
                    call_command('migrate', 'sessions')
                    state = 2
                except:
                    state = -1
            elif state == 2:
                call_command('clearsessions')
                state = 3
            elif state == 3:
                user = User.objects.create_user('bouke', '', 'secret')
                state = 4
            elif state == 4:
                session = DjangoSessionStore()
                state = 5
            elif state == 5:
                session['_auth_user_id'] = user.id
                state = 6
            elif state == 6:
                session.save()
                state = 7
            elif state == 7:
                self.assertEqual(Session.objects.count(), 0)
                state = 8
            elif state == 8:
                self.assertEqual(DjangoSession.objects.count(), 1)
                state = 9
            elif state == 9:
                call_command('migratesessions')
                state = 10
            elif state == 10:
                new_sessions = list(Session.objects.all())
                state = 11
            elif state == 11:
                self.assertEqual(len(new_sessions), 1)
                state = 12
            elif state == 12:
                self.assertEqual(new_sessions[0].user, user)
                state = 13
            elif state == 13:
                self.assertEqual(new_sessions[0].ip, '127.0.0.1')
                state = -1
            elif state == 14:
                call_command('migrate', 'sessions', 'zero')
                state = -1