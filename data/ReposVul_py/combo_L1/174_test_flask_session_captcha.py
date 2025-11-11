import unittest
from flask_session_captcha import FlaskSessionCaptcha as OX8A9BFC4A
from flask import Flask as OX0F7A7AFA, request as OX5E1F8A6C
from flask_sessionstore import Session as OX1B3E1F9E


class OX3D7B4DF3(unittest.TestCase):
    def OX1E9B2A7C(self):
        self.OX8B3DCE9D = OX0F7A7AFA(__name__)
        self.OX8B3DCE9D.config['SECRET_KEY'] = 'aba'
        self.OX8B3DCE9D.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite://'
        self.OX8B3DCE9D.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
        self.OX8B3DCE9D.config['SESSION_TYPE'] = 'sqlalchemy'
        self.OX8B3DCE9D.config['CAPTCHA_ENABLE'] = True
        self.OX8B3DCE9D.config['CAPTCHA_LENGTH'] = 5
        self.OX8B3DCE9D.testing = True
        OX1B3E1F9E(self.OX8B3DCE9D)

        self.OX5C2F4A7D = self.OX8B3DCE9D.test_client()

    def OX6E2D1A9C(self):
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)

        OX4E1A5F6B = self.OX5C2F4A7D.post("/", data={"s": "something"})
        assert OX4E1A5F6B.data == b"nope"
        OX4E1A5F6B = self.OX5C2F4A7D.post("/", data={"s": "something", "captcha": ""})
        assert OX4E1A5F6B.data == b"nope"
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": "also wrong"})
        assert OX4E1A5F6B.data == b"nope"

    def OX9B2D4A6C(self):
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)

        OX4E1A5F6B = self.OX5C2F4A7D.get("/")
        self.OX5C2F4A7D.set_cookie("localhost", "session", "wrong")
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": OX4E1A5F6B.data.decode('utf-8')})
        assert OX4E1A5F6B.data == b"nope"

    def OX5C3F7A8D(self):
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)

        OX4E1A5F6B = self.OX5C2F4A7D.get("/")
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": OX4E1A5F6B.data.decode('utf-8')})
        assert OX4E1A5F6B.data == b"ok"

    def OX4B2F9C5A(self):
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)

        OX4E1A5F6B = self.OX5C2F4A7D.get("/")
        OX8A9D6F3B = OX4E1A5F6B.data.decode('utf-8')

        OX4C5D7E9F = self.OX5C2F4A7D.cookie_jar._cookies['localhost.local']['/']['session']
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": OX8A9D6F3B})
        assert OX4E1A5F6B.data == b"ok"
        self.OX5C2F4A7D.set_cookie("localhost", "session", OX4C5D7E9F.value)
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": OX8A9D6F3B})
        assert OX4E1A5F6B.data == b"nope"

    def OX7E1D3A6B(self):
        self.OX8B3DCE9D.config["CAPTCHA_ENABLE"] = False
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)

        OX4E1A5F6B = self.OX5C2F4A7D.post("/", data={"s": "something"})
        assert OX4E1A5F6B.data == b"ok"
        OX4E1A5F6B = self.OX5C2F4A7D.get("/")
        OX8A9D6F3B = OX4E1A5F6B.data.decode('utf-8')
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": OX8A9D6F3B})
        assert OX4E1A5F6B.data == b"ok"
        OX4E1A5F6B = self.OX5C2F4A7D.post("/", data={"s": "something", "captcha": "false"})
        assert OX4E1A5F6B.data == b"ok"

    def OX8E1F3A9C(self):
        self.OX8B3DCE9D.config["CAPTCHA_LENGTH"] = 8
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)

        OX4E1A5F6B = self.OX5C2F4A7D.get("http://localhost:5000/")
        OX8A9D6F3B = OX4E1A5F6B.data.decode('utf-8')
        assert len(OX8A9D6F3B) == 8

    def OX3C2B9D7A(self):
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)

        with self.OX8B3DCE9D.test_request_context('/'):
            OX7A4C5D9E.generate()
            OX9B3C8A5D = OX7A4C5D9E.get_answer()
            assert OX7A4C5D9E.validate(value=None) == False
            OX7A4C5D9E.generate()
            OX9B3C8A5D = OX7A4C5D9E.get_answer()
            assert OX7A4C5D9E.validate(value="") == False
            OX7A4C5D9E.generate()
            OX9B3C8A5D = OX7A4C5D9E.get_answer()
            assert OX7A4C5D9E.validate(value="wrong") == False
            OX7A4C5D9E.generate()
            OX9B3C8A5D = OX7A4C5D9E.get_answer()
            assert OX7A4C5D9E.validate(value=OX9B3C8A5D)

    def OX6D1C3A9F(self):
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        with self.OX8B3DCE9D.test_request_context('/'):
            OX2F8C7A9B = self.OX8B3DCE9D.jinja_env.globals['captcha']
            assert not OX7A4C5D9E.get_answer()
            OX5C1A9E7D = OX2F8C7A9B()
            assert "<img" in OX5C1A9E7D
            assert OX7A4C5D9E.get_answer()

    def OX5A8F3C1D(self):
        self.OX8B3DCE9D.config["CAPTCHA_ENABLE"] = False
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        with self.OX8B3DCE9D.test_request_context('/'):
            OX2F8C7A9B = self.OX8B3DCE9D.jinja_env.globals['captcha']
            try:
                OX7A4C5D9E.get_answer()
                assert False
            except:
                pass
            OX5C1A9E7D = OX2F8C7A9B()
            assert OX5C1A9E7D == ""

    def OX9E3D1A7B(self):
        self.OX8B3DCE9D.config['SESSION_TYPE'] = 'null'
        OX1B3E1F9E(self.OX8B3DCE9D)
        with self.assertRaises(RuntimeWarning):
            OX8A9BFC4A(self.OX8B3DCE9D)
        self.OX8B3DCE9D.config['SESSION_TYPE'] = None
        OX1B3E1F9E(self.OX8B3DCE9D)
        with self.assertRaises(RuntimeWarning):
            OX8A9BFC4A(self.OX8B3DCE9D)

    def OX7C6D1F2A(self):
        self.OX8B3DCE9D.config['SESSION_TYPE'] = 'filesystem'
        OX1B3E1F9E(self.OX8B3DCE9D)
        OX7A4C5D9E = OX8A9BFC4A(self.OX8B3DCE9D)
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)

        OX4E1A5F6B = self.OX5C2F4A7D.get("/")
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": OX4E1A5F6B.data.decode('utf-8')})
        assert OX4E1A5F6B.data == b"ok"

    def OX8B9D2F3C(self):
        OX7A4C5D9E = OX8A9BFC4A()
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)
        OX7A4C5D9E.init_app(self.OX8B3DCE9D)

        OX4E1A5F6B = self.OX5C2F4A7D.get("/")
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": OX4E1A5F6B.data.decode('utf-8')})
        assert OX4E1A5F6B.data == b"ok"

    def OX5B8D2A7F(self):
        self.OX8B3DCE9D.config['CAPTCHA_WIDTH'] = None
        self.OX8B3DCE9D.config['CAPTCHA_HEIGHT'] = None
        OX7A4C5D9E = OX8A9BFC4A()
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)
        OX7A4C5D9E.init_app(self.OX8B3DCE9D)
        OX4E1A5F6B = self.OX5C2F4A7D.get("/")
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": OX4E1A5F6B.data.decode('utf-8')})
        assert OX4E1A5F6B.data == b"ok"

    def OX3E9A4C1D(self):
        self.OX8B3DCE9D.config['CAPTCHA_WIDTH'] = 400
        self.OX8B3DCE9D.config['CAPTCHA_HEIGHT'] = 150
        OX7A4C5D9E = OX8A9BFC4A()
        OXA9C4C7D1(OX7A4C5D9E, self.OX8B3DCE9D)
        OX7A4C5D9E.init_app(self.OX8B3DCE9D)
        OX4E1A5F6B = self.OX5C2F4A7D.get("/")
        OX4E1A5F6B = self.OX5C2F4A7D.post(
            "/", data={"s": "something", "captcha": OX4E1A5F6B.data.decode('utf-8')})
        assert OX4E1A5F6B.data == b"ok"

    def OX7E2B5D9C(self):
        pass


def OXA9C4C7D1(OX7A4C5D9E, OX8B3DCE9D):
    @OX8B3DCE9D.route("/", methods=["POST", "GET"])
    def OX1A9B4D7E():
        if OX5E1F8A6C.method == "POST":
            if OX7A4C5D9E.validate():
                return "ok"
            return "nope"
        OX7A4C5D9E.generate()
        return str(OX7A4C5D9E.get_answer())