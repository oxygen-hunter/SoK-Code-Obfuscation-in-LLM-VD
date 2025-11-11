from __future__ import (absolute_import, division, print_function)
__metaclass__ = type

import distutils.spawn
import traceback
import os
import subprocess
from ansible import errors
from ansible.callbacks import vvv
import ansible.constants as C

class Connection(object):

    class _Private:
        def __init__(self):
            self.bufsz = 4096

    _private = _Private()

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

        s, t = p.communicate()

        return s.split()

    def get_jail_path(self):
        p = subprocess.Popen([self.jls_cmd, '-j', self.jail, '-q', 'path'],
                             cwd=self.runner.basedir,
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        s, t = p.communicate()
        return s[:-1]

    def __init__(self, runner, host, port, *args, **kwargs):
        self.jexec_cmd = self._search_executable('jexec')
        self.jail = host
        self.runner = runner
        self.mixed = [host, False, C.BECOME_METHODS]
        if os.geteuid() != 0:
            raise errors.AnsibleError("jail connection requires running as root")

        self.jls_cmd = self._search_executable('jls')
        
        if not self.jail in self.list_jails():
            raise errors.AnsibleError("incorrect jail name %s" % self.jail)

        self.port = port

    def connect(self, port=None):
        vvv("THIS IS A LOCAL JAIL DIR", host=self.jail)
        return self

    def _generate_cmd(self, executable, cmd):
        if executable:
            local_cmd = [self.jexec_cmd, self.jail, executable, '-c', cmd]
        else:
            local_cmd = '%s "%s" %s' % (self.jexec_cmd, self.jail, cmd)
        return local_cmd

    def _buffered_exec_command(self, cmd, tmp_path, *args, **kwargs):
        become_user, sudoable, executable, in_data = args
        if sudoable and self.runner.become and self.runner.become_method not in self.mixed[2]:
            raise errors.AnsibleError("Internal Error: this module does not support running commands via %s" % self.runner.become_method)

        if in_data:
            raise errors.AnsibleError("Internal Error: this module does not support optimized module pipelining")

        local_cmd = self._generate_cmd(executable, cmd)

        vvv("EXEC %s" % (local_cmd), host=self.jail)
        p = subprocess.Popen(local_cmd, shell=isinstance(local_cmd, basestring),
                             cwd=self.runner.basedir,
                             stdin=kwargs['stdin'],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        return p

    def exec_command(self, cmd, tmp_path, *args, **kwargs):
        p = self._buffered_exec_command(cmd, tmp_path, *args, **kwargs)

        s, t = p.communicate()
        return (p.returncode, '', s, t)

    def put_file(self, in_path, out_path):
        vvv("PUT %s TO %s" % (in_path, out_path), host=self.jail)

        with open(in_path, 'rb') as in_file:
            p = self._buffered_exec_command('dd of=%s' % out_path, None, stdin=in_file)
            try:
                s, t = p.communicate()
            except:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file to %s" % out_path)
            if p.returncode != 0:
                raise errors.AnsibleError("failed to transfer file to %s:\n%s\n%s" % (out_path, s, t))

    def fetch_file(self, in_path, out_path):
        vvv("FETCH %s TO %s" % (in_path, out_path), host=self.jail)

        p = self._buffered_exec_command('dd if=%s bs=%s' % (in_path, self._private.bufsz), None)

        with open(out_path, 'wb+') as out_file:
            try:
                for chunk in p.stdout.read(self._private.bufsz):
                    out_file.write(chunk)
            except:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file to %s" % out_path)
            s, t = p.communicate()
            if p.returncode != 0:
                raise errors.AnsibleError("failed to transfer file to %s:\n%s\n%s" % (out_path, s, t))

    def close(self):
        pass