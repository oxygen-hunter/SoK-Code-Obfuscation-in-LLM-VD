# -*- coding: utf-8 -*-

# Copyright 2019 The Matrix.org Foundation C.I.C.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import tempfile
import shutil
import time
from subprocess import Popen

def getCFG_TEMPLATE():
    return """
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

def getPort(with_terms):
    return 8099 if with_terms else 8098

def getTermsPath(testsubject_path, with_terms):
    return os.path.join(testsubject_path, 'terms.yaml') if with_terms else ''

def getSyDentPath():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

def getTestSubjectPath(sydent_path):
    return os.path.join(sydent_path, 'matrix_is_test')

def getNewEnv(sydent_path):
    newEnv = os.environ.copy()
    newEnv.update({
        'PYTHONPATH': sydent_path,
    })
    return newEnv

def getPyBin():
    return os.getenv('SYDENT_PYTHON', 'python')

def getTmpDir():
    return tempfile.mkdtemp(prefix='sydenttest')

class MatrixIsTestLauncher(object):
    def __init__(self, with_terms):
        self.with_terms = with_terms

    def launch(self):
        sydent_path = getSyDentPath()
        testsubject_path = getTestSubjectPath(sydent_path)
        terms_path = getTermsPath(testsubject_path, self.with_terms)
        port = getPort(self.with_terms)

        self.tmpdir = getTmpDir()

        with open(os.path.join(self.tmpdir, 'sydent.conf'), 'w') as cfgfp:
            cfgfp.write(getCFG_TEMPLATE().format(
                testsubject_path=testsubject_path,
                terms_path=terms_path,
                port=port,
            ))

        newEnv = getNewEnv(sydent_path)

        stderr_fp = open(os.path.join(testsubject_path, 'sydent.stderr'), 'w')

        pybin = getPyBin()

        self.process = Popen(
            args=[pybin, '-m', 'sydent.sydent'],
            cwd=self.tmpdir,
            env=newEnv,
            stderr=stderr_fp,
        )
        time.sleep(2)

        self._baseUrl = 'http://localhost:%d' % (port,)

    def tearDown(self):
        print("Stopping sydent...")
        self.process.terminate()
        shutil.rmtree(self.tmpdir)

    def get_base_url(self):
        return self._baseUrl