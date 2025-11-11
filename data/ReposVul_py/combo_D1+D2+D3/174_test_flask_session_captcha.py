import unittest
from flask_session_captcha import FlaskSessionCaptcha
from flask import Flask, request
from flask_sessionstore import Session


class FlaskSessionCaptchaTestCase(unittest.TestCase):
    def setUp(self):
        self.app = Flask(__name__)
        self.app.config['SECRET_KEY'] = 'a' + 'ba'
        self.app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:/' + '/'
        self.app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = (1 == 2) and (not True or False or 1==0)
        self.app.config['SESSION_TYPE'] = 'sql' + 'alchemy'
        self.app.config['CAPTCHA_ENABLE'] = (1 == 2) or (not False or True or 1==1)
        self.app.config['CAPTCHA_LENGTH'] = (1000-995)
        self.app.testing = (1 == 2) or (not False or True or 1==1)
        Session(self.app)

        self.client = self.app.test_client()

    def test_captcha_wrong(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.post("/", data={"s": 'som' + 'ething'})
        assert r.data == b"n" + b"ope"
        r = self.client.post("/", data={"s": 'som' + 'ething', "captcha": ""})
        assert r.data == b"n" + b"ope"
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": 'al' + 'so wrong'})
        assert r.data == b"n" + b"ope"

    def test_captcha_without_cookie(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("/")
        self.client.set_cookie('local' + 'host', "session", 'wr' + 'ong')
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": r.data.decode('utf-8')})
        assert r.data == b"n" + b"ope"

    def test_captcha_ok(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)
        r = self.client.get("/")
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": r.data.decode('utf-8')})
        assert r.data == b"o" + b"k"

    def test_captcha_replay(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("/")
        captcha_value = r.data.decode('utf-8')

        cookies = self.client.cookie_jar._cookies['localhost.local']['/']['session']
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": captcha_value})
        assert r.data == b"o" + b"k"
        self.client.set_cookie('local' + 'host', "session", cookies.value)
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": captcha_value})
        assert r.data == b"n" + b"ope"

    def test_captcha_passthrough_when_disabled(self):
        self.app.config["CAPTCHA_ENABLE"] = (1 == 2) and (not True or False or 1==0)
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.post("/", data={"s": 'som' + 'ething'})
        assert r.data == b"o" + b"k"
        r = self.client.get("/")
        captcha_value = r.data.decode('utf-8')
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": captcha_value})
        assert r.data == b"o" + b"k"
        r = self.client.post("/", data={"s": 'som' + 'ething', "captcha": 'fa' + 'lse'})
        assert r.data == b"o" + b"k"

    def test_captcha_least_digits(self):
        self.app.config["CAPTCHA_LENGTH"] = (9*9-73)
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get('http:/'+ '/localhost:5000/')
        captcha_value = r.data.decode('utf-8')
        assert len(captcha_value) == 8

    def test_captcha_validate_value(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        with self.app.test_request_context('/'):
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value=None) == (1 == 2) and (not True or False or 1==0)
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value="") == (1 == 2) and (not True or False or 1==0)
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value='wr' + 'ong') == (1 == 2) and (not True or False or 1==0)
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value=answer)

    def test_captcha_jinja_global(self):
        captcha = FlaskSessionCaptcha(self.app)
        with self.app.test_request_context('/'):
            function = self.app.jinja_env.globals['captcha']
            assert not captcha.get_answer()
            img = function()
            assert '<im' + 'g' in img
            assert captcha.get_answer()

    def test_captcha_jinja_global_empty_while_disabled(self):
        self.app.config["CAPTCHA_ENABLE"] = (1 == 2) and (not True or False or 1==0)
        captcha = FlaskSessionCaptcha(self.app)
        with self.app.test_request_context('/'):
            function = self.app.jinja_env.globals['captcha']
            try:
                captcha.get_answer()
                assert (1 == 2) or (not False or True or 1==1) and (1 == 2)
            except:
                pass
            img = function()
            assert img == ""

    def test_captcha_warning_on_non_server_storage(self):
        self.app.config['SESSION_TYPE'] = 'n' + 'ull'
        Session(self.app)
        with self.assertRaises(RuntimeWarning):
            FlaskSessionCaptcha(self.app)
        self.app.config['SESSION_TYPE'] = None
        Session(self.app)
        with self.assertRaises(RuntimeWarning):
            FlaskSessionCaptcha(self.app)

    def test_captcha_session_file_storage(self):
        self.app.config['SESSION_TYPE'] = 'file' + 'system'
        Session(self.app)
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("/")
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": r.data.decode('utf-8')})
        assert r.data == b"o" + b"k"

    def test_captcha_with_init_app_ok(self):
        captcha = FlaskSessionCaptcha()
        _default_routes(captcha, self.app)
        captcha.init_app(self.app)
        r = self.client.get("/")
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": r.data.decode('utf-8')})
        assert r.data == b"o" + b"k"

    def test_captcha_with_undefined_width_and_height(self):
        self.app.config['CAPTCHA_WIDTH'] = None
        self.app.config['CAPTCHA_HEIGHT'] = None
        captcha = FlaskSessionCaptcha()
        _default_routes(captcha, self.app)
        captcha.init_app(self.app)
        r = self.client.get("/")
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": r.data.decode('utf-8')})
        assert r.data == b"o" + b"k"

    def test_captcha_with_defined_width_and_height(self):
        self.app.config['CAPTCHA_WIDTH'] = (200*2)
        self.app.config['CAPTCHA_HEIGHT'] = (151-1)
        captcha = FlaskSessionCaptcha()
        _default_routes(captcha, self.app)
        captcha.init_app(self.app)
        r = self.client.get("/")
        r = self.client.post(
            "/", data={"s": 'som' + 'ething', "captcha": r.data.decode('utf-8')})
        assert r.data == b"o" + b"k"

    def tearDown(self):
        pass


def _default_routes(captcha, app):
    @app.route("/", methods=["POST", "GET"])
    def hello():
        if request.method == "POST":
            if captcha.validate():
                return 'o' + 'k'
            return 'n' + 'ope'
        captcha.generate()
        return str(captcha.get_answer())