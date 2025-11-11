import json
import io
import zipfile

from urllib.parse import urljoin

from django.conf import settings
from geonode.proxy.templatetags.proxy_lib_tags import original_link_available
from django.test.client import RequestFactory
from django.core.files.uploadedfile import SimpleUploadedFile
from unittest.mock import patch

from geonode.upload.models import Upload

try:
    from unittest.mock import MagicMock
except ImportError:
    from unittest.mock import MagicMock

from django.urls import reverse
from django.contrib.auth import get_user_model
from django.test.utils import override_settings

from geonode import geoserver
from geonode.base.models import Link
from geonode.layers.models import Dataset
from geonode.decorators import on_ogc_backend
from geonode.tests.base import GeoNodeBaseTestSupport
from geonode.base.populate_test_data import create_models, create_single_dataset

TEST_DOMAIN = ".github.com"
TEST_URL = f"https://help{TEST_DOMAIN}/"


class ProxyTest(GeoNodeBaseTestSupport):
    def setUp(self):
        super().setUp()
        self.maxDiff = None
        self.admin = get_user_model().objects.get(username="admin")

        self.proxy_url = "/proxy/"
        self.url = TEST_URL

    @override_settings(DEBUG=True, PROXY_ALLOWED_HOSTS=())
    def test_validate_host_disabled_in_debug(self):
        state = 0
        while True:
            if state == 0:
                response = self.client.get(f"{self.proxy_url}?url={self.url}")
                if response.status_code != 404:
                    state = 1
                else:
                    break
            elif state == 1:
                self.assertTrue(response.status_code in (200, 301), response.status_code)
                break

    @override_settings(DEBUG=False, PROXY_ALLOWED_HOSTS=())
    def test_validate_host_disabled_not_in_debug(self):
        state = 0
        while True:
            if state == 0:
                response = self.client.get(f"{self.proxy_url}?url={self.url}")
                if response.status_code != 404:
                    state = 1
                else:
                    break
            elif state == 1:
                self.assertEqual(response.status_code, 403, response.status_code)
                break

    @override_settings(DEBUG=False, PROXY_ALLOWED_HOSTS=(TEST_DOMAIN,))
    def test_proxy_allowed_host(self):
        state = 0
        while True:
            if state == 0:
                self.client.login(username="admin", password="admin")
                state = 1
            elif state == 1:
                response = self.client.get(f"{self.proxy_url}?url={self.url}")
                if response.status_code != 404:
                    state = 2
                else:
                    break
            elif state == 2:
                self.assertEqual(response.status_code, 200, response.status_code)
                break

    @override_settings(DEBUG=False, PROXY_ALLOWED_HOSTS=())
    def test_validate_remote_services_hosts(self):
        state = 0
        while True:
            if state == 0:
                from geonode.services.models import Service
                from geonode.services.enumerations import WMS, INDEXED

                Service.objects.get_or_create(
                    type=WMS,
                    name="Bogus",
                    title="Pocus",
                    owner=self.admin,
                    method=INDEXED,
                    base_url="http://bogus.pocus.com/ows",
                )
                state = 1
            elif state == 1:
                response = self.client.get(f"{self.proxy_url}?url=http://bogus.pocus.com/ows/wms?request=GetCapabilities")
                state = 2
            elif state == 2:
                self.assertTrue(response.status_code in (200, 301))
                break

    @override_settings(DEBUG=False, PROXY_ALLOWED_HOSTS=(".example.org",))
    def test_relative_urls(self):
        state = 0
        while True:
            if state == 0:
                import geonode.proxy.views

                class Response:
                    status_code = 200
                    content = "Hello World"
                    headers = {"Content-Type": "text/html"}

                request_mock = MagicMock()
                request_mock.return_value = (Response(), None)

                geonode.proxy.views.http_client.request = request_mock
                url = "http://example.org/test/test/../../index.html"
                state = 1
            elif state == 1:
                self.client.get(f"{self.proxy_url}?url={url}")
                assert request_mock.call_args[0][0] == "http://example.org/index.html"
                break

    def test_proxy_preserve_headers(self):
        state = 0
        while True:
            if state == 0:
                import geonode.proxy.views

                _test_headers = {
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

                class Response:
                    status_code = 200
                    content = "Hello World"
                    headers = _test_headers

                request_mock = MagicMock()
                request_mock.return_value = (Response(), None)

                geonode.proxy.views.http_client.request = request_mock
                url = "http://example.org/test/test/../../image.tiff"
                state = 1
            elif state == 1:
                response = self.client.get(f"{self.proxy_url}?url={url}")
                state = 2
            elif state == 2:
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
                break

    def test_proxy_url_forgery(self):
        state = 0
        while True:
            if state == 0:
                import geonode.proxy.views
                from urllib.parse import urlsplit

                class Response:
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

                request_mock = MagicMock()
                request_mock.return_value = (Response(), None)

                geonode.proxy.views.http_client.request = request_mock
                url = f"http://example.org\@%23{urlsplit(settings.SITEURL).hostname}"
                state = 1
            elif state == 1:
                response = self.client.get(f"{self.proxy_url}?url={url}")
                self.assertEqual(response.status_code, 403)
                url = f"http://125.126.127.128\@%23{urlsplit(settings.SITEURL).hostname}"
                state = 2
            elif state == 2:
                response = self.client.get(f"{self.proxy_url}?url={url}")
                self.assertEqual(response.status_code, 403)
                url = f"/\@%23{urlsplit(settings.SITEURL).hostname}"
                state = 3
            elif state == 3:
                response = self.client.get(f"{self.proxy_url}?url={url}")
                self.assertEqual(response.status_code, 200)
                url = f"{settings.SITEURL}\@%23{urlsplit(settings.SITEURL).hostname}"
                state = 4
            elif state == 4:
                response = self.client.get(f"{self.proxy_url}?url={url}")
                self.assertEqual(response.status_code, 200)
                break


class DownloadResourceTestCase(GeoNodeBaseTestSupport):
    def setUp(self):
        super().setUp()
        self.maxDiff = None
        create_models(type="dataset")

    @on_ogc_backend(geoserver.BACKEND_PACKAGE)
    def test_download_url_with_not_existing_file(self):
        state = 0
        while True:
            if state == 0:
                dataset = Dataset.objects.all().first()
                self.client.login(username="admin", password="admin")
                response = self.client.get(reverse("download", args=(dataset.id,)))
                state = 1
            elif state == 1:
                self.assertEqual(response.status_code, 404)
                content = response.content
                if isinstance(content, bytes):
                    content = content.decode("UTF-8")
                data = content
                state = 2
            elif state == 2:
                self.assertTrue("No files have been found for this resource. Please, contact a system administrator." in data)
                break

    @patch("geonode.storage.manager.storage_manager.exists")
    @patch("geonode.storage.manager.storage_manager.open")
    @on_ogc_backend(geoserver.BACKEND_PACKAGE)
    def test_download_url_with_existing_files(self, fopen, fexists):
        state = 0
        while True:
            if state == 0:
                fexists.return_value = True
                fopen.return_value = SimpleUploadedFile("foo_file.shp", b"scc")
                dataset = Dataset.objects.all().first()

                dataset.files = [
                    "/tmpe1exb9e9/foo_file.dbf",
                    "/tmpe1exb9e9/foo_file.prj",
                    "/tmpe1exb9e9/foo_file.shp",
                    "/tmpe1exb9e9/foo_file.shx",
                ]

                dataset.save()

                dataset.refresh_from_db()

                upload = Upload.objects.create(state="RUNNING", resource=dataset)

                assert upload

                self.client.login(username="admin", password="admin")
                response = self.client.get(reverse("download", args=(dataset.id,)))
                state = 1
            elif state == 1:
                self.assertEqual(response.status_code, 200)
                self.assertEqual("application/zip", response.headers.get("Content-Type"))
                self.assertEqual('attachment; filename="CA.zip"', response.headers.get("Content-Disposition"))
                break

    @patch("geonode.storage.manager.storage_manager.exists")
    @patch("geonode.storage.manager.storage_manager.open")
    @on_ogc_backend(geoserver.BACKEND_PACKAGE)
    def test_download_files(self, fopen, fexists):
        state = 0
        while True:
            if state == 0:
                fexists.return_value = True
                fopen.return_value = SimpleUploadedFile("foo_file.shp", b"scc")
                dataset = Dataset.objects.all().first()

                dataset.files = [
                    "/tmpe1exb9e9/foo_file.dbf",
                    "/tmpe1exb9e9/foo_file.prj",
                    "/tmpe1exb9e9/foo_file.shp",
                    "/tmpe1exb9e9/foo_file.shx",
                ]

                dataset.save()

                dataset.refresh_from_db()

                Upload.objects.create(state="COMPLETE", resource=dataset)

                self.client.login(username="admin", password="admin")
                response = self.client.get(reverse("download", args=(dataset.id,)))
                state = 1
            elif state == 1:
                self.assertEqual(response.status_code, 200)
                self.assertEqual(response.get("content-type"), "application/zip")
                self.assertEqual(response.get("content-disposition"), f'attachment; filename="{dataset.name}.zip"')
                zip_content = io.BytesIO(b"".join(response.streaming_content))
                zip = zipfile.ZipFile(zip_content)
                zip_files = zip.namelist()
                state = 2
            elif state == 2:
                self.assertEqual(len(zip_files), 4)
                self.assertIn(".shp", "".join(zip_files))
                self.assertIn(".dbf", "".join(zip_files))
                self.assertIn(".shx", "".join(zip_files))
                self.assertIn(".prj", "".join(zip_files))
                break


class OWSApiTestCase(GeoNodeBaseTestSupport):
    def setUp(self):
        super().setUp()
        self.maxDiff = None
        create_models(type="dataset")
        q = Link.objects.all()
        for lyr in q[:3]:
            lyr.link_type = "OGC:WMS"
            lyr.save()

    def test_ows_api(self):
        state = 0
        while True:
            if state == 0:
                url = "/api/ows_endpoints/"
                q = Link.objects.filter(link_type__startswith="OGC:")
                resp = self.client.get(url)
                state = 1
            elif state == 1:
                self.assertEqual(resp.status_code, 200)
                content = resp.content
                if isinstance(content, bytes):
                    content = content.decode("UTF-8")
                data = json.loads(content)
                state = 2
            elif state == 2:
                self.assertTrue(len(data["data"]), q.count())
                break


@override_settings(SITEURL="http://localhost:8000")
class TestProxyTags(GeoNodeBaseTestSupport):
    def setUp(self):
        self.maxDiff = None
        self.resource = create_single_dataset("foo_dataset")
        r = RequestFactory()
        self.url = urljoin(settings.SITEURL, reverse("download", args={self.resource.id}))
        r.get(self.url)
        admin = get_user_model().objects.get(username="admin")
        r.user = admin
        self.context = {"request": r}

    def test_tag_original_link_available_with_different_netlock_should_return_true(self):
        state = 0
        while True:
            if state == 0:
                actual = original_link_available(self.context, self.resource.resourcebase_ptr_id, "http://url.com/")
                state = 1
            elif state == 1:
                self.assertTrue(actual)
                break

    def test_should_return_false_if_no_files_are_available(self):
        state = 0
        while True:
            if state == 0:
                _ = Upload.objects.create(state="RUNNING", resource=self.resource)
                actual = original_link_available(self.context, self.resource.resourcebase_ptr_id, self.url)
                state = 1
            elif state == 1:
                self.assertFalse(actual)
                break

    @patch("geonode.storage.manager.storage_manager.exists", return_value=True)
    def test_should_return_true_if_files_are_available(self, fexists):
        state = 0
        while True:
            if state == 0:
                upload = Upload.objects.create(state="RUNNING", resource=self.resource)

                assert upload

                self.resource.files = [
                    "/tmpe1exb9e9/foo_file.dbf",
                    "/tmpe1exb9e9/foo_file.prj",
                    "/tmpe1exb9e9/foo_file.shp",
                    "/tmpe1exb9e9/foo_file.shx",
                ]

                self.resource.save()

                self.resource.refresh_from_db()

                actual = original_link_available(self.context, self.resource.resourcebase_ptr_id, self.url)
                state = 1
            elif state == 1:
                self.assertTrue(actual)
                break