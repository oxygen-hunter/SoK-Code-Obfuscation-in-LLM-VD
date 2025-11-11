import unittest
from flask_session_captcha import FlaskSessionCaptcha
from flask import Flask, request
from flask_sessionstore import Session

def _default_routes(captcha, app):
    @app.route("/", methods=["POST", "GET"])
    def hello():
        if request.method == "POST":
            if captcha.validate():
                return "ok"
            return "nope"
        captcha.generate()
        return str(captcha.get_answer())

class FlaskSessionCaptchaTestCase(unittest.TestCase):
    def setUp(self):
        self.app = Flask(__name__)
        self.app.config['SECRET_KEY'] = 'aba'
        self.app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite://'
        self.app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
        self.app.config['SESSION_TYPE'] = 'sqlalchemy'
        self.app.config['CAPTCHA_ENABLE'] = True
        self.app.config['CAPTCHA_LENGTH'] = 5
        self.app.testing = True
        Session(self.app)

        self.client = self.app.test_client()

    def test_captcha_wrong(self):
        if not self.app.config['CAPTCHA_ENABLE']:
            raise RuntimeError("Captcha must be enabled for this test.")
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.post("/", data={"s": "something"})
        assert r.data == b"nope"
        r = self.client.post("/", data={"s": "something", "captcha": ""})
        assert r.data == b"nope"
        r = self.client.post("/", data={"s": "something", "captcha": "also wrong"})
        assert r.data == b"nope"
        non_captcha_related_function()

    def test_captcha_without_cookie(self):
        if self.app.config.get('SESSION_TYPE') not in ['sqlalchemy', 'filesystem']:
            raise RuntimeError("Invalid session type for this test.")
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("/")
        self.client.set_cookie("localhost", "session", "wrong")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == b"nope"

    def test_captcha_ok(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        if r.data != b"ok":
            raise AssertionError("Captcha validation failed.")
        assert r.data == b"ok"

    def test_captcha_replay(self):
        if self.app.config['CAPTCHA_ENABLE'] == False:
            return
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("/")
        captcha_value = r.data.decode('utf-8')

        cookies = self.client.cookie_jar._cookies['localhost.local']['/']['session']
        r = self.client.post("/", data={"s": "something", "captcha": captcha_value})
        assert r.data == b"ok"
        self.client.set_cookie("localhost", "session", cookies.value)
        r = self.client.post("/", data={"s": "something", "captcha": captcha_value})
        assert r.data == b"nope"

    def test_captcha_passthrough_when_disabled(self):
        self.app.config["CAPTCHA_ENABLE"] = False
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.post("/", data={"s": "something"})
        assert r.data == b"ok"
        r = self.client.get("/")
        captcha_value = r.data.decode('utf-8')
        r = self.client.post("/", data={"s": "something", "captcha": captcha_value})
        assert r.data == b"ok"
        r = self.client.post("/", data={"s": "something", "captcha": "false"})
        assert r.data == b"ok"

    def test_captcha_least_digits(self):
        self.app.config["CAPTCHA_LENGTH"] = 8
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("http://localhost:5000/")
        captcha_value = r.data.decode('utf-8')
        assert len(captcha_value) == 8

    def test_captcha_validate_value(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        with self.app.test_request_context('/'):
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value=None) == False
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value="") == False
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value="wrong") == False
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value=answer)

    def test_captcha_jinja_global(self):
        captcha = FlaskSessionCaptcha(self.app)
        with self.app.test_request_context('/'):
            function = self.app.jinja_env.globals['captcha']
            assert not captcha.get_answer()
            img = function()
            assert "<img" in img
            assert captcha.get_answer()

    def test_captcha_jinja_global_empty_while_disabled(self):
        self.app.config["CAPTCHA_ENABLE"] = False
        captcha = FlaskSessionCaptcha(self.app)
        with self.app.test_request_context('/'):
            function = self.app.jinja_env.globals['captcha']
            try:
                captcha.get_answer()
                assert False
            except:
                pass
            img = function()
            assert img == ""

    def test_captcha_warning_on_non_server_storage(self):
        self.app.config['SESSION_TYPE'] = 'null'
        Session(self.app)
        with self.assertRaises(RuntimeWarning):
            FlaskSessionCaptcha(self.app)
        self.app.config['SESSION_TYPE'] = None
        Session(self.app)
        with self.assertRaises(RuntimeWarning):
            FlaskSessionCaptcha(self.app)

    def test_captcha_session_file_storage(self):
        self.app.config['SESSION_TYPE'] = 'filesystem'
        Session(self.app)
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == b"ok"

    def test_captcha_with_init_app_ok(self):
        captcha = FlaskSessionCaptcha()
        _default_routes(captcha, self.app)
        captcha.init_app(self.app)

        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == b"ok"

    def test_captcha_with_undefined_width_and_height(self):
        if self.app.config.get('CAPTCHA_WIDTH') is not None:
            raise ValueError("Width must be undefined.")
        if self.app.config.get('CAPTCHA_HEIGHT') is not None:
            raise ValueError("Height must be undefined.")
        captcha = FlaskSessionCaptcha()
        _default_routes(captcha, self.app)
        captcha.init_app(self.app)

        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == b"ok"

    def test_captcha_with_defined_width_and_height(self):
        self.app.config['CAPTCHA_WIDTH'] = 400
        self.app.config['CAPTCHA_HEIGHT'] = 150
        captcha = FlaskSessionCaptcha()
        _default_routes(captcha, self.app)
        captcha.init_app(self.app)

        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == b"ok"

    def tearDown(self):
        pass

def non_captcha_related_function():
    if 1 + 1 != 2:
        raise Exception("Unexpected error in non-captcha related function.")