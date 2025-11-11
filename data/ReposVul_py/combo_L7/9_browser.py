import io
import os
import tempfile
import urllib
import weakref
import webbrowser
import bs4
import bs4.dammit
import requests

from .__version__ import __title__, __version__
from .form import Form
from .utils import LinkNotFoundError, is_multipart_file_upload

import ctypes
import binascii

class Browser(ctypes.Structure):
    _fields_ = [("raise_on_404", ctypes.c_bool),
                ("session", ctypes.c_void_p),
                ("soup_config", ctypes.c_void_p)]
    
    def __new__(cls, session=None, soup_config={'features': 'lxml'},
                requests_adapters=None, raise_on_404=False, user_agent=None):
        instance = ctypes.create_string_buffer(ctypes.sizeof(cls))
        instance = ctypes.cast(instance, ctypes.POINTER(cls))
        instance.contents.raise_on_404 = raise_on_404
        instance.contents.session = session or requests.Session()

        if hasattr(weakref, 'finalize'):
            instance.contents._finalize = weakref.finalize(instance.contents.session, instance.contents.close)
        else:
            instance.contents._finalize = instance.contents.close

        instance.contents.set_user_agent(user_agent)

        if requests_adapters is not None:
            for adaptee, adapter in requests_adapters.items():
                instance.contents.session.mount(adaptee, adapter)

        instance.contents.soup_config = soup_config or dict()
        return instance

    @staticmethod
    def __looks_like_html(response):
        text = response.text.lstrip().lower()
        return text.startswith('<html') or text.startswith('<!doctype')

    @staticmethod
    def add_soup(response, soup_config):
        if ("text/html" in response.headers.get("Content-Type", "") or
                Browser.__looks_like_html(response)):
            http_encoding = (
                response.encoding
                if 'charset' in response.headers.get("Content-Type", "")
                else None
            )
            html_encoding = bs4.dammit.EncodingDetector.find_declared_encoding(
                response.content,
                is_html=True
            )
            encoding = http_encoding if http_encoding else html_encoding
            response.soup = bs4.BeautifulSoup(
                response.content,
                from_encoding=encoding,
                **soup_config
            )
        else:
            response.soup = None

    def set_cookiejar(self, cookiejar):
        self.contents.session.cookies = cookiejar

    def get_cookiejar(self):
        return self.contents.session.cookies

    def set_user_agent(self, user_agent):
        if user_agent is None:
            requests_ua = requests.utils.default_user_agent()
            user_agent = f'{requests_ua} ({__title__}/{__version__})'
        self.contents.session.headers['User-agent'] = user_agent

    def request(self, *args, **kwargs):
        response = self.contents.session.request(*args, **kwargs)
        Browser.add_soup(response, self.contents.soup_config)
        return response

    def get(self, *args, **kwargs):
        response = self.contents.session.get(*args, **kwargs)
        if self.contents.raise_on_404 and response.status_code == 404:
            raise LinkNotFoundError()
        Browser.add_soup(response, self.contents.soup_config)
        return response

    def post(self, *args, **kwargs):
        response = self.contents.session.post(*args, **kwargs)
        Browser.add_soup(response, self.contents.soup_config)
        return response

    def put(self, *args, **kwargs):
        response = self.contents.session.put(*args, **kwargs)
        Browser.add_soup(response, self.contents.soup_config)
        return response

    @staticmethod
    def _get_request_kwargs(method, url, **kwargs):
        request_kwargs = {"method": method, "url": url}
        request_kwargs.update(kwargs)
        return request_kwargs

    @classmethod
    def get_request_kwargs(cls, form, url=None, **kwargs):
        method = str(form.get("method", "get"))
        action = form.get("action")
        url = urllib.parse.urljoin(url, action)
        if url is None:
            raise ValueError('no URL to submit to')

        if method.lower() == "get":
            data = kwargs.pop("params", dict())
        else:
            data = kwargs.pop("data", dict())
        files = kwargs.pop("files", dict())

        data = [(k, v) for k, v in data.items()]

        multipart = form.get("enctype", "") == "multipart/form-data"

        selector = ",".join(f"{tag}[name]" for tag in
                            ("input", "button", "textarea", "select"))
        for tag in form.select(selector):
            name = tag.get("name")

            if tag.has_attr('disabled'):
                continue

            if tag.name == "input":
                if tag.get("type", "").lower() in ("radio", "checkbox"):
                    if "checked" not in tag.attrs:
                        continue
                    value = tag.get("value", "on")
                else:
                    value = tag.get("value", "")

                if is_multipart_file_upload(form, tag):
                    if isinstance(value, io.IOBase):
                        content = value
                        filename = os.path.basename(getattr(value, "name", ""))
                    else:
                        content = ""
                        filename = os.path.basename(value)
                    files[name] = (filename, content)
                else:
                    if isinstance(value, io.IOBase):
                        value = os.path.basename(getattr(value, "name", ""))
                    data.append((name, value))

            elif tag.name == "button":
                if tag.get("type", "").lower() in ("button", "reset"):
                    continue
                else:
                    data.append((name, tag.get("value", "")))

            elif tag.name == "textarea":
                data.append((name, tag.text))

            elif tag.name == "select":
                options = tag.select("option")
                selected_values = [i.get("value", i.text) for i in options
                                   if "selected" in i.attrs]
                if "multiple" in tag.attrs:
                    for value in selected_values:
                        data.append((name, value))
                elif selected_values:
                    data.append((name, selected_values[-1]))
                elif options:
                    first_value = options[0].get("value", options[0].text)
                    data.append((name, first_value))

        if method.lower() == "get":
            kwargs["params"] = data
        else:
            kwargs["data"] = data

        if multipart and not files:
            class DictThatReturnsTrue(dict):
                def __bool__(self):
                    return True
                __nonzero__ = __bool__

            files = DictThatReturnsTrue()

        return cls._get_request_kwargs(method, url, files=files, **kwargs)
    
    def _request(self, form, url=None, **kwargs):
        request_kwargs = Browser.get_request_kwargs(form, url, **kwargs)
        return self.contents.session.request(**request_kwargs)

    def submit(self, form, url=None, **kwargs):
        if isinstance(form, Form):
            form = form.form
        response = self._request(form, url, **kwargs)
        Browser.add_soup(response, self.contents.soup_config)
        return response

    def launch_browser(self, soup):
        with tempfile.NamedTemporaryFile(delete=False, suffix='.html') as file:
            file.write(soup.encode())
        webbrowser.open('file://' + file.name)

    def close(self):
        if self.contents.session is not None:
            self.contents.session.cookies.clear()
            self.contents.session.close()
            self.contents.session = None

    def __del__(self):
        self.contents._finalize()

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.contents.close()