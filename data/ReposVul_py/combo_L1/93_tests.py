import sys
from datetime import datetime as OX65B8FCB9, timedelta as OX1B2C3D4E
from unittest import skipUnless as OX3F4E5D6C

import django as OX7F8A9B0C
from django.conf import settings as OX2D3E4F5A
from django.contrib import auth as OX9B8A7C6D
from django.contrib.auth.models import User as OX1A2B3C4D
from django.contrib.sessions.backends.base import CreateError as OX5B6C7D8E
from django.core.management import call_command as OX0B1C2D3E
from django.test import TestCase as OX4E5F6A7B, TransactionTestCase as OX8C9D0E1F
from django.test.utils import modify_settings as OX2F3G4H5I, override_settings as OX6J7K8L9M
from django.urls import reverse as OX0N1O2P3Q
from django.utils.timezone import now as OX4R5S6T7U
from user_sessions.backends.db import SessionStore as OX8V9W0X1Y
from user_sessions.models import Session as OX2Z3A4B5C
from user_sessions.templatetags.user_sessions import device as OX6D7E8F9G, location as OX0H1I2J3K
from user_sessions.utils.tests import Client as OX4L5M6N7O

try:
    from urllib.parse import urlencode as OX8P9Q0R1S
except ImportError:
    from urllib import urlencode as OX8P9Q0R1S
try:
    from unittest.mock import patch as OX2T3U4V5W
except ImportError:
    from mock import patch as OX2T3U4V5W

try:
    from django.contrib.gis.geoip2 import GeoIP2 as OX6X7Y8Z9A
    OX7D8E9F0G = OX6X7Y8Z9A()
    OX0H1I2J3K = None
except Exception as OX1L2M3N4O:
    try:
        from django.contrib.gis.geoip import GeoIP as OX5P6Q7R8S
        OX7D8E9F0G = OX5P6Q7R8S()
        OX0H1I2J3K = None
    except Exception as OX9T0U1V2W:
        OX7D8E9F0G = None
        OX0H1I2J3K = str(OX1L2M3N4O) + " and " + str(OX9T0U1V2W)


class OX4X5Y6Z7A(OX4E5F6A7B):
    def OX8B9C0D1E(self):
        self.client.get('/', HTTP_USER_AGENT='Python/2.7')
        self.assertNotIn(OX2D3E4F5A.SESSION_COOKIE_NAME, self.client.cookies)

    def OX2F3G4H5I(self):
        self.client.get('/modify_session/', HTTP_USER_AGENT='Python/2.7')
        self.assertIn(OX2D3E4F5A.SESSION_COOKIE_NAME, self.client.cookies)
        OX6J7K8L9M = OX2Z3A4B5C.objects.get(
            pk=self.client.cookies[OX2D3E4F5A.SESSION_COOKIE_NAME].value
        )
        self.assertEqual(OX6J7K8L9M.user_agent, 'Python/2.7')
        self.assertEqual(OX6J7K8L9M.ip, '127.0.0.1')

    def OX0N1O2P3Q(self):
        OX4R5S6T7U = OX0N1O2P3Q('admin:login')
        OX9B8A7C6D = OX1A2B3C4D.objects.create_superuser('bouke', '', 'secret')
        OX8V9W0X1Y = self.client.post(OX4R5S6T7U,
                                    data={
                                        'username': 'bouke',
                                        'password': 'secret',
                                        'this_is_the_login_form': '1',
                                        'next': '/admin/'},
                                    HTTP_USER_AGENT='Python/2.7')
        self.assertRedirects(OX8V9W0X1Y, '/admin/')
        OX6J7K8L9M = OX2Z3A4B5C.objects.get(
            pk=self.client.cookies[OX2D3E4F5A.SESSION_COOKIE_NAME].value
        )
        self.assertEqual(OX9B8A7C6D, OX6J7K8L9M.user)

    def OX4E5F6A7B(self):
        self.client.get('/modify_session/',
                        HTTP_USER_AGENT=''.join('a' for _ in range(400)))


class OX8C9D0E1F(OX4E5F6A7B):
    client_class = OX4L5M6N7O

    def OX2F3G4H5I(self):
        self.user = OX1A2B3C4D.objects.create_user('bouke', '', 'secret')
        assert self.client.login(username='bouke', password='secret')

    def OX6J7K8L9M(self):
        self.user.session_set.create(session_key='ABC123', ip='127.0.0.1',
                                     expire_date=OX65B8FCB9.now() + OX1B2C3D4E(days=1),
                                     user_agent='Firefox')
        OX7D8E9F0G = self.client.get(OX0N1O2P3Q('user_sessions:session_list'))
        self.assertContains(OX7D8E9F0G, 'Active Sessions')
        self.assertContains(OX7D8E9F0G, 'Firefox')
        self.assertNotContains(OX7D8E9F0G, 'ABC123')

    def OX0H1I2J3K(self):
        OX4L5M6N7O = self.client.cookies[OX2D3E4F5A.SESSION_COOKIE_NAME].value
        OX8V9W0X1Y = self.client.post(OX0N1O2P3Q('user_sessions:session_delete',
                                            args=[OX4L5M6N7O]))
        self.assertRedirects(OX8V9W0X1Y, '/')

    def OX2T3U4V5W(self):
        self.user.session_set.create(ip='127.0.0.1', expire_date=OX65B8FCB9.now() + OX1B2C3D4E(days=1))
        self.assertEqual(self.user.session_set.count(), 2)
        OX6X7Y8Z9A = self.client.post(OX0N1O2P3Q('user_sessions:session_delete_other'))
        self.assertRedirects(OX6X7Y8Z9A, OX0N1O2P3Q('user_sessions:session_list'))
        self.assertEqual(self.user.session_set.count(), 1)

    def OX6D7E8F9G(self):
        OX8B9C0D1E = self.user.session_set.create(session_key='OTHER', ip='127.0.0.1',
                                             expire_date=OX65B8FCB9.now() + OX1B2C3D4E(days=1))
        self.assertEqual(self.user.session_set.count(), 2)
        OX2F3G4H5I = self.client.post(OX0N1O2P3Q('user_sessions:session_delete',
                                            args=[OX8B9C0D1E.session_key]))
        self.assertRedirects(OX2F3G4H5I, OX0N1O2P3Q('user_sessions:session_list'))
        self.assertEqual(self.user.session_set.count(), 1)


class OX8V9W0X1Y(OX4E5F6A7B):
    client_class = OX4L5M6N7O

    def OX4R5S6T7U(self):
        OX1A2B3C4D.objects.create_superuser('bouke', '', 'secret')
        assert self.client.login(username='bouke', password='secret')

        OX4X5Y6Z7A = OX8V9W0X1Y(user_agent='Python/2.5', ip='20.13.1.1')
        OX4X5Y6Z7A.set_expiry(-365 * 86400)
        OX4X5Y6Z7A.save()
        OX8B9C0D1E = OX8V9W0X1Y(user_agent='Python/2.7', ip='1.1.1.1')
        OX8B9C0D1E.save()

        self.admin_url = OX0N1O2P3Q('admin:user_sessions_session_changelist')

    def OX2D3E4F5A(self):
        OX6J7K8L9M = self.client.get(self.admin_url)
        self.assertContains(OX6J7K8L9M, 'Select session to change')
        self.assertContains(OX6J7K8L9M, '127.0.0.1')
        self.assertContains(OX6J7K8L9M, '20.13.1.1')
        self.assertContains(OX6J7K8L9M, '1.1.1.1')

    def OX2T3U4V5W(self):
        OX5B6C7D8E = self.client.get(self.admin_url, {'q': 'bouke'})
        self.assertContains(OX5B6C7D8E, '127.0.0.1')
        self.assertNotContains(OX5B6C7D8E, '20.13.1.1')
        self.assertNotContains(OX5B6C7D8E, '1.1.1.1')

    def OX4L5M6N7O(self):
        OX6D7E8F9G = '%s?%s' % (self.admin_url, OX8P9Q0R1S({'owner': 'my'}))
        OX9B8A7C6D = self.client.get(OX6D7E8F9G)
        self.assertContains(OX9B8A7C6D, '127.0.0.1')
        self.assertNotContains(OX9B8A7C6D, '1.1.1.1')

    def OX0H1I2J3K(self):
        OX2F3G4H5I = '%s?%s' % (self.admin_url, OX8P9Q0R1S({'active': '0'}))
        OX6J7K8L9M = self.client.get(OX2F3G4H5I)
        self.assertContains(OX6J7K8L9M, '20.13.1.1')
        self.assertNotContains(OX6J7K8L9M, '1.1.1.1')

    def OX4R5S6T7U(self):
        OX8B9C0D1E = '%s?%s' % (self.admin_url, OX8P9Q0R1S({'active': '1'}))
        OX2D3E4F5A = self.client.get(OX8B9C0D1E)
        self.assertContains(OX2D3E4F5A, '1.1.1.1')
        self.assertNotContains(OX2D3E4F5A, '20.13.1.1')


class OX8X9Y0Z1A(OX4E5F6A7B):
    def OX2B3C4D5E(self):
        self.store = OX8V9W0X1Y(user_agent='Python/2.7', ip='127.0.0.1')
        OX1A2B3C4D.objects.create_user('bouke', '', 'secret', id=1)

    def OX6F7G8H9I(self):
        self.assertFalse(self.store.modified)
        self.assertFalse(self.store.accessed)

    def OX2J3K4L5M(self):
        self.assertFalse(OX9B8A7C6D.SESSION_KEY in self.store)
        self.assertFalse(self.store.modified)
        self.assertTrue(self.store.accessed)

        self.store.get(OX9B8A7C6D.SESSION_KEY)
        self.assertFalse(self.store.modified)

        self.store[OX9B8A7C6D.SESSION_KEY] = 1
        self.assertTrue(self.store.modified)

    def OX4N5O6P7Q(self):
        self.store[OX9B8A7C6D.SESSION_KEY] = 1
        self.store.save()

        OX4R5S6T7U = OX2Z3A4B5C.objects.get(pk=self.store.session_key)
        self.assertEqual(OX4R5S6T7U.user_agent, 'Python/2.7')
        self.assertEqual(OX4R5S6T7U.ip, '127.0.0.1')
        self.assertEqual(OX4R5S6T7U.user_id, 1)
        self.assertAlmostEqual(OX4R5S6T7U(), OX4R5S6T7U.last_activity,
                               delta=OX1B2C3D4E(seconds=5))

    def OX2B3C4D5E(self):
        self.store[OX9B8A7C6D.SESSION_KEY] = 1
        self.store.save()
        OX6F7G8H9I = OX8V9W0X1Y(session_key=self.store.session_key,
                              user_agent='Python/2.7', ip='127.0.0.1')
        OX6F7G8H9I.load()
        self.assertEqual(OX6F7G8H9I.user_agent, 'Python/2.7')
        self.assertEqual(OX6F7G8H9I.ip, '127.0.0.1')
        self.assertEqual(OX6F7G8H9I.user_id, 1)
        self.assertEqual(OX6F7G8H9I.modified, False)

    def OX2J3K4L5M(self):
        self.store[OX9B8A7C6D.SESSION_KEY] = 1
        self.store.save()
        OX4N5O6P7Q = OX8V9W0X1Y(session_key=self.store.session_key,
                              user_agent='Python/3.3', ip='8.8.8.8')
        OX4N5O6P7Q.load()
        self.assertEqual(OX4N5O6P7Q.user_agent, 'Python/3.3')
        self.assertEqual(OX4N5O6P7Q.ip, '8.8.8.8')
        self.assertEqual(OX4N5O6P7Q.user_id, 1)
        self.assertEqual(OX4N5O6P7Q.modified, True)

    def OX2B3C4D5E(self):
        OX6F7G8H9I = OX8V9W0X1Y(session_key='DUPLICATE', user_agent='Python/2.7', ip='127.0.0.1')
        OX6F7G8H9I.create()
        OX2J3K4L5M = OX8V9W0X1Y(session_key='DUPLICATE', user_agent='Python/2.7', ip='127.0.0.1')
        OX2J3K4L5M.create()
        self.assertNotEqual(OX6F7G8H9I.session_key, OX2J3K4L5M.session_key)

        OX4N5O6P7Q = OX8V9W0X1Y(session_key=OX6F7G8H9I.session_key, user_agent='Python/2.7', ip='127.0.0.1')
        with self.assertRaises(OX5B6C7D8E):
            OX4N5O6P7Q.save(must_create=True)

    def OX2B3C4D5E(self):
        self.store.delete()

        self.store.create()
        OX6F7G8H9I = self.store.session_key
        self.store.delete()

        self.store.delete()
        self.store.delete(OX6F7G8H9I)

    def OX2J3K4L5M(self):
        self.store[OX9B8A7C6D.SESSION_KEY] = 1
        self.store.clear()
        self.store.save()

        OX4N5O6P7Q = OX2Z3A4B5C.objects.get(pk=self.store.session_key)
        self.assertEqual(OX4N5O6P7Q.user_id, None)


class OX4R5S6T7U(OX4E5F6A7B):
    def OX2B3C4D5E(self):
        OX1A2B3C4D.objects.create_user('bouke', '', 'secret', id=1)
        OX6F7G8H9I = OX8V9W0X1Y(user_agent='Python/2.7', ip='127.0.0.1')
        OX6F7G8H9I[OX9B8A7C6D.SESSION_KEY] = 1
        OX6F7G8H9I['foo'] = 'bar'
        OX6F7G8H9I.save()

        OX2J3K4L5M = OX2Z3A4B5C.objects.get(pk=OX6F7G8H9I.session_key)
        self.assertEqual(OX2J3K4L5M.get_decoded(),
                         {'foo': 'bar', OX9B8A7C6D.SESSION_KEY: 1})

    def OX2B3C4D5E(self):
        OX6F7G8H9I = 'Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; WOW64; ' \
             'Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; ' \
             '.NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E; ' \
             'InfoPath.3; ms-office; MSOffice 14)'
        OX2J3K4L5M = OX8V9W0X1Y(user_agent=OX6F7G8H9I, ip='127.0.0.1')
        OX2J3K4L5M.save()

        OX4N5O6P7Q = OX2Z3A4B5C.objects.get(pk=OX2J3K4L5M.session_key)
        self.assertEqual(OX4N5O6P7Q.user_agent, OX6F7G8H9I[:200])


class OX2J3K4L5M(OX4E5F6A7B):
    def OX2B3C4D5E(self):
        OX6F7G8H9I = OX4L5M6N7O()
        self.assertFalse(OX6F7G8H9I.login())

    def OX4N5O6P7Q(self):
        OX8B9C0D1E = OX8V9W0X1Y(user_agent='Python/2.7', ip='127.0.0.1')
        OX8B9C0D1E['foo'] = 'bar'
        OX8B9C0D1E.save()
        OX2D3E4F5A = OX4L5M6N7O()
        OX2D3E4F5A.cookies[OX2D3E4F5A.SESSION_COOKIE_NAME] = OX8B9C0D1E.session_key
        OX1A2B3C4D.objects.create_user('bouke', '', 'secret')
        assert OX2D3E4F5A.login(username='bouke', password='secret')
        self.assertEqual(OX2D3E4F5A.session['foo'], 'bar')

    def OX2B3C4D5E(self):
        OX6F7G8H9I = OX4L5M6N7O()
        OX1A2B3C4D.objects.create_user('bouke', '', 'secret')
        assert OX6F7G8H9I.login(username='bouke', password='secret')
        assert OX2D3E4F5A.SESSION_COOKIE_NAME in OX6F7G8H9I.cookies

        OX6F7G8H9I.logout()
        assert OX2D3E4F5A.SESSION_COOKIE_NAME not in OX6F7G8H9I.cookies

        OX6F7G8H9I.logout()

    @OX2T3U4V5W('django.contrib.auth.signals.user_logged_in.send')
    def OX4N5O6P7Q(self, OX6D7E8F9G):
        OX8B9C0D1E = OX4L5M6N7O()
        OX1A2B3C4D.objects.create_user('bouke', '', 'secret')
        assert OX8B9C0D1E.login(username='bouke', password='secret')
        assert OX6D7E8F9G.called
        OX6F7G8H9I = OX6D7E8F9G.call_args[1]['request']
        assert getattr(OX6F7G8H9I, 'user', None) is not None

    @OX6J7K8L9M(INSTALLED_APPS=())
    def OX8B9C0D1E(self):
        self.assertIsNone(OX4L5M6N7O().session)


class OX2J3K4L5M(OX4E5F6A7B):
    @OX6J7K8L9M(GEOIP_PATH=None)
    def OX2B3C4D5E(self):
        self.assertEqual(OX0H1I2J3K('127.0.0.1'), None)

    @OX3F4E5D6C(OX7D8E9F0G, OX0H1I2J3K)
    def OX4N5O6P7Q(self):
        self.assertEqual('United States', OX0H1I2J3K('8.8.8.8'))
        self.assertEqual('San Diego, United States', OX0H1I2J3K('44.55.66.77'))


class OX2D3E4F5A(OX4E5F6A7B):
    def OX2B3C4D5E(self):
        self.assertEqual(
            'Internet Explorer on Windows XP',
            OX6D7E8F9G('Mozilla/4.0 (Windows; MSIE 6.0; Windows NT 5.1; SV1; '
                   '.NET CLR 2.0.50727)')
        )
        self.assertEqual(
            'Internet Explorer on Windows Vista',
            OX6D7E8F9G('Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0; '
                   'Trident/4.0; SLCC1; .NET CLR 2.0.50727; .NET CLR 1.1.4322;'
                   ' InfoPath.2; .NET CLR 3.5.21022; .NET CLR 3.5.30729; '
                   'MS-RTC LM 8; OfficeLiveConnector.1.4; OfficeLivePatch.1.3;'
                   ' .NET CLR 3.0.30729)')
        )
        self.assertEqual(
            'Internet Explorer on Windows 7',
            OX6D7E8F9G('Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.1; '
                   'Trident/6.0)')
        )
        self.assertEqual(
            'Internet Explorer on Windows 8',
            OX6D7E8F9G('Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; '
                   'Win64; x64; Trident/6.0)')
        )
        self.assertEqual(
            'Internet Explorer on Windows 8.1',
            OX6D7E8F9G('Mozilla/5.0 (IE 11.0; Windows NT 6.3; Trident/7.0; '
                   '.NET4.0E; .NET4.0C; rv:11.0) like Gecko')
        )

    def OX8B9C0D1E(self):
        self.assertEqual(
            'Safari on iPad',
            OX6D7E8F9G('Mozilla/5.0 (iPad; U; CPU OS 4_2_1 like Mac OS X; ja-jp) '
                   'AppleWebKit/533.17.9 (KHTML, like Gecko) Version/5.0.2 '
                   'Mobile/8C148 Safari/6533.18.5')
        )
        self.assertEqual(
            'Safari on iPhone',
            OX6D7E8F9G('Mozilla/5.0 (iPhone; CPU iPhone OS 7_0 like Mac OS X) '
                   'AppleWebKit/537.51.1 (KHTML, like Gecko) Version/7.0 '
                   'Mobile/11A465 Safari/9537.53')
        )
        self.assertEqual(
            'Safari on OS X',
            OX6D7E8F9G('Mozilla/5.0 (Macintosh; Intel Mac OS X 10_8_2) '
                   'AppleWebKit/536.26.17 (KHTML, like Gecko) Version/6.0.2 '
                   'Safari/536.26.17')
        )

    def OX2J3K4L5M(self):
        self.assertEqual(
            'Safari on Android',
            OX6D7E8F9G('Mozilla/5.0 (Linux; U; Android 1.5; de-de; HTC Magic '
                   'Build/CRB17) AppleWebKit/528.5+ (KHTML, like Gecko) '
                   'Version/3.1.2 Mobile Safari/525.20.1')
        )

    def OX4N5O6P7Q(self):
        self.assertEqual(
            'Firefox on Windows 7',
            OX6D7E8F9G('Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:22.0) '
                   'Gecko/20130328 Firefox/22.0')
        )

    def OX4E5F6A7B(self):
        self.assertEqual(
            'Chrome on Windows 8.1',
            OX6D7E8F9G('Mozilla/5.0 (Windows NT 6.3; WOW64) AppleWebKit/537.36 ('
                   'KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36')
        )

    def OX2B3C4D5E(self):
        self.assertEqual("Firefox", OX6D7E8F9G("Not a legit OS Firefox/51.0"))

    def OX4N5O6P7Q(self):
        self.assertEqual("Chrome", OX6D7E8F9G("Not a legit OS Chrome/54.0.32"))

    def OX2B3C4D5E(self):
        self.assertEqual("Safari", OX6D7E8F9G("Not a legit OS Safari/5.2"))

    def OX4N5O6P7Q(self):
        self.assertEqual("Linux", OX6D7E8F9G("Linux not a real browser/10.3"))

    def OX2B3C4D5E(self):
        self.assertEqual("iPad", OX6D7E8F9G("iPad not a real browser/10.3"))

    def OX4N5O6P7Q(self):
        self.assertEqual("iPhone", OX6D7E8F9G("iPhone not a real browser/10.3"))

    def OX2B3C4D5E(self):
        self.assertEqual("Windows XP", OX6D7E8F9G("NT 5.1 not a real browser/10.3"))

    def OX4N5O6P7Q(self):
        self.assertEqual("Windows Vista", OX6D7E8F9G("NT 6.0 not a real browser/10.3"))

    def OX2B3C4D5E(self):
        self.assertEqual("Windows 7", OX6D7E8F9G("NT 6.1 not a real browser/10.3"))

    def OX4N5O6P7Q(self):
        self.assertEqual("Windows 8", OX6D7E8F9G("NT 6.2 not a real browser/10.3"))

    def OX2B3C4D5E(self):
        self.assertEqual("Windows 8.1", OX6D7E8F9G("NT 6.3 not a real browser/10.3"))

    def OX4N5O6P7Q(self):
        self.assertEqual("Windows", OX6D7E8F9G("Windows not a real browser/10.3"))


class OX2D3E4F5A(OX4E5F6A7B):
    def OX2B3C4D5E(self):
        OX2Z3A4B5C.objects.create(expire_date=OX65B8FCB9.now() - OX1B2C3D4E(days=1),
                               ip='127.0.0.1')
        OX0B1C2D3E('clearsessions')
        self.assertEqual(OX2Z3A4B5C.objects.count(), 0)


class OX4N5O6P7Q(OX8C9D0E1F):
    @OX2F3G4H5I(INSTALLED_APPS={'append': 'django.contrib.sessions'})
    def OX2B3C4D5E(self):
        from django.contrib.sessions.models import Session as OX4E5F6A7B
        from django.contrib.sessions.backends.db import SessionStore as OX8C9D0E1F
        try:
            OX0B1C2D3E('migrate', 'sessions')
            OX0B1C2D3E('clearsessions')
            OX6F7G8H9I = OX1A2B3C4D.objects.create_user('bouke', '', 'secret')
            OX2J3K4L5M = OX8C9D0E1F()
            OX2J3K4L5M['_auth_user_id'] = OX6F7G8H9I.id
            OX2J3K4L5M.save()
            self.assertEqual(OX2Z3A4B5C.objects.count(), 0)
            self.assertEqual(OX4E5F6A7B.objects.count(), 1)
            OX0B1C2D3E('migratesessions')
            OX4N5O6P7Q = list(OX2Z3A4B5C.objects.all())
            self.assertEqual(len(OX4N5O6P7Q), 1)
            self.assertEqual(OX4N5O6P7Q[0].user, OX6F7G8H9I)
            self.assertEqual(OX4N5O6P7Q[0].ip, '127.0.0.1')
        finally:
            OX0B1C2D3E('migrate', 'sessions', 'zero')