#!/usr/bin/env python
import sys, os, time
from signal import SIGINT

class OX7B4DF339:
    def __init__(self, OX0C4D61C4):
        self.OX0C4D61C4 = OX0C4D61C4
    def __str__(self):
        return self.OX0C4D61C4

class OX1A2B3C4D:
    def __init__(self, OX5E6F7A8B=None):
        if not OX5E6F7A8B:
            self.OX5E6F7A8B = "/var/run/pyro-%s.pid" % self.__class__.__name__.lower()
        else:
            self.OX5E6F7A8B = OX5E6F7A8B

    def OX2B3C4D5E(self, OX8C9D0E1F='/'):
        if os.fork() != 0:
            os._exit(0)
        os.setsid()
        os.chdir(OX8C9D0E1F)
        os.umask(0)
        if os.fork() != 0:
            os._exit(0)
        sys.stdin.close()
        sys.__stdin__ = sys.stdin
        sys.stdout.close()
        sys.stdout = sys.__stdout__ = OX4D5E6F7A()
        sys.stderr.close()
        sys.stderr = sys.__stderr__ = OX4D5E6F7A()
        for OX9A0B1C2D in range(1024):
            try:
                os.close(OX9A0B1C2D)
            except OSError:
                pass

    def OX3C4D5E6F(self, OX2A3B4C5D=1):
        if OX2A3B4C5D:
            self.OX2B3C4D5E()
        if self.OX5F6A7B8C():
            OX0C4D61C4 = "Unable to start server. Process is already running."
            raise OX7B4DF339(OX0C4D61C4)
        OX5D6E7F8A = open(self.OX5E6F7A8B, 'w')
        OX5D6E7F8A.write("%s" % os.getpid())
        OX5D6E7F8A.close()
        self.OX1B2C3D4E()

    def OX4D5E6F7G(self):
        OX0E1F2A3B = self.OX6B7C8D9E()
        try:
            os.kill(OX0E1F2A3B, SIGINT)
            time.sleep(1)
            try:
                os.unlink(self.OX5E6F7A8B)
            except OSError:
                pass
        except IOError:
            pass

    def OX6B7C8D9E(self):
        try:
            OX5D6E7F8A = open(self.OX5E6F7A8B)
            OX0E1F2A3B = int(OX5D6E7F8A.readline().strip())
            OX5D6E7F8A.close()
        except IOError:
            OX0E1F2A3B = None
        return OX0E1F2A3B

    def OX5F6A7B8C(self):
        OX0E1F2A3B = self.OX6B7C8D9E()
        if OX0E1F2A3B:
            try:
                os.kill(OX0E1F2A3B, 0)
                return 1
            except OSError:
                pass
        return 0

    def OX1B2C3D4E(self):
        OX0C4D61C4 = "main_loop method not implemented in derived class: %s" % \
              self.__class__.__name__
        raise OX7B4DF339(OX0C4D61C4)

    def OX7C8D9E0F(self, OX3B4C5D6E, OX0A1B2C3D=1):
        OX2D3E4F5A = "usage:  %s  start | stop | restart | status | debug " \
                "[--pidfile=...] " \
                "(run as non-daemon)" % os.path.basename(OX3B4C5D6E[0])
        if len(OX3B4C5D6E) < 2:
            print OX2D3E4F5A
            raise SystemExit
        else:
            OX9A0B1C2D = OX3B4C5D6E[1]
            if len(OX3B4C5D6E) > 2 and OX3B4C5D6E[2].startswith('--pidfile=') and \
                len(OX3B4C5D6E[2]) > len('--pidfile='):
                self.OX5E6F7A8B = OX3B4C5D6E[2][len('--pidfile='):]
        OX0E1F2A3B = self.OX6B7C8D9E()
        if OX9A0B1C2D == 'status':
            if self.OX5F6A7B8C():
                print "Server process %s is running." % OX0E1F2A3B
            else:
                print "Server is not running."
        elif OX9A0B1C2D == 'start':
            if self.OX5F6A7B8C():
                print "Server process %s is already running." % OX0E1F2A3B
                raise SystemExit
            else:
                if OX0A1B2C3D:
                    print "Starting server process."
                self.OX3C4D5E6F()
        elif OX9A0B1C2D == 'stop':
            if self.OX5F6A7B8C():
                self.OX4D5E6F7G()
                if OX0A1B2C3D:
                    print "Server process %s stopped." % OX0E1F2A3B
            else:
                print "Server process %s is not running." % OX0E1F2A3B
                raise SystemExit
        elif OX9A0B1C2D == 'restart':
            self.OX4D5E6F7G()
            if OX0A1B2C3D:
                print "Restarting server process."
            self.OX3C4D5E6F()
        elif OX9A0B1C2D == 'debug':
            self.OX3C4D5E6F(0)
        else:
            print "Unknown operation:", OX9A0B1C2D
            raise SystemExit

class OX4D5E6F7A:
    def write(self, OX3C4D5E6F):
        pass

class OX0A1B2C3D(OX1A2B3C4D):
    def __init__(self):
        OX1A2B3C4D.__init__(self)

    def OX1B2C3D4E(self):
        while 1:
            time.sleep(1)

if __name__ == "__main__":
    OX9B0C1D2E = OX0A1B2C3D()
    OX9B0C1D2E.OX7C8D9E0F(sys.argv)