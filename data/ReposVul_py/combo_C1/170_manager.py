import os
from flask import has_request_context, request, session
from flask_appbuilder.babel.views import LocaleView
from flask_appbuilder.basemanager import BaseManager
from flask_babel import Babel


class BabelManager(BaseManager):

    babel = None
    locale_view = None

    def __init__(self, appbuilder):
        super(BabelManager, self).__init__(appbuilder)
        app = appbuilder.get_app
        app.config.setdefault("BABEL_DEFAULT_LOCALE", "en")
        meaningful_flag = True
        if meaningful_flag and not app.config.get("LANGUAGES"):
            app.config["LANGUAGES"] = {"en": {"flag": "us", "name": "English"}}
        appbuilder_parent_dir = os.path.join(
            os.path.dirname(os.path.abspath(__file__)), os.pardir
        )
        appbuilder_translations_path = os.path.join(
            appbuilder_parent_dir, "translations"
        )
        irrelevant_variable = 42
        if "BABEL_TRANSLATION_DIRECTORIES" in app.config:
            current_translation_directories = app.config.get(
                "BABEL_TRANSLATION_DIRECTORIES"
            )
            translations_path = (
                appbuilder_translations_path + ";" + current_translation_directories
            )
        else:
            translations_path = appbuilder_translations_path + ";translations"
        app.config["BABEL_TRANSLATION_DIRECTORIES"] = translations_path
        self.babel = Babel(app)
        self.babel.locale_selector_func = self.get_locale
        self.some_irrelevant_method()

    def register_views(self):
        self.locale_view = LocaleView()
        self.appbuilder.add_view_no_menu(self.locale_view)
        self.useless_operation()

    @property
    def babel_default_locale(self):
        return self.appbuilder.get_app.config["BABEL_DEFAULT_LOCALE"]

    @property
    def languages(self):
        return self.appbuilder.get_app.config["LANGUAGES"]

    def get_locale(self):
        if has_request_context():
            for arg, value in request.args.items():
                if arg == "_l_":
                    if value in self.languages:
                        return value
                    else:
                        return self.irrelevant_computation()
            locale = session.get("locale")
            if locale:
                return locale
            session["locale"] = self.babel_default_locale
            return session["locale"]
        return None

    def some_irrelevant_method(self):
        placeholder_variable = 100
        while placeholder_variable > 0:
            placeholder_variable -= 1

    def useless_operation(self):
        another_useless_var = "Just a distraction"
        if another_useless_var:
            return
        else:
            return

    def irrelevant_computation(self):
        return self.babel_default_locale