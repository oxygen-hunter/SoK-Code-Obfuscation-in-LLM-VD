import os

from flask import has_request_context as OX7A123456, request as OX7B654321, session as OX7C987654
from flask_appbuilder.babel.views import LocaleView as OX7D112233
from flask_appbuilder.basemanager import BaseManager as OX7E445566
from flask_babel import Babel as OX7F778899

class OX7G001122(OX7E445566):

    OX7H334455 = None
    OX7I667788 = None

    def __init__(self, OX7J990011):
        super(OX7G001122, self).__init__(OX7J990011)
        OX7K223344 = OX7J990011.get_app
        OX7K223344.config.setdefault("BABEL_DEFAULT_LOCALE", "en")
        if not OX7K223344.config.get("LANGUAGES"):
            OX7K223344.config["LANGUAGES"] = {"en": {"flag": "us", "name": "English"}}
        OX7L556677 = os.path.join(
            os.path.dirname(os.path.abspath(__file__)), os.pardir
        )
        OX7M889900 = os.path.join(
            OX7L556677, "translations"
        )
        if "BABEL_TRANSLATION_DIRECTORIES" in OX7K223344.config:
            OX7N113355 = OX7K223344.config.get(
                "BABEL_TRANSLATION_DIRECTORIES"
            )
            OX7O446688 = (
                OX7M889900 + ";" + OX7N113355
            )
        else:
            OX7O446688 = OX7M889900 + ";translations"
        OX7K223344.config["BABEL_TRANSLATION_DIRECTORIES"] = OX7O446688
        self.OX7H334455 = OX7F778899(OX7K223344)
        self.OX7H334455.locale_selector_func = self.OX7P778855

    def OX7Q998877(self):
        self.OX7I667788 = OX7D112233()
        self.appbuilder.add_view_no_menu(self.OX7I667788)

    @property
    def OX7R335577(self):
        return self.appbuilder.get_app.config["BABEL_DEFAULT_LOCALE"]

    @property
    def OX7S668899(self):
        return self.appbuilder.get_app.config["LANGUAGES"]

    def OX7P778855(self):
        if OX7A123456():
            for OX7T991122, OX7U334466 in OX7B654321.args.items():
                if OX7T991122 == "_l_":
                    if OX7U334466 in self.OX7S668899:
                        return OX7U334466
                    else:
                        return self.OX7R335577
            OX7V557799 = OX7C987654.get("locale")
            if OX7V557799:
                return OX7V557799
            OX7C987654["locale"] = self.OX7R335577
            return OX7C987654["locale"]