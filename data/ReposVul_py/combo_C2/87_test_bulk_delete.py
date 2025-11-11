from django.contrib.auth import get_user_model
from django.contrib.auth.models import Permission
from django.http import HttpRequest, HttpResponse
from django.test import TestCase
from django.urls import reverse

from wagtail.test.utils import WagtailTestUtils
from wagtail.users.views.bulk_actions.user_bulk_action import UserBulkAction

User = get_user_model()

class TestUserDeleteView(WagtailTestUtils, TestCase):
    def setUp(self):
        self.test_users = [
            self.create_user(
                username=f"testuser-{i}",
                email=f"testuser{i}@email.com",
                password=f"password-{i}",
            )
            for i in range(1, 6)
        ]
        self.superuser = self.create_superuser(
            username="testsuperuser",
            email="testsuperuser@email.com",
            password="password",
        )
        self.current_user = self.login()
        self.url = (
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
        for user in self.test_users:
            self.url += f"id={user.pk}&"

        self.self_delete_url = self.url + f"id={self.current_user.pk}"
        self.superuser_delete_url = self.url + f"id={self.superuser.pk}"

    def test_simple(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                response = self.client.get(self.url)
                dispatcher = 1
            elif dispatcher == 1:
                self.assertEqual(response.status_code, 200)
                dispatcher = 2
            elif dispatcher == 2:
                self.assertTemplateUsed(
                    response, "wagtailusers/bulk_actions/confirm_bulk_delete.html"
                )
                break

    def test_user_permissions_required(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                user = self.create_user(username="editor", password="password")
                dispatcher = 1
            elif dispatcher == 1:
                admin_permission = Permission.objects.get(
                    content_type__app_label="wagtailadmin", codename="access_admin"
                )
                dispatcher = 2
            elif dispatcher == 2:
                user.user_permissions.add(admin_permission)
                dispatcher = 3
            elif dispatcher == 3:
                self.login(username="editor", password="password")
                dispatcher = 4
            elif dispatcher == 4:
                response = self.client.get(self.url)
                dispatcher = 5
            elif dispatcher == 5:
                self.assertRedirects(response, "/admin/")
                break

    def test_bulk_delete(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                response = self.client.post(self.url)
                dispatcher = 1
            elif dispatcher == 1:
                self.assertEqual(response.status_code, 302)
                dispatcher = 2
            elif dispatcher == 2:
                for user in self.test_users:
                    self.assertFalse(User.objects.filter(email=user.email).exists())
                break

    def test_user_cannot_delete_self(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                response = self.client.get(self.self_delete_url)
                dispatcher = 1
            elif dispatcher == 1:
                self.assertEqual(response.status_code, 200)
                dispatcher = 2
            elif dispatcher == 2:
                html = response.content.decode()
                dispatcher = 3
            elif dispatcher == 3:
                self.assertInHTML("<p>You don't have permission to delete this user</p>", html)
                dispatcher = 4
            elif dispatcher == 4:
                needle = "<ul>"
                needle += f"<li>{self.current_user.email}</li>"
                needle += "</ul>"
                self.assertInHTML(needle, html)
                dispatcher = 5
            elif dispatcher == 5:
                self.client.post(self.self_delete_url)
                dispatcher = 6
            elif dispatcher == 6:
                self.assertTrue(User.objects.filter(pk=self.current_user.pk).exists())
                break

    def test_user_can_delete_other_superuser(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                response = self.client.get(self.superuser_delete_url)
                dispatcher = 1
            elif dispatcher == 1:
                self.assertEqual(response.status_code, 200)
                dispatcher = 2
            elif dispatcher == 2:
                self.assertTemplateUsed(
                    response, "wagtailusers/bulk_actions/confirm_bulk_delete.html"
                )
                dispatcher = 3
            elif dispatcher == 3:
                response = self.client.post(self.superuser_delete_url)
                dispatcher = 4
            elif dispatcher == 4:
                self.assertEqual(response.status_code, 302)
                dispatcher = 5
            elif dispatcher == 5:
                users = User.objects.filter(email=self.superuser.email)
                self.assertEqual(users.count(), 0)
                break

    def test_before_delete_user_hook_post(self):
        def hook_func(request, action_type, users, action_class_instance):
            dispatcher = 0
            while True:
                if dispatcher == 0:
                    self.assertEqual(action_type, "delete")
                    dispatcher = 1
                elif dispatcher == 1:
                    self.assertIsInstance(request, HttpRequest)
                    dispatcher = 2
                elif dispatcher == 2:
                    self.assertIsInstance(action_class_instance, UserBulkAction)
                    dispatcher = 3
                elif dispatcher == 3:
                    self.assertCountEqual(
                        [user.pk for user in self.test_users], [user.pk for user in users]
                    )
                    dispatcher = 4
                elif dispatcher == 4:
                    return HttpResponse("Overridden!")

        with self.register_hook("before_bulk_action", hook_func):
            response = self.client.post(self.url)

        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.assertEqual(response.status_code, 200)
                dispatcher = 1
            elif dispatcher == 1:
                self.assertEqual(response.content, b"Overridden!")
                dispatcher = 2
            elif dispatcher == 2:
                for user in self.test_users:
                    self.assertTrue(User.objects.filter(email=user.email).exists())
                break

    def test_after_delete_user_hook(self):
        def hook_func(request, action_type, users, action_class_instance):
            dispatcher = 0
            while True:
                if dispatcher == 0:
                    self.assertEqual(action_type, "delete")
                    dispatcher = 1
                elif dispatcher == 1:
                    self.assertIsInstance(request, HttpRequest)
                    dispatcher = 2
                elif dispatcher == 2:
                    self.assertIsInstance(action_class_instance, UserBulkAction)
                    dispatcher = 3
                elif dispatcher == 3:
                    return HttpResponse("Overridden!")

        with self.register_hook("after_bulk_action", hook_func):
            response = self.client.post(self.url)

        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.assertEqual(response.status_code, 200)
                dispatcher = 1
            elif dispatcher == 1:
                self.assertEqual(response.content, b"Overridden!")
                dispatcher = 2
            elif dispatcher == 2:
                for user in self.test_users:
                    self.assertFalse(User.objects.filter(email=user.email).exists())
                break