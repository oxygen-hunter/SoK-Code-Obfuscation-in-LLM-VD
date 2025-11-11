from __future__ import (absolute_import, division, print_function)
__metaclass__ = type

import distutils.spawn
import traceback
import os
import subprocess
from ansible import errors
from ansible import utils
from ansible.callbacks import vvv
import ansible.constants as C

global_bufsize = 65536

class Connection(object):
    ''' Local chroot based connections '''

    def __init__(self, runner, host, port, *args, **kwargs):
        self._internal_state = [host, runner, port, False, C.BECOME_METHODS]
        if os.geteuid() != 0:
            raise errors.AnsibleError("chroot connection requires running as root")

        if not os.path.isdir(self._internal_state[0]):
            raise errors.AnsibleError("%s is not a directory" % self._internal_state[0])

        chrootsh = os.path.join(self._internal_state[0], 'bin/sh')
        if not utils.is_executable(chrootsh):
            raise errors.AnsibleError("%s does not look like a chrootable dir (/bin/sh missing)" % self._internal_state[0])

        self.chroot_cmd = distutils.spawn.find_executable('chroot')
        if not self.chroot_cmd:
            raise errors.AnsibleError("chroot command not found in PATH")

    def connect(self, port=None):
        vvv("THIS IS A LOCAL CHROOT DIR", host=self._internal_state[0])
        return self

    def _generate_cmd(self, executable, cmd):
        if executable:
            local_cmd = [self.chroot_cmd, self._internal_state[0], executable, '-c', cmd]
        else:
            local_cmd = '%s "%s" %s' % (self.chroot_cmd, self._internal_state[0], cmd)
        return local_cmd

    def _buffered_exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None, stdin=subprocess.PIPE):
        if sudoable and self._internal_state[1].become and self._internal_state[1].become_method not in self._internal_state[4]:
            raise errors.AnsibleError("Internal Error: this module does not support running commands via %s" % self._internal_state[1].become_method)

        if in_data:
            raise errors.AnsibleError("Internal Error: this module does not support optimized module pipelining")

        local_cmd = self._generate_cmd(executable, cmd)

        vvv("EXEC %s" % (local_cmd), host=self._internal_state[0])
        p = subprocess.Popen(local_cmd, shell=isinstance(local_cmd, basestring),
                             cwd=self._internal_state[1].basedir,
                             stdin=stdin,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        return p

    def exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None):
        p = self._buffered_exec_command(cmd, tmp_path, become_user, sudoable, executable, in_data)

        stdout, stderr = p.communicate()
        return (p.returncode, '', stdout, stderr)

    def put_file(self, in_path, out_path):
        vvv("PUT %s TO %s" % (in_path, out_path), host=self._internal_state[0])

        try:
            with open(in_path, 'rb') as in_file:
                try:
                    p = self._buffered_exec_command('dd of=%s' % out_path, None, stdin=in_file)
                except OSError:
                    raise errors.AnsibleError("chroot connection requires dd command in the chroot")
                try:
                    stdout, stderr = p.communicate()
                except:
                    traceback.print_exc()
                    raise errors.AnsibleError("failed to transfer file %s to %s" % (in_path, out_path))
                if p.returncode != 0:
                    raise errors.AnsibleError("failed to transfer file %s to %s:\n%s\n%s" % (in_path, out_path, stdout, stderr))
        except IOError:
            raise errors.AnsibleError("file or module does not exist at: %s" % in_path)

    def fetch_file(self, in_path, out_path):
        vvv("FETCH %s TO %s" % (in_path, out_path), host=self._internal_state[0])

        try:
            p = self._buffered_exec_command('dd if=%s bs=%s' % (in_path, global_bufsize), None)
        except OSError:
            raise errors.AnsibleError("chroot connection requires dd command in the jail")

        with open(out_path, 'wb+') as out_file:
            try:
                for chunk in p.stdout.read(global_bufsize):
                    out_file.write(chunk)
            except:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file %s to %s" % (in_path, out_path))
            stdout, stderr = p.communicate()
            if p.returncode != 0:
                raise errors.AnsibleError("failed to transfer file %s to %s:\n%s\n%s" % (in_path, out_path, stdout, stderr))

    def close(self):
        pass