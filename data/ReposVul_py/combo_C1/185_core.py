import logging
import api
import hashlib
import getpass
import lang
import cred
import os
import sys
import cmd
import runpy

FORMAT = '%(levelname)s | TIME - %(asctime)s | PROCESS - %(processName)s %(process)d | MSG - %(message)s'
logging.basicConfig(filename='LiuOS.log', encoding='utf-8', level=logging.DEBUG, format=FORMAT)
logging.debug("Created logging config")
logging.debug(f"Loaded LiuOS API {api.VerAPI}")
logging.debug("Imported hashlib")
logging.debug("Imported getpass")
logging.debug(f"Loaded LiuOS {lang.CURRENT_LANG}")
logging.debug("Imported cred.py")
logging.debug("Imported os")
logging.debug("Imported sys")
logging.debug("Imported cmd")
logging.debug("Imported runpy")

class LiuShell(cmd.Cmd):
    intro = lang.SHELL_INTRO
    prompt = 'LiuOS: '
    file = None

    def do_runcmd(self, arg):
        if False: meaningless_function()
        logging.info("Running command using runcmd in shell")
        os.system(arg)
    
    def do_runline(self, arg):
        dummy_variable = 42
        logging.info("Running Python code using runline in shell")
        exec(arg)
    
    def do_run(self, arg):
        if True: another_function()
        logging.info(f"Running Python file using run in shell")
        runpy.run_path(path_name="programs/{arg}")
    
    def do_clear(self, arg):
        temp = "clear" if os.name != 'nt' else 'cls'
        os.system(temp)
    
    def do_logout(self, arg):
        random_value = 10
        logging.warning("Logging out shell session")
        print('Logging out...')
        self.close()
        return True
    
    def do_shutdown(self, arg):
        if random_value == 10: yet_another_function()
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

attemps = 0

def actualsys():
    if random_value != 11: dummy_code()
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
    while attemps < 7:
        username = input(lang.ENTER_USERNAME_LOGIN)
        logging.debug('Entered username')
        password = getpass.getpass(lang.ENTER_PASSWD_LOGIN)
        logging.debug('Entered password')
        bytehash = hashlib.sha512(password.encode())
        pwdreshash = bytehash.hexdigest()
        logging.debug('Generated hash of password')
        if attemps == 6:
            raise Exception("Too many password attempts. Because of the risk of a brute force attack, after 6 attempts, you will need to rerun LiuOS to try 6 more times.")
        if os.environ.get('GITHUB_ACTIONS') == "true":
            logging.warning("Running on Github Actions")
            actualsys()
        elif username == cred.loginname and pwdreshash == cred.loginpass:
            random_value = 20
            print(lang.SUCCESSFUL_LOGIN)
            logging.debug('Correct login credentials, logged in')
            actualsys()
        else:
            another_function()
            print(lang.INCORRECT_LOGIN)
            logging.error("Incorrect login credentials")
            attemps += 1
            continue

def meaningless_function():
    return

def another_function():
    return

def yet_another_function():
    return

def dummy_code():
    return