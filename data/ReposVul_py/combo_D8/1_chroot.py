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

def getBufferSize():
    return 65536

def getChrootCmd():
    return distutils.spawn.find_executable('chroot')

def getRunner(r):
    return r

def getHost(h):
    return h

def getPort(p):
    return p

def getBecomeMethodsSupported():
    return C.BECOME_METHODS

def isChrootDir(directory):
    return os.path.isdir(directory)

def getChrootShPath(directory):
    return os.path.join(directory, 'bin/sh')

def findExecutable(path):
    return utils.is_executable(path)

def getEuid():
    return os.geteuid()

def getBasedir(runner):
    return runner.basedir

def getVVVMessage(message, host):
    return vvv(message, host=host)

class Connection(object):
    ''' Local chroot based connections '''

    def __init__(self, runner, host, port, *args, **kwargs):
        self.chroot = getHost(host)
        self.has_pipelining = False
        self.become_methods_supported = getBecomeMethodsSupported()

        if getEuid() != 0:
            raise errors.AnsibleError("chroot connection requires running as root")

        if not isChrootDir(self.chroot):
            raise errors.AnsibleError("%s is not a directory" % self.chroot)

        chrootsh = getChrootShPath(self.chroot)
        if not findExecutable(chrootsh):
            raise errors.AnsibleError("%s does not look like a chrootable dir (/bin/sh missing)" % self.chroot)

        self.chroot_cmd = getChrootCmd()
        if not self.chroot_cmd:
            raise errors.AnsibleError("chroot command not found in PATH")

        self.runner = getRunner(runner)
        self.host = getHost(host)
        self.port = getPort(port)

    def connect(self, port=None):
        getVVVMessage("THIS IS A LOCAL CHROOT DIR", host=self.chroot)
        return self

    def _generate_cmd(self, executable, cmd):
        if executable:
            local_cmd = [self.chroot_cmd, self.chroot, executable, '-c', cmd]
        else:
            local_cmd = '%s "%s" %s' % (self.chroot_cmd, self.chroot, cmd)
        return local_cmd

    def _buffered_exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None, stdin=subprocess.PIPE):

        if sudoable and self.runner.become and self.runner.become_method not in self.become_methods_supported:
            raise errors.AnsibleError("Internal Error: this module does not support running commands via %s" % self.runner.become_method)

        if in_data:
            raise errors.AnsibleError("Internal Error: this module does not support optimized module pipelining")

        local_cmd = self._generate_cmd(executable, cmd)

        getVVVMessage("EXEC %s" % (local_cmd), host=self.chroot)
        p = subprocess.Popen(local_cmd, shell=isinstance(local_cmd, basestring),
                             cwd=getBasedir(self.runner),
                             stdin=stdin,
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        return p

    def exec_command(self, cmd, tmp_path, become_user=None, sudoable=False, executable='/bin/sh', in_data=None):
        p = self._buffered_exec_command(cmd, tmp_path, become_user, sudoable, executable, in_data)

        stdout, stderr = p.communicate()
        return (p.returncode, '', stdout, stderr)

    def put_file(self, in_path, out_path):
        getVVVMessage("PUT %s TO %s" % (in_path, out_path), host=self.chroot)

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
        getVVVMessage("FETCH %s TO %s" % (in_path, out_path), host=self.chroot)

        try:
            p = self._buffered_exec_command('dd if=%s bs=%s' % (in_path, getBufferSize()), None)
        except OSError:
            raise errors.AnsibleError("chroot connection requires dd command in the jail")

        with open(out_path, 'wb+') as out_file:
            try:
                for chunk in p.stdout.read(getBufferSize()):
                    out_file.write(chunk)
            except:
                traceback.print_exc()
                raise errors.AnsibleError("failed to transfer file %s to %s" % (in_path, out_path))
            stdout, stderr = p.communicate()
            if p.returncode != 0:
                raise errors.AnsibleError("failed to transfer file %s to %s:\n%s\n%s" % (in_path, out_path, stdout, stderr))

    def close(self):
        pass