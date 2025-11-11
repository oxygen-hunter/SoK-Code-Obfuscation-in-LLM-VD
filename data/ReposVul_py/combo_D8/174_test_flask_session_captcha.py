import unittest
from flask_session_captcha import FlaskSessionCaptcha
from flask import Flask, request
from flask_sessionstore import Session


class FlaskSessionCaptchaTestCase(unittest.TestCase):
    def setUp(self):
        def getValueA(): return 'aba'
        def getValueB(): return 'sqlite://'
        def getValueC(): return False
        def getValueD(): return 'sqlalchemy'
        def getValueE(): return True
        def getValueF(): return 5

        self.app = Flask(__name__)
        self.app.config['SECRET_KEY'] = getValueA()
        self.app.config['SQLALCHEMY_DATABASE_URI'] = getValueB()
        self.app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = getValueC()
        self.app.config['SESSION_TYPE'] = getValueD()
        self.app.config['CAPTCHA_ENABLE'] = getValueE()
        self.app.config['CAPTCHA_LENGTH'] = getValueF()
        self.app.testing = getValueE()
        Session(self.app)

        self.client = self.app.test_client()

    def test_captcha_wrong(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        def getValueG(): return "something"
        def getValueH(): return ""
        def getValueI(): return "also wrong"
        def getValueJ(): return b"nope"

        r = self.client.post("/", data={"s": getValueG()})
        assert r.data == getValueJ()
        r = self.client.post("/", data={"s": getValueG(), "captcha": getValueH()})
        assert r.data == getValueJ()
        r = self.client.post("/", data={"s": getValueG(), "captcha": getValueI()})
        assert r.data == getValueJ()

    def test_captcha_without_cookie(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        def getValueK(): return "localhost"
        def getValueL(): return "session"
        def getValueM(): return "wrong"
        def getValueN(): return b"nope"

        r = self.client.get("/")
        self.client.set_cookie(getValueK(), getValueL(), getValueM())
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == getValueN()

    def test_captcha_ok(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        def getValueO(): return b"ok"

        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == getValueO()

    def test_captcha_replay(self):
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        def getValueP(): return b"ok"
        def getValueQ(): return b"nope"

        r = self.client.get("/")
        captcha_value = r.data.decode('utf-8')

        cookies = self.client.cookie_jar._cookies['localhost.local']['/']['session']
        r = self.client.post("/", data={"s": "something", "captcha": captcha_value})
        assert r.data == getValueP()
        self.client.set_cookie("localhost", "session", cookies.value)
        r = self.client.post("/", data={"s": "something", "captcha": captcha_value})
        assert r.data == getValueQ()

    def test_captcha_passthrough_when_disabled(self):
        def getValueR(): return False
        def getValueS(): return b"ok"
        def getValueT(): return "false"

        self.app.config["CAPTCHA_ENABLE"] = getValueR()
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.post("/", data={"s": "something"})
        assert r.data == getValueS()
        r = self.client.get("/")
        captcha_value = r.data.decode('utf-8')
        r = self.client.post("/", data={"s": "something", "captcha": captcha_value})
        assert r.data == getValueS()
        r = self.client.post("/", data={"s": "something", "captcha": getValueT()})
        assert r.data == getValueS()

    def test_captcha_least_digits(self):
        def getValueU(): return 8

        self.app.config["CAPTCHA_LENGTH"] = getValueU()
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("http://localhost:5000/")
        captcha_value = r.data.decode('utf-8')
        assert len(captcha_value) == getValueU()

    def test_captcha_validate_value(self):
        def getValueV(): return False

        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        with self.app.test_request_context('/'):
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value=None) == getValueV()
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value="") == getValueV()
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value="wrong") == getValueV()
            captcha.generate()
            answer = captcha.get_answer()
            assert captcha.validate(value=answer)

    def test_captcha_jinja_global(self):
        def getValueW(): return "<img"

        captcha = FlaskSessionCaptcha(self.app)
        with self.app.test_request_context('/'):
            function = self.app.jinja_env.globals['captcha']
            assert not captcha.get_answer()
            img = function()
            assert getValueW() in img
            assert captcha.get_answer()

    def test_captcha_jinja_global_empty_while_disabled(self):
        def getValueX(): return False
        def getValueY(): return ""

        self.app.config["CAPTCHA_ENABLE"] = getValueX()
        captcha = FlaskSessionCaptcha(self.app)
        with self.app.test_request_context('/'):
            function = self.app.jinja_env.globals['captcha']
            try:
                captcha.get_answer()
                assert getValueX()
            except:
                pass
            img = function()
            assert img == getValueY()

    def test_captcha_warning_on_non_server_storage(self):
        def getValueZ(): return 'null'
        def getValueAA(): return None

        self.app.config['SESSION_TYPE'] = getValueZ()
        Session(self.app)
        with self.assertRaises(RuntimeWarning):
            FlaskSessionCaptcha(self.app)
        self.app.config['SESSION_TYPE'] = getValueAA()
        Session(self.app)
        with self.assertRaises(RuntimeWarning):
            FlaskSessionCaptcha(self.app)

    def test_captcha_session_file_storage(self):
        def getValueAB(): return 'filesystem'
        def getValueAC(): return b"ok"

        self.app.config['SESSION_TYPE'] = getValueAB()
        Session(self.app)
        captcha = FlaskSessionCaptcha(self.app)
        _default_routes(captcha, self.app)

        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == getValueAC()

    def test_captcha_with_init_app_ok(self):
        def getValueAD(): return b"ok"

        captcha = FlaskSessionCaptcha()
        _default_routes(captcha, self.app)
        captcha.init_app(self.app)
        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == getValueAD()

    def test_captcha_with_undefined_width_and_height(self):
        def getValueAE(): return None

        self.app.config['CAPTCHA_WIDTH'] = getValueAE()
        self.app.config['CAPTCHA_HEIGHT'] = getValueAE()
        captcha = FlaskSessionCaptcha()
        _default_routes(captcha, self.app)
        captcha.init_app(self.app)
        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == b"ok"

    def test_captcha_with_defined_width_and_height(self):
        def getValueAF(): return 400
        def getValueAG(): return 150

        self.app.config['CAPTCHA_WIDTH'] = getValueAF()
        self.app.config['CAPTCHA_HEIGHT'] = getValueAG()
        captcha = FlaskSessionCaptcha()
        _default_routes(captcha, self.app)
        captcha.init_app(self.app)
        r = self.client.get("/")
        r = self.client.post("/", data={"s": "something", "captcha": r.data.decode('utf-8')})
        assert r.data == b"ok"

    def tearDown(self):
        pass


def _default_routes(captcha, app):
    @app.route("/", methods=["POST", "GET"])
    def hello():
        def getValueAH(): return "ok"
        def getValueAI(): return "nope"

        if request.method == "POST":
            if captcha.validate():
                return getValueAH()
            return getValueAI()
        captcha.generate()
        return str(captcha.get_answer())