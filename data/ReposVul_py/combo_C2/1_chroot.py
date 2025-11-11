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

BUFSIZE = 65536

class Connection(object):
    def __init__(self, runner, host, port, *args, **kwargs):
        state = 0
        while True:
            if state == 0:
                self.chroot = host
                self.has_pipelining = False
                self.become_methods_supported=C.BECOME_METHODS
                state = 1
            elif state == 1:
                if os.geteuid() != 0:
                    state = 2
                else:
                    state = 3
            elif state == 2:
                raise errors.AnsibleError("chroot connection requires running as root")
            elif state == 3:
                if not os.path.isdir(self.chroot):
                    state = 4
                else:
                    state = 5
            elif state == 4:
                raise errors.AnsibleError("%s is not a directory" % self.chroot)
            elif state == 5:
                chrootsh = os.path.join(self.chroot, 'bin/sh')
                if not utils.is_executable(chrootsh):
                    state = 6
                else:
                    state = 7
            elif state == 6:
                raise errors.AnsibleError("%s does not look like a chrootable dir (/bin/sh missing)" % self.chroot)
            elif state == 7:
                self.chroot_cmd = distutils.spawn.find_executable('chroot')
                if not self.chroot_cmd:
                    state = 8
                else:
                    state = 9
            elif state == 8:
                raise errors.AnsibleError("chroot command not found in PATH")
            elif state == 9:
                self.runner = runner
                self.host = host
                self.port = port
                break

    def connect(self, port=None):
        state = 0
        while True:
            if state == 0:
                vvv("THIS IS A LOCAL CHROOT DIR", host=self.chroot)
                state = 1
            elif state == 1:
                return self

    def _generate_cmd(self, executable, cmd):
        state = 0
        while True:
            if state == 0:
                if executable:
                    local_cmd = [self.chroot_cmd, self.chroot, executable, '-c', cmd]
                    state = 1
                else:
                    local_cmd = '%s "%s" %s' % (self.chroot_cmd, self.chroot, cmd)
                    state = 1
            elif state == 1:
                return local_cmd

    def _buffered_exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None, stdin=subprocess.PIPE):
        state = 0
        while True:
            if state == 0:
                if sudoable and self.runner.become and self.runner.become_method not in self.become_methods_supported:
                    state = 1
                else:
                    state = 2
            elif state == 1:
                raise errors.AnsibleError("Internal Error: this module does not support running commands via %s" % self.runner.become_method)
            elif state == 2:
                if in_data:
                    state = 3
                else:
                    state = 4
            elif state == 3:
                raise errors.AnsibleError("Internal Error: this module does not support optimized module pipelining")
            elif state == 4:
                local_cmd = self._generate_cmd(executable, cmd)
                state = 5
            elif state == 5:
                vvv("EXEC %s" % (local_cmd), host=self.chroot)
                p = subprocess.Popen(local_cmd, shell=isinstance(local_cmd, basestring),
                                     cwd=self.runner.basedir,
                                     stdin=stdin,
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                state = 6
            elif state == 6:
                return p

    def exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None):
        state = 0
        while True:
            if state == 0:
                p = self._buffered_exec_command(cmd, tmp_path, become_user, sudoable, executable, in_data)
                state = 1
            elif state == 1:
                stdout, stderr = p.communicate()
                state = 2
            elif state == 2:
                return (p.returncode, '', stdout, stderr)

    def put_file(self, in_path, out_path):
        state = 0
        while True:
            if state == 0:
                vvv("PUT %s TO %s" % (in_path, out_path), host=self.chroot)
                state = 1
            elif state == 1:
                try:
                    with open(in_path, 'rb') as in_file:
                        state = 2
                except IOError:
                    state = 8
            elif state == 2:
                try:
                    p = self._buffered_exec_command('dd of=%s' % out_path, None, stdin=in_file)
                    state = 4
                except OSError:
                    state = 3
            elif state == 3:
                raise errors.AnsibleError("chroot connection requires dd command in the chroot")
            elif state == 4:
                try:
                    stdout, stderr = p.communicate()
                    state = 5
                except:
                    state = 6
            elif state == 5:
                if p.returncode != 0:
                    state = 7
                else:
                    break
            elif state == 6:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file %s to %s" % (in_path, out_path))
            elif state == 7:
                raise errors.AnsibleError("failed to transfer file %s to %s:\n%s\n%s" % (in_path, out_path, stdout, stderr))
            elif state == 8:
                raise errors.AnsibleError("file or module does not exist at: %s" % in_path)

    def fetch_file(self, in_path, out_path):
        state = 0
        while True:
            if state == 0:
                vvv("FETCH %s TO %s" % (in_path, out_path), host=self.chroot)
                state = 1
            elif state == 1:
                try:
                    p = self._buffered_exec_command('dd if=%s bs=%s' % (in_path, BUFSIZE), None)
                    state = 3
                except OSError:
                    state = 2
            elif state == 2:
                raise errors.AnsibleError("chroot connection requires dd command in the jail")
            elif state == 3:
                with open(out_path, 'wb+') as out_file:
                    state = 4
            elif state == 4:
                try:
                    for chunk in p.stdout.read(BUFSIZE):
                        out_file.write(chunk)
                    state = 5
                except:
                    state = 6
            elif state == 5:
                stdout, stderr = p.communicate()
                if p.returncode != 0:
                    state = 7
                else:
                    break
            elif state == 6:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file %s to %s" % (in_path, out_path))
            elif state == 7:
                raise errors.AnsibleError("failed to transfer file %s to %s:\n%s\n%s" % (in_path, out_path, stdout, stderr))

    def close(self):
        state = 0
        while True:
            if state == 0:
                break