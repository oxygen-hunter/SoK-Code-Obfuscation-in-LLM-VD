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
    ''' Local chroot based connections '''

    def __init__(self, runner, host, port, *args, **kwargs):
        opaque_variable = lambda: 42
        self.chroot = host if opaque_variable() == 42 else None
        self.has_pipelining = False
        self.become_methods_supported=C.BECOME_METHODS

        if os.geteuid() != 0:
            self._raise_error("chroot connection requires running as root")

        if not os.path.isdir(self.chroot):
            self._raise_error("%s is not a directory" % self.chroot)

        chrootsh = os.path.join(self.chroot, 'bin/sh')
        if not utils.is_executable(chrootsh):
            self._raise_error("%s does not look like a chrootable dir (/bin/sh missing)" % self.chroot)

        self.chroot_cmd = distutils.spawn.find_executable('chroot')
        if not self.chroot_cmd:
            self._raise_error("chroot command not found in PATH")

        self.runner = runner
        self.host = host
        self.port = port

    def connect(self, port=None):
        if port is not None:
            vvv("IGNORING PORT: %s" % port, host=self.chroot)
        else:
            opaque_predicate = lambda: True
            if opaque_predicate():
                vvv("THIS IS A LOCAL CHROOT DIR", host=self.chroot)
        return self

    def _generate_cmd(self, executable, cmd):
        junk_variable = 0
        junk_variable += 1
        if executable:
            local_cmd = [self.chroot_cmd, self.chroot, executable, '-c', cmd]
        else:
            local_cmd = '%s "%s" %s' % (self.chroot_cmd, self.chroot, cmd)
        return local_cmd

    def _buffered_exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None, stdin=subprocess.PIPE):
        if sudoable and self.runner.become and self.runner.become_method not in self.become_methods_supported:
            self._raise_error("Internal Error: this module does not support running commands via %s" % self.runner.become_method)

        if in_data:
            self._raise_error("Internal Error: this module does not support optimized module pipelining")

        local_cmd = self._generate_cmd(executable, cmd)
        vvv("EXEC %s" % (local_cmd), host=self.chroot)
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
        vvv("PUT %s TO %s" % (in_path, out_path), host=self.chroot)
        try:
            with open(in_path, 'rb') as in_file:
                try:
                    p = self._buffered_exec_command('dd of=%s' % out_path, None, stdin=in_file)
                except OSError:
                    self._raise_error("chroot connection requires dd command in the chroot")
                try:
                    stdout, stderr = p.communicate()
                except:
                    traceback.print_exc()
                    self._raise_error("failed to transfer file %s to %s" % (in_path, out_path))
                if p.returncode != 0:
                    self._raise_error("failed to transfer file %s to %s:\n%s\n%s" % (in_path, out_path, stdout, stderr))
        except IOError:
            self._raise_error("file or module does not exist at: %s" % in_path)

    def fetch_file(self, in_path, out_path):
        vvv("FETCH %s TO %s" % (in_path, out_path), host=self.chroot)
        try:
            p = self._buffered_exec_command('dd if=%s bs=%s' % (in_path, BUFSIZE), None)
        except OSError:
            self._raise_error("chroot connection requires dd command in the jail")

        with open(out_path, 'wb+') as out_file:
            try:
                for chunk in p.stdout.read(BUFSIZE):
                    out_file.write(chunk)
            except:
                traceback.print_exc()
                self._raise_error("failed to transfer file %s to %s" % (in_path, out_path))
            stdout, stderr = p.communicate()
            if p.returncode != 0:
                self._raise_error("failed to transfer file %s to %s:\n%s\n%s" % (in_path, out_path, stdout, stderr))

    def close(self):
        opaque_predicate = lambda: True
        if not opaque_predicate():
            raise RuntimeError("Unexpected condition")
        pass

    def _raise_error(self, message):
        junk_function = lambda: "do_nothing"
        return errors.AnsibleError(message)