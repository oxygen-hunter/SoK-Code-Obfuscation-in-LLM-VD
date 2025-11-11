from __future__ import (absolute_import, division, print_function)
__metaclass__ = type

import distutils.spawn
import traceback
import os
import subprocess
from ansible import errors
from ansible.callbacks import vvv
import ansible.constants as C

BUFSIZE = 4096

class Connection(object):
    ''' Local BSD Jail based connections '''

    def _search_executable(self, executable):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                cmd = distutils.spawn.find_executable(executable)
                dispatcher = 1
            elif dispatcher == 1:
                if not cmd:
                    dispatcher = 2
                else:
                    dispatcher = 3
            elif dispatcher == 2:
                raise errors.AnsibleError("%s command not found in PATH") % executable
            elif dispatcher == 3:
                return cmd

    def list_jails(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                p = subprocess.Popen([self.jls_cmd, '-q', 'name'],
                                     cwd=self.runner.basedir,
                                     stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                dispatcher = 1
            elif dispatcher == 1:
                stdout, stderr = p.communicate()
                dispatcher = 2
            elif dispatcher == 2:
                return stdout.split()

    def get_jail_path(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                p = subprocess.Popen([self.jls_cmd, '-j', self.jail, '-q', 'path'],
                                     cwd=self.runner.basedir,
                                     stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                dispatcher = 1
            elif dispatcher == 1:
                stdout, stderr = p.communicate()
                dispatcher = 2
            elif dispatcher == 2:
                return stdout[:-1]

    def __init__(self, runner, host, port, *args, **kwargs):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.jail = host
                self.runner = runner
                self.host = host
                self.has_pipelining = False
                self.become_methods_supported=C.BECOME_METHODS
                dispatcher = 1
            elif dispatcher == 1:
                if os.geteuid() != 0:
                    dispatcher = 2
                else:
                    dispatcher = 3
            elif dispatcher == 2:
                raise errors.AnsibleError("jail connection requires running as root")
            elif dispatcher == 3:
                self.jls_cmd = self._search_executable('jls')
                self.jexec_cmd = self._search_executable('jexec')
                dispatcher = 4
            elif dispatcher == 4:
                if not self.jail in self.list_jails():
                    dispatcher = 5
                else:
                    dispatcher = 6
            elif dispatcher == 5:
                raise errors.AnsibleError("incorrect jail name %s" % self.jail)
            elif dispatcher == 6:
                self.host = host
                self.port = port
                return

    def connect(self, port=None):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                vvv("THIS IS A LOCAL JAIL DIR", host=self.jail)
                dispatcher = 1
            elif dispatcher == 1:
                return self

    def _generate_cmd(self, executable, cmd):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if executable:
                    dispatcher = 1
                else:
                    dispatcher = 2
            elif dispatcher == 1:
                local_cmd = [self.jexec_cmd, self.jail, executable, '-c', cmd]
                dispatcher = 3
            elif dispatcher == 2:
                local_cmd = '%s "%s" %s' % (self.jexec_cmd, self.jail, cmd)
                dispatcher = 3
            elif dispatcher == 3:
                return local_cmd

    def _buffered_exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None, stdin=subprocess.PIPE):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if sudoable and self.runner.become and self.runner.become_method not in self.become_methods_supported:
                    dispatcher = 1
                else:
                    dispatcher = 2
            elif dispatcher == 1:
                raise errors.AnsibleError("Internal Error: this module does not support running commands via %s" % self.runner.become_method)
            elif dispatcher == 2:
                if in_data:
                    dispatcher = 3
                else:
                    dispatcher = 4
            elif dispatcher == 3:
                raise errors.AnsibleError("Internal Error: this module does not support optimized module pipelining")
            elif dispatcher == 4:
                local_cmd = self._generate_cmd(executable, cmd)
                dispatcher = 5
            elif dispatcher == 5:
                vvv("EXEC %s" % (local_cmd), host=self.jail)
                dispatcher = 6
            elif dispatcher == 6:
                p = subprocess.Popen(local_cmd, shell=isinstance(local_cmd, basestring),
                                     cwd=self.runner.basedir,
                                     stdin=stdin,
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                dispatcher = 7
            elif dispatcher == 7:
                return p

    def exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                p = self._buffered_exec_command(cmd, tmp_path, become_user, sudoable, executable, in_data)
                dispatcher = 1
            elif dispatcher == 1:
                stdout, stderr = p.communicate()
                dispatcher = 2
            elif dispatcher == 2:
                return (p.returncode, '', stdout, stderr)

    def put_file(self, in_path, out_path):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                vvv("PUT %s TO %s" % (in_path, out_path), host=self.jail)
                dispatcher = 1
            elif dispatcher == 1:
                with open(in_path, 'rb') as in_file:
                    dispatcher = 2
            elif dispatcher == 2:
                p = self._buffered_exec_command('dd of=%s' % out_path, None, stdin=in_file)
                dispatcher = 3
            elif dispatcher == 3:
                try:
                    stdout, stderr = p.communicate()
                    dispatcher = 6
                except:
                    dispatcher = 4
            elif dispatcher == 4:
                traceback.print_exc()
                dispatcher = 5
            elif dispatcher == 5:
                raise errors.AnsibleError("failed to transfer file to %s" % out_path)
            elif dispatcher == 6:
                if p.returncode != 0:
                    dispatcher = 7
                else:
                    return
            elif dispatcher == 7:
                raise errors.AnsibleError("failed to transfer file to %s:\n%s\n%s" % (out_path, stdout, stderr))

    def fetch_file(self, in_path, out_path):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                vvv("FETCH %s TO %s" % (in_path, out_path), host=self.jail)
                dispatcher = 1
            elif dispatcher == 1:
                p = self._buffered_exec_command('dd if=%s bs=%s' % (in_path, BUFSIZE), None)
                dispatcher = 2
            elif dispatcher == 2:
                with open(out_path, 'wb+') as out_file:
                    dispatcher = 3
            elif dispatcher == 3:
                try:
                    for chunk in p.stdout.read(BUFSIZE):
                        out_file.write(chunk)
                    dispatcher = 6
                except:
                    dispatcher = 4
            elif dispatcher == 4:
                traceback.print_exc()
                dispatcher = 5
            elif dispatcher == 5:
                raise errors.AnsibleError("failed to transfer file to %s" % out_path)
            elif dispatcher == 6:
                stdout, stderr = p.communicate()
                dispatcher = 7
            elif dispatcher == 7:
                if p.returncode != 0:
                    dispatcher = 8
                else:
                    return
            elif dispatcher == 8:
                raise errors.AnsibleError("failed to transfer file to %s:\n%s\n%s" % (out_path, stdout, stderr))

    def close(self):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                return