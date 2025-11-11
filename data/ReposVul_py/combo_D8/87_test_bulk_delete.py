from django.contrib.auth import get_user_model
from django.contrib.auth.models import Permission
from django.http import HttpRequest, HttpResponse
from django.test import TestCase
from django.urls import reverse

from wagtail.test.utils import WagtailTestUtils
from wagtail.users.views.bulk_actions.user_bulk_action import UserBulkAction

User = get_user_model()

def get_users():
    return [
        create_user(
            username=f"testuser-{i}",
            email=f"testuser{i}@email.com",
            password=f"password-{i}",
        )
        for i in range(1, 6)
    ]

def get_superuser_instance():
    return create_superuser(
        username="testsuperuser",
        email="testsuperuser@email.com",
        password="password",
    )

def get_current_user(instance):
    return instance.login()

def get_url(instance, user_obj):
    url = (
        reverse(
            "wagtail_bulk_action",
            args=(
                User._meta.app_label,
                User._meta.model_name,
                "delete",
            ),
        )
        + "?"
    )
    for user in user_obj:
        url += f"id={user.pk}&"
    return url

def get_self_delete_url(url, current_user):
    return url + f"id={current_user.pk}"

def get_superuser_delete_url(url, superuser):
    return url + f"id={superuser.pk}"

def create_user(username, email, password):
    return User.objects.create_user(username=username, email=email, password=password)

def create_superuser(username, email, password):
    return User.objects.create_superuser(username=username, email=email, password=password)

class TestUserDeleteView(WagtailTestUtils, TestCase):
    def setUp(self):
        self.test_users = get_users()
        self.superuser = get_superuser_instance()
        self.current_user = get_current_user(self)
        self.url = get_url(self, self.test_users)
        self.self_delete_url = get_self_delete_url(self.url, self.current_user)
        self.superuser_delete_url = get_superuser_delete_url(self.url, self.superuser)

    def test_simple(self):
        response = self.client.get(self.url)
        self.assertEqual(response.status_code, 200)
        self.assertTemplateUsed(
            response, "wagtailusers/bulk_actions/confirm_bulk_delete.html"
        )

    def test_user_permissions_required(self):
        user = create_user(username="editor", email="editor@email.com", password="password")
        admin_permission = Permission.objects.get(
            content_type__app_label="wagtailadmin", codename="access_admin"
        )
        user.user_permissions.add(admin_permission)
        self.login(username="editor", password="password")

        response = self.client.get(self.url)
        self.assertRedirects(response, "/admin/")

    def test_bulk_delete(self):
        response = self.client.post(self.url)
        self.assertEqual(response.status_code, 302)
        for user in self.test_users:
            self.assertFalse(User.objects.filter(email=user.email).exists())

    def test_user_cannot_delete_self(self):
        response = self.client.get(self.self_delete_url)
        self.assertEqual(response.status_code, 200)
        html = response.content.decode()
        self.assertInHTML("<p>You don't have permission to delete this user</p>", html)

        needle = "<ul>"
        needle += f"<li>{self.current_user.email}</li>"
        needle += "</ul>"
        self.assertInHTML(needle, html)

        self.client.post(self.self_delete_url)
        self.assertTrue(User.objects.filter(pk=self.current_user.pk).exists())

    def test_user_can_delete_other_superuser(self):
        response = self.client.get(self.superuser_delete_url)
        self.assertEqual(response.status_code, 200)
        self.assertTemplateUsed(
            response, "wagtailusers/bulk_actions/confirm_bulk_delete.html"
        )

        response = self.client.post(self.superuser_delete_url)
        self.assertEqual(response.status_code, 302)
        users = User.objects.filter(email=self.superuser.email)
        self.assertEqual(users.count(), 0)

    def test_before_delete_user_hook_post(self):
        def hook_func(request, action_type, users, action_class_instance):
            self.assertEqual(action_type, "delete")
            self.assertIsInstance(request, HttpRequest)
            self.assertIsInstance(action_class_instance, UserBulkAction)
            self.assertCountEqual(
                [user.pk for user in self.test_users], [user.pk for user in users]
            )

            return HttpResponse("Overridden!")

        with self.register_hook("before_bulk_action", hook_func):
            response = self.client.post(self.url)

        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.content, b"Overridden!")

        for user in self.test_users:
            self.assertTrue(User.objects.filter(email=user.email).exists())

    def test_after_delete_user_hook(self):
        def hook_func(request, action_type, users, action_class_instance):
            self.assertEqual(action_type, "delete")
            self.assertIsInstance(request, HttpRequest)
            self.assertIsInstance(action_class_instance, UserBulkAction)

            return HttpResponse("Overridden!")

        with self.register_hook("after_bulk_action", hook_func):
            response = self.client.post(self.url)

        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.content, b"Overridden!")

        for user in self.test_users:
            self.assertFalse(User.objects.filter(email=user.email).exists())