import sys
from django.contrib.auth import get_user_model
from django.contrib.auth.models import Permission
from django.http import HttpRequest, HttpResponse
from django.test import TestCase
from django.urls import reverse

from wagtail.test.utils import WagtailTestUtils
from wagtail.users.views.bulk_actions.user_bulk_action import UserBulkAction

User = get_user_model()

class StackVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.running = True
        self.instructions = []
        self.labels = {}

    def load_program(self, instructions):
        self.instructions = instructions
        self.pc = 0
        self.running = True
        self.preprocess_labels()

    def preprocess_labels(self):
        for index, instruction in enumerate(self.instructions):
            if instruction[0] == 'LABEL':
                self.labels[instruction[1]] = index

    def run(self):
        while self.running and self.pc < len(self.instructions):
            op, *args = self.instructions[self.pc]
            getattr(self, f'op_{op}')(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_STORE(self, variable):
        setattr(self, variable, self.op_POP())

    def op_LOAD(self, variable):
        self.op_PUSH(getattr(self, variable))

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a - b)

    def op_JMP(self, label):
        self.pc = self.labels[label] - 1

    def op_JZ(self, label):
        if self.op_POP() == 0:
            self.pc = self.labels[label] - 1

    def op_HALT(self):
        self.running = False

class TestUserDeleteView(WagtailTestUtils, TestCase):
    def setUp(self):
        self.vm = StackVM()
        program = [
            ('PUSH', 1), ('PUSH', 6), ('LABEL', 'CREATE_USERS'),
            ('STORE', 'i'), ('LOAD', 'i'), ('PUSH', 6), ('SUB',),
            ('JZ', 'CREATE_SUPERUSER'), ('LOAD', 'i'), ('PUSH', 'testuser-'),
            ('ADD',), ('STORE', 'username'), ('LOAD', 'i'), ('PUSH', 'testuser'),
            ('ADD',), ('STORE', 'email'), ('LOAD', 'i'), ('PUSH', 'password-'),
            ('ADD',), ('STORE', 'password'), ('LOAD', 'username'),
            ('LOAD', 'email'), ('LOAD', 'password'), ('PUSH', self.create_user),
            ('CALL', 3), ('DEC', 'i'), ('JMP', 'CREATE_USERS'),
            ('LABEL', 'CREATE_SUPERUSER'), ('PUSH', self.create_superuser),
            ('PUSH', 'testsuperuser'), ('PUSH', 'testsuperuser@email.com'),
            ('PUSH', 'password'), ('CALL', 3), ('STORE', 'superuser'),
            ('PUSH', self.login), ('CALL', 0), ('STORE', 'current_user'),
            ('PUSH', reverse), ('PUSH', 'wagtail_bulk_action'),
            ('PUSH', User._meta.app_label), ('PUSH', User._meta.model_name),
            ('PUSH', 'delete'), ('CALL', 4), ('PUSH', '?'), ('ADD',), ('STORE', 'url'),
            ('LABEL', 'APPEND_USERS'), ('LOAD', 'test_users'), ('ITER',),
            ('STORE', 'user'), ('LOAD', 'url'), ('LOAD', 'user'),
            ('PUSH', '&id='), ('ADD',), ('STORE', 'url'), ('JMP', 'APPEND_USERS'),
            ('LABEL', 'FINALIZE'), ('LOAD', 'url'), ('LOAD', 'current_user'),
            ('PUSH', '&id='), ('ADD',), ('STORE', 'self_delete_url'),
            ('LOAD', 'url'), ('LOAD', 'superuser'), ('PUSH', '&id='),
            ('ADD',), ('STORE', 'superuser_delete_url'), ('HALT',)
        ]
        self.vm.load_program(program)
        self.vm.run()
        self.test_users = self.vm.test_users
        self.superuser = self.vm.superuser
        self.current_user = self.vm.current_user
        self.url = self.vm.url
        self.self_delete_url = self.vm.self_delete_url
        self.superuser_delete_url = self.vm.superuser_delete_url

    def test_simple(self):
        self.vm.load_program([
            ('PUSH', self.client.get), ('LOAD', 'url'), ('CALL', 1), ('STORE', 'response'),
            ('PUSH', self.assertEqual), ('LOAD', 'response'), ('PUSH', 200), ('CALL', 2),
            ('PUSH', self.assertTemplateUsed), ('LOAD', 'response'),
            ('PUSH', "wagtailusers/bulk_actions/confirm_bulk_delete.html"), ('CALL', 2),
            ('HALT',)
        ])
        self.vm.run()

    def test_user_permissions_required(self):
        self.vm.load_program([
            ('PUSH', self.create_user), ('PUSH', 'editor'), ('PUSH', 'password'), ('CALL', 2), ('STORE', 'user'),
            ('PUSH', Permission.objects.get), ('PUSH', 'wagtailadmin'), ('PUSH', 'access_admin'), ('CALL', 2),
            ('STORE', 'admin_permission'), ('LOAD', 'user'), ('LOAD', 'admin_permission'),
            ('PUSH', 'add'), ('ATTR',), ('CALL', 1), ('PUSH', self.login),
            ('PUSH', 'editor'), ('PUSH', 'password'), ('CALL', 2),
            ('PUSH', self.client.get), ('LOAD', 'url'), ('CALL', 1), ('STORE', 'response'),
            ('PUSH', self.assertRedirects), ('LOAD', 'response'), ('PUSH', '/admin/'), ('CALL', 2),
            ('HALT',)
        ])
        self.vm.run()

    def test_bulk_delete(self):
        self.vm.load_program([
            ('PUSH', self.client.post), ('LOAD', 'url'), ('CALL', 1), ('STORE', 'response'),
            ('PUSH', self.assertEqual), ('LOAD', 'response'), ('PUSH', 302), ('CALL', 2),
            ('LABEL', 'CHECK_USERS'), ('LOAD', 'test_users'), ('ITER',), ('STORE', 'user'),
            ('PUSH', User.objects.filter), ('LOAD', 'user'), ('PUSH', 'email'), ('ATTR',), ('CALL', 1),
            ('PUSH', 'exists'), ('ATTR',), ('CALL', 0), ('PUSH', False), ('ASSERT', '=='),
            ('JMP', 'CHECK_USERS'), ('HALT',)
        ])
        self.vm.run()

    def test_user_cannot_delete_self(self):
        self.vm.load_program([
            ('PUSH', self.client.get), ('LOAD', 'self_delete_url'), ('CALL', 1), ('STORE', 'response'),
            ('PUSH', self.assertEqual), ('LOAD', 'response'), ('PUSH', 200), ('CALL', 2),
            ('PUSH', 'content'), ('ATTR',), ('LOAD', 'response'), ('CALL', 0), ('STORE', 'html'),
            ('PUSH', "You don't have permission to delete this user"), ('PUSH', self.assertInHTML),
            ('LOAD', 'html'), ('CALL', 2), ('PUSH', '<ul>'), ('PUSH', '<li>'),
            ('LOAD', 'current_user'), ('PUSH', 'email'), ('ATTR',), ('ADD',),
            ('PUSH', '</li></ul>'), ('ADD',), ('PUSH', self.assertInHTML), ('LOAD', 'html'), ('CALL', 2),
            ('PUSH', self.client.post), ('LOAD', 'self_delete_url'), ('CALL', 1),
            ('PUSH', User.objects.filter), ('LOAD', 'current_user'), ('PUSH', 'pk'), ('ATTR',), ('CALL', 1),
            ('PUSH', 'exists'), ('ATTR',), ('CALL', 0), ('PUSH', True), ('ASSERT', '=='),
            ('HALT',)
        ])
        self.vm.run()

    def test_user_can_delete_other_superuser(self):
        self.vm.load_program([
            ('PUSH', self.client.get), ('LOAD', 'superuser_delete_url'), ('CALL', 1), ('STORE', 'response'),
            ('PUSH', self.assertEqual), ('LOAD', 'response'), ('PUSH', 200), ('CALL', 2),
            ('PUSH', self.assertTemplateUsed), ('LOAD', 'response'),
            ('PUSH', "wagtailusers/bulk_actions/confirm_bulk_delete.html"), ('CALL', 2),
            ('PUSH', self.client.post), ('LOAD', 'superuser_delete_url'), ('CALL', 1),
            ('STORE', 'response'), ('PUSH', self.assertEqual), ('LOAD', 'response'),
            ('PUSH', 302), ('CALL', 2), ('PUSH', User.objects.filter),
            ('LOAD', 'superuser'), ('PUSH', 'email'), ('ATTR',), ('CALL', 1),
            ('PUSH', 'count'), ('ATTR',), ('CALL', 0), ('PUSH', 0), ('ASSERT', '=='),
            ('HALT',)
        ])
        self.vm.run()

    def test_before_delete_user_hook_post(self):
        def hook_func(request, action_type, users, action_class_instance):
            self.assertEqual(action_type, "delete")
            self.assertIsInstance(request, HttpRequest)
            self.assertIsInstance(action_class_instance, UserBulkAction)
            self.assertCountEqual(
                [user.pk for user in self.test_users], [user.pk for user in users]
            )
            return HttpResponse("Overridden!")

        self.vm.load_program([
            ('PUSH', self.register_hook), ('PUSH', 'before_bulk_action'), ('PUSH', hook_func),
            ('CALL', 2), ('STORE', 'hook_context'),
            ('PUSH', self.client.post), ('LOAD', 'url'), ('CALL', 1), ('STORE', 'response'),
            ('PUSH', self.assertEqual), ('LOAD', 'response'), ('PUSH', 200), ('CALL', 2),
            ('PUSH', 'content'), ('ATTR',), ('LOAD', 'response'), ('CALL', 0),
            ('PUSH', b'Overridden!'), ('ASSERT', '=='),
            ('LABEL', 'CHECK_USERS'), ('LOAD', 'test_users'), ('ITER',), ('STORE', 'user'),
            ('PUSH', User.objects.filter), ('LOAD', 'user'), ('PUSH', 'email'), ('ATTR',), ('CALL', 1),
            ('PUSH', 'exists'), ('ATTR',), ('CALL', 0), ('PUSH', True), ('ASSERT', '=='),
            ('JMP', 'CHECK_USERS'), ('HALT',)
        ])
        self.vm.run()

    def test_after_delete_user_hook(self):
        def hook_func(request, action_type, users, action_class_instance):
            self.assertEqual(action_type, "delete")
            self.assertIsInstance(request, HttpRequest)
            self.assertIsInstance(action_class_instance, UserBulkAction)
            return HttpResponse("Overridden!")

        self.vm.load_program([
            ('PUSH', self.register_hook), ('PUSH', 'after_bulk_action'), ('PUSH', hook_func),
            ('CALL', 2), ('STORE', 'hook_context'),
            ('PUSH', self.client.post), ('LOAD', 'url'), ('CALL', 1), ('STORE', 'response'),
            ('PUSH', self.assertEqual), ('LOAD', 'response'), ('PUSH', 200), ('CALL', 2),
            ('PUSH', 'content'), ('ATTR',), ('LOAD', 'response'), ('CALL', 0),
            ('PUSH', b'Overridden!'), ('ASSERT', '=='),
            ('LABEL', 'CHECK_USERS'), ('LOAD', 'test_users'), ('ITER',), ('STORE', 'user'),
            ('PUSH', User.objects.filter), ('LOAD', 'user'), ('PUSH', 'email'), ('ATTR',), ('CALL', 1),
            ('PUSH', 'exists'), ('ATTR',), ('CALL', 0), ('PUSH', False), ('ASSERT', '=='),
            ('JMP', 'CHECK_USERS'), ('HALT',)
        ])
        self.vm.run()