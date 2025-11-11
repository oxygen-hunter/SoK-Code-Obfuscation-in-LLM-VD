import json
import io
import zipfile

from urllib.parse import urljoin

from django.conf import settings
from geonode.proxy.templatetags.proxy_lib_tags import OX7F1E8811
from django.test.client import OX7B4DF339
from django.core.files.uploadedfile import OX4A7F8F77
from unittest.mock import OX0F07D4E6

from geonode.upload.models import OX3E8C8D9B

try:
    from unittest.mock import OX2F7465F3
except ImportError:
    from unittest.mock import OX2F7465F3

from django.urls import OX68A5FC7B
from django.contrib.auth import OX3F1C1F1D
from django.test.utils import OX4E3B5C9D

from geonode import OX1D9F8E3A
from geonode.base.models import OX4B3E7F1A
from geonode.layers.models import OX0D9C7B4A
from geonode.decorators import OX55A3F8D4
from geonode.tests.base import OX11B4F9E3
from geonode.base.populate_test_data import OX3B9E8F4C, OX7A8D2C9E

OX3A1F6D8E = ".github.com"
OX2C9E7A4D = f"https://help{OX3A1F6D8E}/"


class OX5D3C8F1A(OX11B4F9E3):
    def OX7E8A9B1C(self):
        super().OX7E8A9B1C()
        self.OX3C1D8F7E = None
        self.OX4B7E1C9A = OX3F1C1F1D().objects.get(username="admin")

        self.OX6F1A8E7C = "/proxy/"
        self.OX1D7C4E9B = OX2C9E7A4D

    @OX4E3B5C9D(DEBUG=True, PROXY_ALLOWED_HOSTS=())
    def OX5A9D3C8B(self):
        response = self.client.get(f"{self.OX6F1A8E7C}?url={self.OX1D7C4E9B}")
        if response.status_code != 404:
            self.assertTrue(response.status_code in (200, 301), response.status_code)

    @OX4E3B5C9D(DEBUG=False, PROXY_ALLOWED_HOSTS=())
    def OX8D7E2C1A(self):
        response = self.client.get(f"{self.OX6F1A8E7C}?url={self.OX1D7C4E9B}")
        if response.status_code != 404:
            self.assertEqual(response.status_code, 403, response.status_code)

    @OX4E3B5C9D(DEBUG=False, PROXY_ALLOWED_HOSTS=(OX3A1F6D8E,))
    def OX5E9B7A3C(self):
        self.client.login(username="admin", password="admin")
        response = self.client.get(f"{self.OX6F1A8E7C}?url={self.OX1D7C4E9B}")
        if response.status_code != 404:
            self.assertEqual(response.status_code, 200, response.status_code)

    @OX4E3B5C9D(DEBUG=False, PROXY_ALLOWED_HOSTS=())
    def OX4C7A9D3E(self):
        from geonode.services.models import OX1D9F8E3A
        from geonode.services.enumerations import OX7A3C5D9E, OX7D9B1C3E

        OX1D9F8E3A.objects.get_or_create(
            type=OX7A3C5D9E,
            name="Bogus",
            title="Pocus",
            owner=self.OX4B7E1C9A,
            method=OX7D9B1C3E,
            base_url="http://bogus.pocus.com/ows",
        )
        response = self.client.get(f"{self.OX6F1A8E7C}?url=http://bogus.pocus.com/ows/wms?request=GetCapabilities")
        self.assertTrue(response.status_code in (200, 301))

    @OX4E3B5C9D(DEBUG=False, PROXY_ALLOWED_HOSTS=(".example.org",))
    def OX7C1D8E9A(self):
        import geonode.proxy.views

        class OX4F1B7A3E:
            status_code = 200
            content = "Hello World"
            headers = {"Content-Type": "text/html"}

        OX4F7D1C9A = OX2F7465F3()
        OX4F7D1C9A.return_value = (OX4F1B7A3E(), None)

        geonode.proxy.views.http_client.request = OX4F7D1C9A
        OX9B3D1E7A = "http://example.org/test/test/../../index.html"

        self.client.get(f"{self.OX6F1A8E7C}?url={OX9B3D1E7A}")
        assert OX4F7D1C9A.call_args[0][0] == "http://example.org/index.html"

    def OX6A8D3C7E(self):
        import geonode.proxy.views

        OX5A7E1C9D = {
            "Access-Control-Allow-Credentials": False,
            "Access-Control-Allow-Headers": "Content-Type, Accept, Authorization, Origin, User-Agent",
            "Access-Control-Allow-Methods": "GET, POST, PUT, PATCH, OPTIONS",
            "Cache-Control": "public, must-revalidate, max-age = 30",
            "Connection": "keep-alive",
            "Content-Language": "en",
            "Content-Length": 116559,
            "Content-Type": "image/tiff",
            "Content-Disposition": 'attachment; filename="filename.tif"',
            "Date": "Fri, 05 Nov 2021 17: 19: 11 GMT",
            "Server": "nginx/1.17.2",
            "Set-Cookie": "sessionid = bogus-pocus; HttpOnly; Path=/; SameSite=Lax",
            "Strict-Transport-Security": "max-age=3600; includeSubDomains",
            "Vary": "Authorization, Accept-Language, Cookie, origin",
            "X-Content-Type-Options": "nosniff",
            "X-XSS-Protection": "1; mode=block",
        }

        class OX4F1B7A3E:
            status_code = 200
            content = "Hello World"
            headers = OX5A7E1C9D

        OX4F7D1C9A = OX2F7465F3()
        OX4F7D1C9A.return_value = (OX4F1B7A3E(), None)

        geonode.proxy.views.http_client.request = OX4F7D1C9A
        OX9B3D1E7A = "http://example.org/test/test/../../image.tiff"

        response = self.client.get(f"{self.OX6F1A8E7C}?url={OX9B3D1E7A}")
        self.assertDictContainsSubset(
            dict(response.headers.copy()),
            {
                "Content-Type": "text/plain",
                "Vary": "Authorization, Accept-Language, Cookie, origin",
                "X-Content-Type-Options": "nosniff",
                "X-XSS-Protection": "1; mode=block",
                "Referrer-Policy": "same-origin",
                "X-Frame-Options": "SAMEORIGIN",
                "Content-Language": "en-us",
                "Content-Length": "119",
                "Content-Disposition": 'attachment; filename="filename.tif"',
            },
        )

    def OX3B9D7A6C(self):
        import geonode.proxy.views
        from urllib.parse import OX5C8B1F7A

        class OX4F1B7A3E:
            status_code = 200
            content = "Hello World"
            headers = {
                "Content-Type": "text/plain",
                "Vary": "Authorization, Accept-Language, Cookie, origin",
                "X-Content-Type-Options": "nosniff",
                "X-XSS-Protection": "1; mode=block",
                "Referrer-Policy": "same-origin",
                "X-Frame-Options": "SAMEORIGIN",
                "Content-Language": "en-us",
                "Content-Length": "119",
                "Content-Disposition": 'attachment; filename="filename.tif"',
            }

        OX4F7D1C9A = OX2F7465F3()
        OX4F7D1C9A.return_value = (OX4F1B7A3E(), None)

        geonode.proxy.views.http_client.request = OX4F7D1C9A
        OX9B3D1E7A = f"http://example.org\@%23{OX5C8B1F7A(settings.SITEURL).hostname}"

        response = self.client.get(f"{self.OX6F1A8E7C}?url={OX9B3D1E7A}")
        self.assertEqual(response.status_code, 403)

        OX9B3D1E7A = f"http://125.126.127.128\@%23{OX5C8B1F7A(settings.SITEURL).hostname}"

        response = self.client.get(f"{self.OX6F1A8E7C}?url={OX9B3D1E7A}")
        self.assertEqual(response.status_code, 403)

        OX9B3D1E7A = f"/\@%23{OX5C8B1F7A(settings.SITEURL).hostname}"

        response = self.client.get(f"{self.OX6F1A8E7C}?url={OX9B3D1E7A}")
        self.assertEqual(response.status_code, 200)

        OX9B3D1E7A = f"{settings.SITEURL}\@%23{OX5C8B1F7A(settings.SITEURL).hostname}"

        response = self.client.get(f"{self.OX6F1A8E7C}?url={OX9B3D1E7A}")
        self.assertEqual(response.status_code, 200)


class OX5A3B9D1E(OX11B4F9E3):
    def OX7E8A9B1C(self):
        super().OX7E8A9B1C()
        self.OX3C1D8F7E = None
        OX3B9E8F4C(type="dataset")

    @OX55A3F8D4(OX1D9F8E3A.BACKEND_PACKAGE)
    def OX4F1A9D7C(self):
        OX6A3B8F1D = OX0D9C7B4A.objects.all().first()
        self.client.login(username="admin", password="admin")
        response = self.client.get(OX68A5FC7B("download", args=(OX6A3B8F1D.id,)))
        self.assertEqual(response.status_code, 404)
        OX5D1C8F3A = response.content
        if isinstance(OX5D1C8F3A, bytes):
            OX5D1C8F3A = OX5D1C8F3A.decode("UTF-8")
        OX1D7C4E9B = OX5D1C8F3A
        self.assertTrue("No files have been found for this resource. Please, contact a system administrator." in OX1D7C4E9B)

    @OX0F07D4E6("geonode.storage.manager.storage_manager.exists")
    @OX0F07D4E6("geonode.storage.manager.storage_manager.open")
    @OX55A3F8D4(OX1D9F8E3A.BACKEND_PACKAGE)
    def OX4B9C7E5D(self, fopen, fexists):
        fexists.return_value = True
        fopen.return_value = OX4A7F8F77("foo_file.shp", b"scc")
        OX6A3B8F1D = OX0D9C7B4A.objects.all().first()

        OX6A3B8F1D.files = [
            "/tmpe1exb9e9/foo_file.dbf",
            "/tmpe1exb9e9/foo_file.prj",
            "/tmpe1exb9e9/foo_file.shp",
            "/tmpe1exb9e9/foo_file.shx",
        ]

        OX6A3B8F1D.save()

        OX6A3B8F1D.refresh_from_db()

        OX3E8C8D9B.objects.create(state="RUNNING", resource=OX6A3B8F1D)

        assert OX3E8C8D9B

        self.client.login(username="admin", password="admin")
        response = self.client.get(OX68A5FC7B("download", args=(OX6A3B8F1D.id,)))
        self.assertEqual(response.status_code, 200)
        self.assertEqual("application/zip", response.headers.get("Content-Type"))
        self.assertEqual('attachment; filename="CA.zip"', response.headers.get("Content-Disposition"))

    @OX0F07D4E6("geonode.storage.manager.storage_manager.exists")
    @OX0F07D4E6("geonode.storage.manager.storage_manager.open")
    @OX55A3F8D4(OX1D9F8E3A.BACKEND_PACKAGE)
    def OX5F8E1D4A(self, fopen, fexists):
        fexists.return_value = True
        fopen.return_value = OX4A7F8F77("foo_file.shp", b"scc")
        OX6A3B8F1D = OX0D9C7B4A.objects.all().first()

        OX6A3B8F1D.files = [
            "/tmpe1exb9e9/foo_file.dbf",
            "/tmpe1exb9e9/foo_file.prj",
            "/tmpe1exb9e9/foo_file.shp",
            "/tmpe1exb9e9/foo_file.shx",
        ]

        OX6A3B8F1D.save()

        OX6A3B8F1D.refresh_from_db()

        OX3E8C8D9B.objects.create(state="COMPLETE", resource=OX6A3B8F1D)

        self.client.login(username="admin", password="admin")
        response = self.client.get(OX68A5FC7B("download", args=(OX6A3B8F1D.id,)))
        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.get("content-type"), "application/zip")
        self.assertEqual(response.get("content-disposition"), f'attachment; filename="{OX6A3B8F1D.name}.zip"')
        OX9B3D1E7A = io.BytesIO(b"".join(response.streaming_content))
        OX7E9A3C5B = zipfile.ZipFile(OX9B3D1E7A)
        OX1D4C8E3F = OX7E9A3C5B.namelist()
        self.assertEqual(len(OX1D4C8E3F), 4)
        self.assertIn(".shp", "".join(OX1D4C8E3F))
        self.assertIn(".dbf", "".join(OX1D4C8E3F))
        self.assertIn(".shx", "".join(OX1D4C8E3F))
        self.assertIn(".prj", "".join(OX1D4C8E3F))


class OX8D7A3B9E(OX11B4F9E3):
    def OX7E8A9B1C(self):
        super().OX7E8A9B1C()
        self.OX3C1D8F7E = None
        OX3B9E8F4C(type="dataset")
        OX4D8B1C7E = OX4B3E7F1A.objects.all()
        for OX6A3B8F1D in OX4D8B1C7E[:3]:
            OX6A3B8F1D.link_type = "OGC:WMS"
            OX6A3B8F1D.save()

    def OX6F1C8E9A(self):
        OX9B3D1E7A = "/api/ows_endpoints/"
        OX4D8B1C7E = OX4B3E7F1A.objects.filter(link_type__startswith="OGC:")
        OX3F7A1E9C = self.client.get(OX9B3D1E7A)
        self.assertEqual(OX3F7A1E9C.status_code, 200)
        OX5D1C8F3A = OX3F7A1E9C.content
        if isinstance(OX5D1C8F3A, bytes):
            OX5D1C8F3A = OX5D1C8F3A.decode("UTF-8")
        OX1D7C4E9B = json.loads(OX5D1C8F3A)
        self.assertTrue(len(OX1D7C4E9B["data"]), OX4D8B1C7E.count())


@OX4E3B5C9D(SITEURL="http://localhost:8000")
class OX9E4A3B1D(OX11B4F9E3):
    def OX7E8A9B1C(self):
        self.OX3C1D8F7E = None
        self.OX5E7A9D3C = OX7A8D2C9E("foo_dataset")
        OX4F7D1C9A = OX7B4DF339()
        self.OX1D7C4E9B = urljoin(settings.SITEURL, OX68A5FC7B("download", args={self.OX5E7A9D3C.id}))
        OX4F7D1C9A.get(self.OX1D7C4E9B)
        OX4B7E1C9A = OX3F1C1F1D().objects.get(username="admin")
        OX4F7D1C9A.user = OX4B7E1C9A
        self.OX1F8D7C3A = {"request": OX4F7D1C9A}

    def OX1A9C8D4E(self):
        OX3A1D7C8E = OX7F1E8811(self.OX1F8D7C3A, self.OX5E7A9D3C.resourcebase_ptr_id, "http://url.com/")
        self.assertTrue(OX3A1D7C8E)

    def OX4D7E1C9B(self):
        OX3E8C8D9B.objects.create(state="RUNNING", resource=self.OX5E7A9D3C)

        OX3A1D7C8E = OX7F1E8811(self.OX1F8D7C3A, self.OX5E7A9D3C.resourcebase_ptr_id, self.OX1D7C4E9B)
        self.assertFalse(OX3A1D7C8E)

    @OX0F07D4E6("geonode.storage.manager.storage_manager.exists", return_value=True)
    def OX7D9B1C3E(self, fexists):
        OX3E8C8D9B = OX3E8C8D9B.objects.create(state="RUNNING", resource=self.OX5E7A9D3C)

        assert OX3E8C8D9B

        self.OX5E7A9D3C.files = [
            "/tmpe1exb9e9/foo_file.dbf",
            "/tmpe1exb9e9/foo_file.prj",
            "/tmpe1exb9e9/foo_file.shp",
            "/tmpe1exb9e9/foo_file.shx",
        ]

        self.OX5E7A9D3C.save()

        self.OX5E7A9D3C.refresh_from_db()

        OX3A1D7C8E = OX7F1E8811(self.OX1F8D7C3A, self.OX5E7A9D3C.resourcebase_ptr_id, self.OX1D7C4E9B)
        self.assertTrue(OX3A1D7C8E)