import logging as L
import ctypes as C
import api as A
import hashlib as H
import getpass as G
import lang as Lg
import cred as Cr
import os as O
import sys as S
import cmd as C
import runpy as R

FMT = '%(levelname)s | TIME - %(asctime)s | PROCESS - %(processName)s %(process)d | MSG - %(message)s'
L.basicConfig(filename='LiuOS.log', encoding='utf-8', level=L.DEBUG, format=FMT)
L.debug("Created logging config")
L.debug(f"Loaded LiuOS API {A.VerAPI}")
L.debug("Imported hashlib")
L.debug("Imported getpass")
L.debug(f"Loaded LiuOS {Lg.CURRENT_LANG}")
L.debug("Imported cred.py")
L.debug("Imported os")
L.debug("Imported sys")
L.debug("Imported cmd")
L.debug("Imported runpy")

class LiuShell(C.Cmd):
    intro = Lg.SHELL_INTRO
    prompt = 'LiuOS: '
    file = None

    def do_runcmd(self, arg):
        L.info("Running command using runcmd in shell")
        O.system(arg)
    def do_runline(self, arg):
        L.info("Running Python code using runline in shell")
        exec(arg)
    def do_run(self, arg):
        L.info(f"Running Python file using run in shell")
        R.run_path(path_name=f"programs/{arg}")
    def do_clear(self, arg):
        O.system('cls' if O.name == 'nt' else 'clear')
    def do_logout(self, arg):
        L.warning("Logging out shell session")
        print('Logging out...')
        self.close()
        return True
    def do_shutdown(self, arg):
        print('Logging out...')
        L.info("Shut down using shell command")
        exit()
        return True

    def do_savecmd(self, arg):
        self.file = open(arg, 'w')
    def do_opencmd(self, arg):
        self.close()
        with open(arg) as f:
            self.cmdqueue.extend(f.read().splitlines())
    def precmd(self, line):
        line = line.lower()
        if self.file and 'playback' not in line:
            print(line, file=self.file)
        return line
    def close(self):
        if self.file:
            self.file.close()
            self.file = None

def parse(arg):
    return tuple(map(int, arg.split()))

Ats = 0

def actualsys():
    L.debug("Launched main system")
    O.system('cls' if O.name == 'nt' else 'clear')
    L.debug("Loaded LiuOS Shell")
    LiuShell().cmdloop()

L.debug("Assigned main system function")

if O.environ.get('GITHUB_ACTIONS') == "true":
    L.info('Running on GitHub Actions, not using the LiuOS Shell')
    print(Lg.ENTER_USERNAME_LOGIN)
    print(Lg.ENTER_PASSWD_LOGIN)
    print(Lg.FAKE_SUCCESSFUL_LOGIN)
    L.warning("Fake login completed")
    print(Lg.SHELL_INTRO)
    print(Lg.SAMPLE_ABC)
    print(Lg.SAMPLE_STRING)
    TestProg = "programs/helloworld.py"
    R.run_path(path_name=TestProg)
    print("Code completed")
else:
    while Ats < 7:
        username = input(Lg.ENTER_USERNAME_LOGIN)
        L.debug('Entered username')
        password = G.getpass(Lg.ENTER_PASSWD_LOGIN)
        L.debug('Entered password')
        bytehash = H.sha512(password.encode())
        pwdreshash = bytehash.hexdigest()
        L.debug('Generated hash of password')
        if Ats == 6:
            raise Exception("Too many password attempts. Because of the risk of a brute force attack, after 6 attempts, you will need to rerun LiuOS to try 6 more times.")
        if O.environ.get('GITHUB_ACTIONS') == "true":
            L.warning("Running on Github Actions")
            actualsys()
        elif username == Cr.loginname and pwdreshash == Cr.loginpass:
            print(Lg.SUCCESSFUL_LOGIN)
            L.debug('Correct login credentials, logged in')
            actualsys()
        else:
            print(Lg.INCORRECT_LOGIN)
            L.error("Incorrect login credentials")
            Ats += 1
            continue