# -*- coding: utf-8 -*-
import os
import tempfile
import shutil
import time
from subprocess import Popen
import ctypes

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
        self.clib = ctypes.CDLL('./example_c_lib.so')

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

        with open(os.path.join(self.tmpdir, 'sydent.conf'), 'w') as cfgfp:
            cfgfp.write(CFG_TEMPLATE.format(
                testsubject_path=testsubject_path,
                terms_path=terms_path,
                port=port,
            ))

        newEnv = os.environ.copy()
        newEnv.update({
            'PYTHONPATH': sydent_path,
        })

        stderr_fp = open(os.path.join(testsubject_path, 'sydent.stderr'), 'w')

        pybin = os.getenv('SYDENT_PYTHON', 'python')

        self.process = Popen(
            args=[pybin, '-m', 'sydent.sydent'],
            cwd=self.tmpdir,
            env=newEnv,
            stderr=stderr_fp,
        )
        self.clib.startup_wait()
        self._baseUrl = 'http://localhost:%d' % (port,)

    def tearDown(self):
        print("Stopping sydent...")
        self.process.terminate()
        shutil.rmtree(self.tmpdir)

    def get_base_url(self):
        return self._baseUrl
```

```c
// example_c_lib.c
#include <unistd.h>

void startup_wait() {
    sleep(2);
}
```

To compile the C code:
```shell
gcc -shared -o example_c_lib.so -fPIC example_c_lib.c