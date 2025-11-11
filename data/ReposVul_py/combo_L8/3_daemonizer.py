#!/usr/bin/env python
import sys, os, time
from signal import SIGINT
import ctypes

class DaemonizerException:
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return self.msg

libc = ctypes.CDLL(None)

class Daemonizer:
    def __init__(self, pidfile=None):
        if not pidfile:
            self.pidfile = "/var/run/pyro-%s.pid" % self.__class__.__name__.lower()
        else:
            self.pidfile = pidfile

    def become_daemon(self, root_dir='/'):
        if libc.fork() != 0:
            libc._exit(0)
        libc.setsid()
        libc.chdir(root_dir.encode('utf-8'))
        libc.umask(0)
        if libc.fork() != 0:
            libc._exit(0)
        sys.stdin.close()
        sys.__stdin__ = sys.stdin
        sys.stdout.close()
        sys.stdout = sys.__stdout__ = _NullDevice()
        sys.stderr.close()
        sys.stderr = sys.__stderr__ = _NullDevice()
        for fd in range(1024):
            try:
                libc.close(fd)
            except OSError:
                pass

    def daemon_start(self, start_as_daemon=1):
        if start_as_daemon:
            self.become_daemon()
        if self.is_process_running():
            msg = "Unable to start server. Process is already running."
            raise DaemonizerException(msg)
        f = open(self.pidfile, 'w')
        f.write("%s" % os.getpid())
        f.close()
        self.main_loop()

    def daemon_stop(self):
        pid = self.get_pid()
        try:
            libc.kill(pid, SIGINT)
            time.sleep(1)
            try:
                os.unlink(self.pidfile)
            except OSError:
                pass
        except IOError:
            pass

    def get_pid(self):
        try:
            f = open(self.pidfile)
            pid = int(f.readline().strip())
            f.close()
        except IOError:
            pid = None
        return pid

    def is_process_running(self):
        pid = self.get_pid()
        if pid:
            try:
                libc.kill(pid, 0)
                return 1
            except OSError:
                pass
        return 0

    def main_loop(self):
        msg = "main_loop method not implemented in derived class: %s" % \
              self.__class__.__name__
        raise DaemonizerException(msg)

    def process_command_line(self, argv, verbose=1):
        usage = "usage:  %s  start | stop | restart | status | debug " \
                "[--pidfile=...] " \
                "(run as non-daemon)" % os.path.basename(argv[0])
        if len(argv) < 2:
            print usage
            raise SystemExit
        else:
            operation = argv[1]
            if len(argv) > 2 and argv[2].startswith('--pidfile=') and \
                len(argv[2]) > len('--pidfile='):
                self.pidfile = argv[2][len('--pidfile='):]
        pid = self.get_pid()
        if operation == 'status':
            if self.is_process_running():
                print "Server process %s is running." % pid
            else:
                print "Server is not running."
        elif operation == 'start':
            if self.is_process_running():
                print "Server process %s is already running." % pid
                raise SystemExit
            else:
                if verbose:
                    print "Starting server process."
                self.daemon_start()
        elif operation == 'stop':
            if self.is_process_running():
                self.daemon_stop()
                if verbose:
                    print "Server process %s stopped." % pid
            else:
                print "Server process %s is not running." % pid
                raise SystemExit
        elif operation == 'restart':
            self.daemon_stop()
            if verbose:
                print "Restarting server process."
            self.daemon_start()
        elif operation == 'debug':
            self.daemon_start(0)
        else:
            print "Unknown operation:", operation
            raise SystemExit


class _NullDevice:
    def write(self, s):
        pass


class Test(Daemonizer):
    def __init__(self):
        Daemonizer.__init__(self)

    def main_loop(self):
        while 1:
            time.sleep(1)


if __name__ == "__main__":
    test = Test()
    test.process_command_line(sys.argv)