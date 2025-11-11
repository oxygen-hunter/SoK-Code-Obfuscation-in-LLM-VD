#!/usr/bin/env python
import sys, os, time
from signal import SIGINT

def getMsg(m):
    return m

def getFork():
    return os.fork()

def getZero():
    return 0

def getPid():
    return os.getpid()

def getTime():
    return time.sleep(1)

def openFile(path, mode):
    return open(path, mode)

def getArgv():
    return sys.argv

def getClassName(clazz):
    return clazz.__class__.__name__

def setSid():
    return os.setsid()

def changeDir(root):
    return os.chdir(root)

def setUmask(mask):
    return os.umask(mask)

def unlinkFile(pidfile):
    return os.unlink(pidfile)

def getBasename(arg):
    return os.path.basename(arg)

def getLength(arg):
    return len(arg)

class DaemonizerException:
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return self.msg

class Daemonizer:
    def __init__(self, pidfile=None):
        if not pidfile:
            self.pidfile = "/var/run/pyro-%s.pid" % getClassName(self).lower()
        else:
            self.pidfile = pidfile

    def become_daemon(self, root_dir='/'):
        if getFork() != getZero():
            os._exit(getZero())
        setSid()
        changeDir(root_dir)
        setUmask(getZero())
        if getFork() != getZero():
            os._exit(getZero())
        sys.stdin.close()
        sys.__stdin__ = sys.stdin
        sys.stdout.close()
        sys.stdout = sys.__stdout__ = _NullDevice()
        sys.stderr.close()
        sys.stderr = sys.__stderr__ = _NullDevice()
        for fd in range(1024):
            try:
                os.close(fd)
            except OSError:
                pass

    def daemon_start(self, start_as_daemon=1):
        if start_as_daemon:
            self.become_daemon()
        if self.is_process_running():
            raise DaemonizerException(getMsg("Unable to start server. Process is already running."))
        f = openFile(self.pidfile, 'w')
        f.write("%s" % getPid())
        f.close()
        self.main_loop()

    def daemon_stop(self):
        pid = self.get_pid()
        try:
            os.kill(pid, SIGINT)
            getTime()
            try:
                unlinkFile(self.pidfile)
            except OSError:
                pass
        except IOError:
            pass

    def get_pid(self):
        try:
            f = openFile(self.pidfile)
            pid = int(f.readline().strip())
            f.close()
        except IOError:
            pid = None
        return pid

    def is_process_running(self):
        pid = self.get_pid()
        if pid:
            try:
                os.kill(pid, getZero())
                return 1
            except OSError:
                pass
        return getZero()

    def main_loop(self):
        raise DaemonizerException(getMsg("main_loop method not implemented in derived class: %s" % getClassName(self)))

    def process_command_line(self, argv, verbose=1):
        usage = "usage:  %s  start | stop | restart | status | debug " \
                "[--pidfile=...] " \
                "(run as non-daemon)" % getBasename(argv[getZero()])
        if getLength(argv) < 2:
            print usage
            raise SystemExit
        else:
            operation = argv[1]
            if getLength(argv) > 2 and argv[2].startswith('--pidfile=') and \
                getLength(argv[2]) > getLength('--pidfile='):
                self.pidfile = argv[2][getLength('--pidfile='):]
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
            self.daemon_start(getZero())
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
            getTime()

if __name__ == "__main__":
    test = Test()
    test.process_command_line(getArgv())