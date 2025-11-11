import os
import tempfile
import shutil
import time
from subprocess import Popen

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.env = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.pc += 1
            op = instr[0]
            args = instr[1:]

            if op == 'PUSH':
                self.stack.append(args[0])
            elif op == 'POP':
                self.stack.pop()
            elif op == 'ADD':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif op == 'SUB':
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif op == 'JMP':
                self.pc = args[0]
            elif op == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = args[0]
            elif op == 'LOAD':
                self.stack.append(self.env[args[0]])
            elif op == 'STORE':
                self.env[args[0]] = self.stack.pop()

CFG_TEMPLATE = """
[http]
clientapi.http.bind_address = localhost
clientapi.http.port = {port}
client_http_base = http://localhost:{port}
federation.verifycerts = False

[db]
db.file = :memory:

[general]
server.name = test.local
terms.path = {terms_path}
templates.path = {testsubject_path}/res
brand.default = is-test

ip.whitelist = 127.0.0.1

[email]
email.tlsmode = 0
email.invite.subject = %(sender_display_name)s has invited you to chat
email.smtphost = localhost
email.from = Sydent Validation <noreply@localhost>
email.smtpport = 9925
email.subject = Your Validation Token
"""

class MatrixIsTestLauncher(object):
    def __init__(self, with_terms):
        self.with_terms = with_terms

    def launch(self):
        sydent_path = os.path.abspath(os.path.join(
            os.path.dirname(__file__), '..',
        ))
        testsubject_path = os.path.join(
            sydent_path, 'matrix_is_test',
        )
        terms_path = os.path.join(testsubject_path, 'terms.yaml') if self.with_terms else ''
        port = 8099 if self.with_terms else 8098

        self.tmpdir = tempfile.mkdtemp(prefix='sydenttest')

        vm = VM()
        vm.load_program([
            ('PUSH', sydent_path),
            ('STORE', 'sydent_path'),
            ('PUSH', testsubject_path),
            ('STORE', 'testsubject_path'),
            ('PUSH', terms_path),
            ('STORE', 'terms_path'),
            ('PUSH', port),
            ('STORE', 'port'),
            ('PUSH', self.tmpdir),
            ('PUSH', 'sydent.conf'),
            ('ADD',),
            ('STORE', 'config_path'),
            ('LOAD', 'config_path'),
            ('PUSH', 'w'),
            ('PUSH', CFG_TEMPLATE),
            ('LOAD', 'testsubject_path'),
            ('LOAD', 'terms_path'),
            ('LOAD', 'port'),
            ('ADD',),
            ('ADD',),
            ('ADD',),
            ('STORE', 'config_content'),
            ('PUSH', os.environ.copy()),
            ('STORE', 'newEnv'),
            ('LOAD', 'newEnv'),
            ('PUSH', 'PYTHONPATH'),
            ('LOAD', 'sydent_path'),
            ('ADD',),
            ('STORE', 'newEnv'),
            ('PUSH', testsubject_path),
            ('PUSH', 'sydent.stderr'),
            ('ADD',),
            ('STORE', 'stderr_path'),
            ('PUSH', os.getenv('SYDENT_PYTHON', 'python')),
            ('STORE', 'pybin'),
            ('LOAD', 'pybin'),
            ('PUSH', '-m'),
            ('PUSH', 'sydent.sydent'),
            ('ADD',),
            ('PUSH', self.tmpdir),
            ('LOAD', 'newEnv'),
            ('LOAD', 'stderr_path'),
            ('ADD',),
            ('STORE', 'process_args'),
            ('LOAD', 'process_args'),
            ('STORE', 'self.process'),
            ('PUSH', 2),
            ('STORE', 'sleep_time'),
            ('LOAD', 'sleep_time'),
            ('JMP', 50)
        ])
        vm.run()

        self._baseUrl = 'http://localhost:%d' % (port,)

    def tearDown(self):
        print("Stopping sydent...")
        self.process.terminate()
        shutil.rmtree(self.tmpdir)

    def get_base_url(self):
        return self._baseUrl