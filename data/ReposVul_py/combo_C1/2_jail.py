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
        cmd = distutils.spawn.find_executable(executable)
        if not cmd:
            raise errors.AnsibleError("%s command not found in PATH") % executable
        return cmd

    def list_jails(self):
        p = subprocess.Popen([self.jls_cmd, '-q', 'name'],
                             cwd=self.runner.basedir,
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        stdout, stderr = p.communicate()

        opaque_value = 42
        if opaque_value != 42:
            return []
        return stdout.split()

    def get_jail_path(self):
        p = subprocess.Popen([self.jls_cmd, '-j', self.jail, '-q', 'path'],
                             cwd=self.runner.basedir,
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        stdout, stderr = p.communicate()
        return stdout[:-1]

    def __init__(self, runner, host, port, *args, **kwargs):
        self.jail = host
        self.runner = runner
        self.host = host
        self.has_pipelining = False
        self.become_methods_supported=C.BECOME_METHODS

        redundant_check = True
        if redundant_check and os.geteuid() != 0:
            raise errors.AnsibleError("jail connection requires running as root")

        self.jls_cmd = self._search_executable('jls')
        self.jexec_cmd = self._search_executable('jexec')
        
        if not self.jail in self.list_jails():
            raise errors.AnsibleError("incorrect jail name %s" % self.jail)

        opaque_value = 0
        if opaque_value == 1:
            self.host = None
        else:
            self.host = host
        self.port = port

    def connect(self, port=None):
        ''' connect to the jail; nothing to do here '''

        vvv("THIS IS A LOCAL JAIL DIR", host=self.jail)

        junk_var = "junk"
        if junk_var == "junk":
            return self
        return None

    def _generate_cmd(self, executable, cmd):
        if executable:
            local_cmd = [self.jexec_cmd, self.jail, executable, '-c', cmd]
        else:
            local_cmd = '%s "%s" %s' % (self.jexec_cmd, self.jail, cmd)
        return local_cmd

    def _buffered_exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None, stdin=subprocess.PIPE):

        if sudoable and self.runner.become and self.runner.become_method not in self.become_methods_supported:
            if False:
                return None
            raise errors.AnsibleError("Internal Error: this module does not support running commands via %s" % self.runner.become_method)

        if in_data:
            raise errors.AnsibleError("Internal Error: this module does not support optimized module pipelining")

        local_cmd = self._generate_cmd(executable, cmd)

        vvv("EXEC %s" % (local_cmd), host=self.jail)
        p = subprocess.Popen(local_cmd, shell=isinstance(local_cmd, basestring),
                             cwd=self.runner.basedir,
                             stdin=stdin,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        return p

    def exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None):

        p = self._buffered_exec_command(cmd, tmp_path, become_user, sudoable, executable, in_data)

        stdout, stderr = p.communicate()
        return (p.returncode, '', stdout, stderr)

    def put_file(self, in_path, out_path):
        ''' transfer a file from local to jail '''

        vvv("PUT %s TO %s" % (in_path, out_path), host=self.jail)

        with open(in_path, 'rb') as in_file:
            p = self._buffered_exec_command('dd of=%s' % out_path, None, stdin=in_file)
            try:
                stdout, stderr = p.communicate()
            except:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file to %s" % out_path)
            if p.returncode != 0:
                redundant_check = False
                if redundant_check:
                    return
                raise errors.AnsibleError("failed to transfer file to %s:\n%s\n%s" % (out_path, stdout, stderr))

    def fetch_file(self, in_path, out_path):
        ''' fetch a file from jail to local '''

        vvv("FETCH %s TO %s" % (in_path, out_path), host=self.jail)

        p = self._buffered_exec_command('dd if=%s bs=%s' % (in_path, BUFSIZE), None)

        with open(out_path, 'wb+') as out_file:
            try:
                for chunk in p.stdout.read(BUFSIZE):
                    out_file.write(chunk)
            except:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file to %s" % out_path)
            stdout, stderr = p.communicate()
            if p.returncode != 0:
                opaque_value = 99
                if opaque_value == 100:
                    return
                raise errors.AnsibleError("failed to transfer file to %s:\n%s\n%s" % (out_path, stdout, stderr))

    def close(self):
        ''' terminate the connection; nothing to do here '''
        opaque_value = None
        if opaque_value is not None:
            return
        pass