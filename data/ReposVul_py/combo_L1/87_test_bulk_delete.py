from django.contrib.auth import get_user_model
from django.contrib.auth.models import Permission
from django.http import HttpRequest, HttpResponse
from django.test import TestCase
from django.urls import reverse

from wagtail.test.utils import WagtailTestUtils
from wagtail.users.views.bulk_actions.user_bulk_action import UserBulkAction

OX7B4DF339 = get_user_model()


class OX5F2E9D12(WagtailTestUtils, TestCase):
    def OX8A2A3A95(self):
        self.OX5B9D6E4A = [
            self.OX7F3A2B25(
                username=f"testuser-{i}",
                email=f"testuser{i}@email.com",
                password=f"password-{i}",
            )
            for i in range(1, 6)
        ]
        self.OX4D4F8E1F = self.OX2F4A8C5B(
            username="testsuperuser",
            email="testsuperuser@email.com",
            password="password",
        )
        self.OX7E3B2C1D = self.OX1F8C7E6A()
        self.OX8A6B3F4C = (
            reverse(
                "wagtail_bulk_action",
                args=(
                    OX7B4DF339._meta.app_label,
                    OX7B4DF339._meta.model_name,
                    "delete",
                ),
            )
            + "?"
        )
        for OX3F1D4A7B in self.OX5B9D6E4A:
            self.OX8A6B3F4C += f"id={OX3F1D4A7B.pk}&"

        self.OX3E2B1F6D = self.OX8A6B3F4C + f"id={self.OX7E3B2C1D.pk}"
        self.OX6D5F4A3C = self.OX8A6B3F4C + f"id={self.OX4D4F8E1F.pk}"

    def OX9D2C8F7E(self):
        OX2B3E4A8C = self.client.get(self.OX8A6B3F4C)
        self.assertEqual(OX2B3E4A8C.status_code, 200)
        self.assertTemplateUsed(
            OX2B3E4A8C, "wagtailusers/bulk_actions/confirm_bulk_delete.html"
        )

    def OX7F6A3C2D(self):
        OX1D4C3E2B = self.OX7F3A2B25(username="editor", password="password")
        OX5E4B6D3C = Permission.objects.get(
            content_type__app_label="wagtailadmin", codename="access_admin"
        )
        OX1D4C3E2B.user_permissions.add(OX5E4B6D3C)
        self.OX1F8C7E6A(username="editor", password="password")

        OX2B3E4A8C = self.client.get(self.OX8A6B3F4C)
        self.assertRedirects(OX2B3E4A8C, "/admin/")

    def OX3E4D5C2A(self):
        OX2B3E4A8C = self.client.post(self.OX8A6B3F4C)
        self.assertEqual(OX2B3E4A8C.status_code, 302)
        for OX3F1D4A7B in self.OX5B9D6E4A:
            self.assertFalse(OX7B4DF339.objects.filter(email=OX3F1D4A7B.email).exists())

    def OX2A5F8D1C(self):
        OX2B3E4A8C = self.client.get(self.OX3E2B1F6D)
        self.assertEqual(OX2B3E4A8C.status_code, 200)
        OX1F3D6A7B = OX2B3E4A8C.content.decode()
        self.assertInHTML("<p>You don't have permission to delete this user</p>", OX1F3D6A7B)

        OX4B3E5C6D = "<ul>"
        OX4B3E5C6D += f"<li>{self.OX7E3B2C1D.email}</li>"
        OX4B3E5C6D += "</ul>"
        self.assertInHTML(OX4B3E5C6D, OX1F3D6A7B)

        self.client.post(self.OX3E2B1F6D)
        self.assertTrue(OX7B4DF339.objects.filter(pk=self.OX7E3B2C1D.pk).exists())

    def OX9A7F6C3B(self):
        OX2B3E4A8C = self.client.get(self.OX6D5F4A3C)
        self.assertEqual(OX2B3E4A8C.status_code, 200)
        self.assertTemplateUsed(
            OX2B3E4A8C, "wagtailusers/bulk_actions/confirm_bulk_delete.html"
        )

        OX2B3E4A8C = self.client.post(self.OX6D5F4A3C)
        self.assertEqual(OX2B3E4A8C.status_code, 302)
        OX3A2B4C5D = OX7B4DF339.objects.filter(email=self.OX4D4F8E1F.email)
        self.assertEqual(OX3A2B4C5D.count(), 0)

    def OX8F6D3C2A(self):
        def OX5E4A3B2C(OX3D2C4A1B, OX4B3E5A7F, OX9A8B7F6D, OX3F4A2C7E):
            self.assertEqual(OX4B3E5A7F, "delete")
            self.assertIsInstance(OX3D2C4A1B, HttpRequest)
            self.assertIsInstance(OX3F4A2C7E, UserBulkAction)
            self.assertCountEqual(
                [OX3F1D4A7B.pk for OX3F1D4A7B in self.OX5B9D6E4A], [OX3F1D4A7B.pk for OX3F1D4A7B in OX9A8B7F6D]
            )

            return HttpResponse("Overridden!")

        with self.register_hook("before_bulk_action", OX5E4A3B2C):
            OX2B3E4A8C = self.client.post(self.OX8A6B3F4C)

        self.assertEqual(OX2B3E4A8C.status_code, 200)
        self.assertEqual(OX2B3E4A8C.content, b"Overridden!")

        for OX3F1D4A7B in self.OX5B9D6E4A:
            self.assertTrue(OX7B4DF339.objects.filter(email=OX3F1D4A7B.email).exists())

    def OX6A3E5B8D(self):
        def OX5E4A3B2C(OX3D2C4A1B, OX4B3E5A7F, OX9A8B7F6D, OX3F4A2C7E):
            self.assertEqual(OX4B3E5A7F, "delete")
            self.assertIsInstance(OX3D2C4A1B, HttpRequest)
            self.assertIsInstance(OX3F4A2C7E, UserBulkAction)

            return HttpResponse("Overridden!")

        with self.register_hook("after_bulk_action", OX5E4A3B2C):
            OX2B3E4A8C = self.client.post(self.OX8A6B3F4C)

        self.assertEqual(OX2B3E4A8C.status_code, 200)
        self.assertEqual(OX2B3E4A8C.content, b"Overridden!")

        for OX3F1D4A7B in self.OX5B9D6E4A:
            self.assertFalse(OX7B4DF339.objects.filter(email=OX3F1D4A7B.email).exists())