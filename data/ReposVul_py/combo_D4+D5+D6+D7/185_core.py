import logging
FORMAT = '%(levelname)s | TIME - %(asctime)s | PROCESS - %(processName)s %(process)d | MSG - %(message)s'
logging.basicConfig(filename='LiuOS.log', encoding='utf-8', level=logging.DEBUG, format=FORMAT)
logging.debug("Created logging config")
import api
logging.debug(f"Loaded LiuOS API {api.VerAPI}")
import hashlib
logging.debug("Imported hashlib")
import getpass
logging.debug("Imported getpass")
import lang
logging.debug(f"Loaded LiuOS {lang.CURRENT_LANG}")
import cred
logging.debug("Imported cred.py")
import os
logging.debug("Imported os")
import sys
logging.debug("Imported sys")
import cmd
logging.debug("Imported cmd")
import runpy
logging.debug("Imported runpy")

class LiuShell(cmd.Cmd):
    prompt = 'LiuOS: '
    intro = lang.SHELL_INTRO
    file = None

    def do_runcmd(self, arg):
        logging.info("Running command using runcmd in shell")
        os.system(arg)
    def do_runline(self, arg):
        logging.info("Running Python code using runline in shell")
        exec(arg)
    def do_run(self, arg):
        logging.info(f"Running Python file using run in shell")
        runpy.run_path(path_name="programs/{arg}")
    def do_clear(self, arg):
        os.system('cls' if os.name == 'nt' else 'clear')
    def do_logout(self, arg):
        logging.warning("Logging out shell session")
        print('Logging out...')
        self.close()
        return True
    def do_shutdown(self, arg):
        print('Logging out...')
        logging.info("Shut down using shell command")
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

g = [0]
def actualsys() :
    logging.debug("Launched main system")
    os.system('cls' if os.name == 'nt' else 'clear')
    logging.debug("Loaded LiuOS Shell")
    LiuShell().cmdloop()
logging.debug("Assigned main system function")
if os.environ.get('GITHUB_ACTIONS') == "true":
    logging.info('Running on GitHub Actions, not using the LiuOS Shell')
    print(lang.ENTER_USERNAME_LOGIN)
    print(lang.ENTER_PASSWD_LOGIN)
    print(lang.FAKE_SUCCESSFUL_LOGIN)
    logging.warning("Fake login completed")
    print(lang.SHELL_INTRO)
    print(lang.SAMPLE_ABC)
    print(lang.SAMPLE_STRING)
    TestProg = "programs/helloworld.py"
    runpy.run_path(path_name=TestProg)
    print("Code completed")
else:
    while g[0] < 7:
        a, b = input(lang.ENTER_USERNAME_LOGIN), getpass.getpass(lang.ENTER_PASSWD_LOGIN)
        logging.debug('Entered username')
        logging.debug('Entered password')
        c, d = hashlib.sha512(b.encode()), c.hexdigest()
        logging.debug('Generated hash of password')
        if g[0] == 6:
            raise Exception("Too many password attempts. Because of the risk of a brute force attack, after 6 attempts, you will need to rerun LiuOS to try 6 more times.")
        if os.environ.get('GITHUB_ACTIONS') == "true":
            logging.warning("Running on Github Actions")
            actualsys()
        elif a == cred.loginname and d == cred.loginpass:
            print(lang.SUCCESSFUL_LOGIN)
            logging.debug('Correct login credentials, logged in')
            actualsys()
        else:
            print(lang.INCORRECT_LOGIN)
            logging.error("Incorrect login credentials")
            g[0] += 1
            continue