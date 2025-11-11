import os

from flask import has_request_context, request, session
from flask_appbuilder.babel.views import LocaleView
from flask_appbuilder.basemanager import BaseManager
from flask_babel import Babel

class BabelManager(BaseManager):

    locale_view = None
    babel = None

    def __init__(self, appbuilder):
        super(BabelManager, self).__init__(appbuilder)
        app = appbuilder.get_app
        app.config.setdefault("BABEL_DEFAULT_LOCALE", "en")
        if not app.config.get("LANGUAGES"):
            app.config["LANGUAGES"] = {"en": {"flag": "us", "name": "English"}}
        
        x0 = os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir)
        x1 = os.path.join(x0, "translations")
        
        if "BABEL_TRANSLATION_DIRECTORIES" in app.config:
            x2 = app.config.get("BABEL_TRANSLATION_DIRECTORIES")
            x3 = x1 + ";" + x2
        else:
            x3 = x1 + ";translations"
        
        app.config["BABEL_TRANSLATION_DIRECTORIES"] = x3
        self.babel = Babel(app)
        self.babel.locale_selector_func = self.get_locale

    def register_views(self):
        self.locale_view = LocaleView()
        self.appbuilder.add_view_no_menu(self.locale_view)

    @property
    def languages(self):
        return self.appbuilder.get_app.config["LANGUAGES"]

    @property
    def babel_default_locale(self):
        return self.appbuilder.get_app.config["BABEL_DEFAULT_LOCALE"]

    def get_locale(self):
        if has_request_context():
            for arg, value in request.args.items():
                if arg == "_l_":
                    if value in self.languages:
                        return value
                    else:
                        return self.babel_default_locale
            x4 = session.get("locale")
            if x4:
                return x4
            session["locale"] = self.babel_default_locale
            return session["locale"]