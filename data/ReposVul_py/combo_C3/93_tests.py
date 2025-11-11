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

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.vars = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr, *args = self.instructions[self.pc]
            getattr(self, instr)(*args)
            self.pc += 1

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        self.stack.pop()

    def ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def JMP(self, target):
        self.pc = target - 1

    def JZ(self, target):
        if self.stack.pop() == 0:
            self.pc = target - 1

    def LOAD(self, var_name):
        self.stack.append(self.vars[var_name])

    def STORE(self, var_name):
        self.vars[var_name] = self.stack.pop()

class MiddlewareTest(TestCase):
    def test_unmodified_session(self):
        instructions = [
            ('PUSH', '/'),
            ('PUSH', 'Python/2.7'),
            ('STORE', 'url'),
            ('STORE', 'agent'),
            ('LOAD', 'url'),
            ('LOAD', 'agent'),
            ('JMP', 1),
        ]
        vm = VirtualMachine()
        vm.load_program(instructions)
        vm.run()
        url = vm.vars['url']
        agent = vm.vars['agent']
        self.client.get(url, HTTP_USER_AGENT=agent)
        self.assertNotIn(settings.SESSION_COOKIE_NAME, self.client.cookies)

    def test_modify_session(self):
        instructions = [
            ('PUSH', '/modify_session/'),
            ('PUSH', 'Python/2.7'),
            ('STORE', 'url'),
            ('STORE', 'agent'),
            ('LOAD', 'url'),
            ('LOAD', 'agent'),
            ('JMP', 2),
        ]
        vm = VirtualMachine()
        vm.load_program(instructions)
        vm.run()
        url = vm.vars['url']
        agent = vm.vars['agent']
        self.client.get(url, HTTP_USER_AGENT=agent)
        self.assertIn(settings.SESSION_COOKIE_NAME, self.client.cookies)
        session_key = self.client.cookies[settings.SESSION_COOKIE_NAME].value
        session = Session.objects.get(pk=session_key)
        self.assertEqual(session.user_agent, 'Python/2.7')
        self.assertEqual(session.ip, '127.0.0.1')

    def test_login(self):
        instructions = [
            ('PUSH', 'admin:login'),
            ('STORE', 'login_url'),
            ('LOAD', 'login_url'),
            ('JMP', 3),
        ]
        vm = VirtualMachine()
        vm.load_program(instructions)
        vm.run()
        admin_login_url = reverse(vm.vars['login_url'])
        user = User.objects.create_superuser('bouke', '', 'secret')
        response = self.client.post(admin_login_url,
                                    data={
                                        'username': 'bouke',
                                        'password': 'secret',
                                        'this_is_the_login_form': '1',
                                        'next': '/admin/'},
                                    HTTP_USER_AGENT='Python/2.7')
        self.assertRedirects(response, '/admin/')
        session = Session.objects.get(
            pk=self.client.cookies[settings.SESSION_COOKIE_NAME].value
        )
        self.assertEqual(user, session.user)

    def test_long_ua(self):
        instructions = [
            ('PUSH', '/modify_session/'),
            ('STORE', 'url'),
            ('LOAD', 'url'),
            ('JMP', 4),
        ]
        vm = VirtualMachine()
        vm.load_program(instructions)
        vm.run()
        url = vm.vars['url']
        self.client.get(url, HTTP_USER_AGENT=''.join('a' for _ in range(400)))