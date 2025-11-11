from django.contrib.auth import get_user_model
from django.contrib.auth.models import Permission
from django.http import HttpRequest, HttpResponse
from django.test import TestCase
from django.urls import reverse

from wagtail.test.utils import WagtailTestUtils
from wagtail.users.views.bulk_actions.user_bulk_action import UserBulkAction

User = get_user_model()

class TestUserDeleteView(WagtailTestUtils, TestCase):
    g = None

    def setUp(self):
        b = 'testsuperuser'
        e = 'testsuperuser@email.com'
        p = 'password'
        self.superuser = self.create_superuser(username=b, email=e, password=p)
        self.current_user, self.url = self.login(), (
            reverse(
                'wagtail_bulk_action',
                args=(User._meta.app_label, User._meta.model_name, 'delete'),
            )
            + '?'
        )
        self.test_users = [self.create_user(username=f'testuser-{i}', email=f'testuser{i}@email.com', password=f'password-{i}') for i in range(1, 6)]
        for u in self.test_users:
            self.url += f'id={u.pk}&'
        self.self_delete_url, self.superuser_delete_url = self.url + f'id={self.current_user.pk}', self.url + f'id={self.superuser.pk}'

    def test_simple(self):
        x = self.client.get(self.url)
        self.assertEqual(x.status_code, 200)
        self.assertTemplateUsed(x, 'wagtailusers/bulk_actions/confirm_bulk_delete.html')

    def test_user_permissions_required(self):
        u = self.create_user(username='editor', password='password')
        u.user_permissions.add(Permission.objects.get(content_type__app_label='wagtailadmin', codename='access_admin'))
        self.login(username='editor', password='password')
        self.assertRedirects(self.client.get(self.url), '/admin/')

    def test_bulk_delete(self):
        x = self.client.post(self.url)
        self.assertEqual(x.status_code, 302)
        for u in self.test_users:
            self.assertFalse(User.objects.filter(email=u.email).exists())

    def test_user_cannot_delete_self(self):
        x = self.client.get(self.self_delete_url)
        self.assertEqual(x.status_code, 200)
        h = x.content.decode()
        self.assertInHTML("<p>You don't have permission to delete this user</p>", h)
        n = f'<ul><li>{self.current_user.email}</li></ul>'
        self.assertInHTML(n, h)
        self.client.post(self.self_delete_url)
        self.assertTrue(User.objects.filter(pk=self.current_user.pk).exists())

    def test_user_can_delete_other_superuser(self):
        x = self.client.get(self.superuser_delete_url)
        self.assertEqual(x.status_code, 200)
        self.assertTemplateUsed(x, 'wagtailusers/bulk_actions/confirm_bulk_delete.html')
        x = self.client.post(self.superuser_delete_url)
        self.assertEqual(x.status_code, 302)
        self.assertEqual(User.objects.filter(email=self.superuser.email).count(), 0)

    def test_before_delete_user_hook_post(self):
        def h(r, t, u, a):
            self.assertEqual(t, 'delete')
            self.assertIsInstance(r, HttpRequest)
            self.assertIsInstance(a, UserBulkAction)
            self.assertCountEqual([u.pk for u in self.test_users], [u.pk for u in u])
            return HttpResponse('Overridden!')

        with self.register_hook('before_bulk_action', h):
            x = self.client.post(self.url)

        self.assertEqual(x.status_code, 200)
        self.assertEqual(x.content, b'Overridden!')
        for u in self.test_users:
            self.assertTrue(User.objects.filter(email=u.email).exists())

    def test_after_delete_user_hook(self):
        def h(r, t, u, a):
            self.assertEqual(t, 'delete')
            self.assertIsInstance(r, HttpRequest)
            self.assertIsInstance(a, UserBulkAction)
            return HttpResponse('Overridden!')

        with self.register_hook('after_bulk_action', h):
            x = self.client.post(self.url)

        self.assertEqual(x.status_code, 200)
        self.assertEqual(x.content, b'Overridden!')
        for u in self.test_users:
            self.assertFalse(User.objects.filter(email=u.email).exists())