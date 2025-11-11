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
        
        # Control flow flattening with dispatcher
        dispatcher = 0
        while dispatcher != -1:
            if dispatcher == 0:
                app = appbuilder.get_app
                app.config.setdefault("BABEL_DEFAULT_LOCALE", "en")
                dispatcher = 1
            elif dispatcher == 1:
                if not app.config.get("LANGUAGES"):
                    app.config["LANGUAGES"] = {"en": {"flag": "us", "name": "English"}}
                appbuilder_parent_dir = os.path.join(
                    os.path.dirname(os.path.abspath(__file__)), os.pardir
                )
                dispatcher = 2
            elif dispatcher == 2:
                appbuilder_translations_path = os.path.join(
                    appbuilder_parent_dir, "translations"
                )
                dispatcher = 3
            elif dispatcher == 3:
                if "BABEL_TRANSLATION_DIRECTORIES" in app.config:
                    current_translation_directories = app.config.get(
                        "BABEL_TRANSLATION_DIRECTORIES"
                    )
                    translations_path = (
                        appbuilder_translations_path + ";" + current_translation_directories
                    )
                else:
                    translations_path = appbuilder_translations_path + ";translations"
                dispatcher = 4
            elif dispatcher == 4:
                app.config["BABEL_TRANSLATION_DIRECTORIES"] = translations_path
                self.babel = Babel(app)
                self.babel.locale_selector_func = self.get_locale
                dispatcher = -1

    def register_views(self):
        self.locale_view = LocaleView()
        self.appbuilder.add_view_no_menu(self.locale_view)

    @property
    def babel_default_locale(self):
        return self.appbuilder.get_app.config["BABEL_DEFAULT_LOCALE"]

    @property
    def languages(self):
        return self.appbuilder.get_app.config["LANGUAGES"]

    def get_locale(self):
        dispatcher = 0
        locale = None
        while dispatcher != -1:
            if dispatcher == 0:
                if has_request_context():
                    dispatcher = 1
                else:
                    return self.babel_default_locale
            elif dispatcher == 1:
                for arg, value in request.args.items():
                    if arg == "_l_":
                        if value in self.languages:
                            return value
                        else:
                            return self.babel_default_locale
                dispatcher = 2
            elif dispatcher == 2:
                locale = session.get("locale")
                if locale:
                    return locale
                session["locale"] = self.babel_default_locale
                return session["locale"]
                dispatcher = -1