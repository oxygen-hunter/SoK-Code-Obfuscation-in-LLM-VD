import sys, os, time
from signal import SIGINT

class DaemonizerException:
    def __init__(self, msg):
        self.msg = msg
    def __str__(self):
        return self.msg

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.registers = {'PC': 0, 'A': 0, 'B': 0, 'C': 0, 'D': 0}
        self.running = True

    def run(self, instructions):
        while self.running:
            instruction = instructions[self.registers['PC']]
            self.registers['PC'] += 1
            self.execute(instruction)
    
    def execute(self, instruction):
        cmd = instruction[0]
        if cmd == 'PUSH':
            self.stack.append(instruction[1])
        elif cmd == 'POP':
            self.stack.pop()
        elif cmd == 'ADD':
            self.stack.append(self.stack.pop() + self.stack.pop())
        elif cmd == 'SUB':
            b, a = self.stack.pop(), self.stack.pop()
            self.stack.append(a - b)
        elif cmd == 'JMP':
            self.registers['PC'] = instruction[1]
        elif cmd == 'JZ':
            if self.stack.pop() == 0:
                self.registers['PC'] = instruction[1]
        elif cmd == 'LOAD':
            self.registers[instruction[1]] = self.stack.pop()
        elif cmd == 'STORE':
            self.stack.append(self.registers[instruction[1]])
        elif cmd == 'HALT':
            self.running = False

class Daemonizer:
    def __init__(self, pidfile=None):
        self.vm = VirtualMachine()
        if not pidfile:
            self.pidfile = "/var/run/pyro-%s.pid" % self.__class__.__name__.lower()
        else:
            self.pidfile = pidfile

    def become_daemon(self, root_dir='/'):
        self.vm.run([
            ('PUSH', 0),
            ('PUSH', os.fork()),
            ('SUB',),
            ('JZ', 6),
            ('LOAD', 'A'),
            ('JMP', 31),
            ('PUSH', os.setsid()),
            ('PUSH', os.chdir(root_dir)),
            ('PUSH', os.umask(0)),
            ('PUSH', os.fork()),
            ('SUB',),
            ('JZ', 17),
            ('LOAD', 'A'),
            ('JMP', 31),
            ('PUSH', sys.stdin.close()),
            ('PUSH', sys.stdout.close()),
            ('PUSH', sys.stderr.close()),
            ('PUSH', range(1024)),
            ('LOAD', 'B'),
            ('PUSH', 1),
            ('SUB',),
            ('JZ', 31),
            ('LOAD', 'C'),
            ('PUSH', os.close(self.vm.stack.pop())),
            ('JMP', 19),
            ('HALT',)
        ])

    def daemon_start(self, start_as_daemon=1):
        if start_as_daemon:
            self.become_daemon()
        if self.is_process_running():
            raise DaemonizerException("Unable to start server. Process is already running.")
        with open(self.pidfile, 'w') as f:
            f.write("%s" % os.getpid())
        self.main_loop()

    def daemon_stop(self):
        pid = self.get_pid()
        try:
            os.kill(pid, SIGINT)
            time.sleep(1)
            try:
                os.unlink(self.pidfile)
            except OSError:
                pass
        except IOError:
            pass

    def get_pid(self):
        try:
            with open(self.pidfile) as f:
                return int(f.readline().strip())
        except IOError:
            return None

    def is_process_running(self):
        pid = self.get_pid()
        if pid:
            try:
                os.kill(pid, 0)
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