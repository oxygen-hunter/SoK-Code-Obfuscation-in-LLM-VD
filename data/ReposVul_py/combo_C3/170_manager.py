import os
from flask import has_request_context, request, session
from flask_appbuilder.babel.views import LocaleView
from flask_appbuilder.basemanager import BaseManager
from flask_babel import Babel

class StackVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.pc += 1
            getattr(self, f'op_{instr[0]}')(*instr[1:])

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(b - a)

    def op_JMP(self, target):
        self.pc = target

    def op_JZ(self, target):
        if self.stack.pop() == 0:
            self.pc = target

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.stack.pop()

class BabelManager(BaseManager):
    babel = None
    locale_view = None

    def __init__(self, appbuilder):
        super(BabelManager, self).__init__(appbuilder)
        self.vm = StackVM()
        self.setup_vm()
        self.vm.run()

    def setup_vm(self):
        # Translated logic into bytecode-like instructions
        self.vm.instructions = [
            ('PUSH', self.appbuilder.get_app),
            ('LOAD', 0),
            ('PUSH', "BABEL_DEFAULT_LOCALE"),
            ('PUSH', "en"),
            ('STORE', 1),
            ('LOAD', 0),
            ('PUSH', "LANGUAGES"),
            ('PUSH', None),
            ('STORE', 2),
            ('LOAD', 2),
            ('PUSH', None),
            ('JZ', 11),
            ('LOAD', 0),
            ('PUSH', "LANGUAGES"),
            ('PUSH', {"en": {"flag": "us", "name": "English"}}),
            ('STORE', 3),
            ('PUSH', os.path.join),
            ('PUSH', os.path.dirname),
            ('PUSH', os.path.abspath),
            ('PUSH', __file__),
            ('CALL', 1),
            ('CALL', 1),
            ('PUSH', os.pardir),
            ('CALL', 2),
            ('STORE', 4),
            ('PUSH', os.path.join),
            ('LOAD', 4),
            ('PUSH', "translations"),
            ('CALL', 2),
            ('STORE', 5),
            ('LOAD', 0),
            ('PUSH', "BABEL_TRANSLATION_DIRECTORIES"),
            ('STORE', 6),
            ('LOAD', 6),
            ('PUSH', None),
            ('JZ', 23),
            ('LOAD', 0),
            ('PUSH', "BABEL_TRANSLATION_DIRECTORIES"),
            ('LOAD', 5),
            ('PUSH', ";"),
            ('LOAD', 6),
            ('PUSH', 3),
            ('ADD',),
            ('STORE', 7),
            ('JMP', 25),
            ('LOAD', 5),
            ('PUSH', ";translations"),
            ('PUSH', 2),
            ('ADD',),
            ('STORE', 7),
            ('LOAD', 0),
            ('PUSH', "BABEL_TRANSLATION_DIRECTORIES"),
            ('LOAD', 7),
            ('STORE', 8),
            ('PUSH', Babel),
            ('LOAD', 0),
            ('CALL', 1),
            ('STORE', 9),
            ('PUSH', self.get_locale),
            ('STORE', 10),
        ]

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
        if has_request_context():
            for arg, value in request.args.items():
                if arg == "_l_":
                    if value in self.languages:
                        return value
                    else:
                        return self.babel_default_locale
            locale = session.get("locale")
            if locale:
                return locale
            session["locale"] = self.babel_default_locale
            return session["locale"]