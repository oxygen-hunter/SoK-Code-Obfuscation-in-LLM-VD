import unittest
from flask_session_captcha import FlaskSessionCaptcha
from flask import Flask, request
from flask_sessionstore import Session

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
        dispatcher = 0
        while True:
            if dispatcher == 0:
                captcha = FlaskSessionCaptcha(self.app)
                _default_routes(captcha, self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.post("/", data={"s": "something"})
                dispatcher = 2
            elif dispatcher == 2:
                assert r.data == b"nope"
                r = self.client.post("/", data={"s": "something", "captcha": ""})
                dispatcher = 3
            elif dispatcher == 3:
                assert r.data == b"nope"
                r = self.client.post("/", data={"s": "something", "captcha": "also wrong"})
                dispatcher = 4
            elif dispatcher == 4:
                assert r.data == b"nope"
                break

    def test_captcha_without_cookie(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                captcha = FlaskSessionCaptcha(self.app)
                _default_routes(captcha, self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.get("/")
                self.client.set_cookie("localhost", "session", "wrong")
                r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
                dispatcher = 2
            elif dispatcher == 2:
                assert r.data == b"nope"
                break

    def test_captcha_ok(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                captcha = FlaskSessionCaptcha(self.app)
                _default_routes(captcha, self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.get("/")
                r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
                dispatcher = 2
            elif dispatcher == 2:
                assert r.data == b"ok"
                break

    def test_captcha_replay(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                captcha = FlaskSessionCaptcha(self.app)
                _default_routes(captcha, self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.get("/")
                captcha_value = r.data.decode('utf-8')
                cookies = self.client.cookie_jar._cookies['localhost.local']['/']['session']
                dispatcher = 2
            elif dispatcher == 2:
                r = self.client.post("/", data={"s": "something", "captcha": captcha_value})
                assert r.data == b"ok"
                self.client.set_cookie("localhost", "session", cookies.value)
                dispatcher = 3
            elif dispatcher == 3:
                r = self.client.post("/", data={"s": "something", "captcha": captcha_value})
                assert r.data == b"nope"
                break

    def test_captcha_passthrough_when_disabled(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.app.config["CAPTCHA_ENABLE"] = False
                captcha = FlaskSessionCaptcha(self.app)
                _default_routes(captcha, self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.post("/", data={"s": "something"})
                assert r.data == b"ok"
                dispatcher = 2
            elif dispatcher == 2:
                r = self.client.get("/")
                captcha_value = r.data.decode('utf-8')
                dispatcher = 3
            elif dispatcher == 3:
                r = self.client.post("/", data={"s": "something", "captcha": captcha_value})
                assert r.data == b"ok"
                dispatcher = 4
            elif dispatcher == 4:
                r = self.client.post("/", data={"s": "something", "captcha": "false"})
                assert r.data == b"ok"
                break

    def test_captcha_least_digits(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.app.config["CAPTCHA_LENGTH"] = 8
                captcha = FlaskSessionCaptcha(self.app)
                _default_routes(captcha, self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.get("http://localhost:5000/")
                captcha_value = r.data.decode('utf-8')
                assert len(captcha_value) == 8
                break

    def test_captcha_validate_value(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                captcha = FlaskSessionCaptcha(self.app)
                _default_routes(captcha, self.app)
                dispatcher = 1
            elif dispatcher == 1:
                with self.app.test_request_context('/'):
                    captcha.generate()
                    answer = captcha.get_answer()
                    assert captcha.validate(value=None) == False
                    dispatcher = 2
            elif dispatcher == 2:
                captcha.generate()
                answer = captcha.get_answer()
                assert captcha.validate(value="") == False
                dispatcher = 3
            elif dispatcher == 3:
                captcha.generate()
                answer = captcha.get_answer()
                assert captcha.validate(value="wrong") == False
                dispatcher = 4
            elif dispatcher == 4:
                captcha.generate()
                answer = captcha.get_answer()
                assert captcha.validate(value=answer)
                break

    def test_captcha_jinja_global(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                captcha = FlaskSessionCaptcha(self.app)
                dispatcher = 1
            elif dispatcher == 1:
                with self.app.test_request_context('/'):
                    function = self.app.jinja_env.globals['captcha']
                    assert not captcha.get_answer()
                    img = function()
                    dispatcher = 2
            elif dispatcher == 2:
                assert "<img" in img
                assert captcha.get_answer()
                break

    def test_captcha_jinja_global_empty_while_disabled(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.app.config["CAPTCHA_ENABLE"] = False
                captcha = FlaskSessionCaptcha(self.app)
                dispatcher = 1
            elif dispatcher == 1:
                with self.app.test_request_context('/'):
                    function = self.app.jinja_env.globals['captcha']
                    try:
                        captcha.get_answer()
                        assert False
                    except:
                        pass
                    dispatcher = 2
            elif dispatcher == 2:
                img = function()
                assert img == ""
                break

    def test_captcha_warning_on_non_server_storage(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.app.config['SESSION_TYPE'] = 'null'
                Session(self.app)
                dispatcher = 1
            elif dispatcher == 1:
                with self.assertRaises(RuntimeWarning):
                    FlaskSessionCaptcha(self.app)
                self.app.config['SESSION_TYPE'] = None
                Session(self.app)
                dispatcher = 2
            elif dispatcher == 2:
                with self.assertRaises(RuntimeWarning):
                    FlaskSessionCaptcha(self.app)
                break

    def test_captcha_session_file_storage(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.app.config['SESSION_TYPE'] = 'filesystem'
                Session(self.app)
                captcha = FlaskSessionCaptcha(self.app)
                _default_routes(captcha, self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.get("/")
                r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
                assert r.data == b"ok"
                break

    def test_captcha_with_init_app_ok(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                captcha = FlaskSessionCaptcha()
                _default_routes(captcha, self.app)
                captcha.init_app(self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.get("/")
                r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
                assert r.data == b"ok"
                break

    def test_captcha_with_undefined_width_and_height(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.app.config['CAPTCHA_WIDTH'] = None
                self.app.config['CAPTCHA_HEIGHT'] = None
                captcha = FlaskSessionCaptcha()
                _default_routes(captcha, self.app)
                captcha.init_app(self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.get("/")
                r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
                assert r.data == b"ok"
                break

    def test_captcha_with_defined_width_and_height(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.app.config['CAPTCHA_WIDTH'] = 400
                self.app.config['CAPTCHA_HEIGHT'] = 150
                captcha = FlaskSessionCaptcha()
                _default_routes(captcha, self.app)
                captcha.init_app(self.app)
                dispatcher = 1
            elif dispatcher == 1:
                r = self.client.get("/")
                r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
                assert r.data == b"ok"
                break

    def tearDown(self):
        pass

def _default_routes(captcha, app):
    @app.route("/", methods=["POST", "GET"])
    def hello():
        if request.method == "POST":
            if captcha.validate():
                return "ok"
            return "nope"
        captcha.generate()
        return str(captcha.get_answer())