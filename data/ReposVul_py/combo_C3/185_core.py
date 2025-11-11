# import stuff
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

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running:
            if self.pc < len(self.instructions):
                instr, *args = self.instructions[self.pc]
                self.pc += 1
                getattr(self, f"op_{instr}")(*args)
            else:
                self.running = False

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, addr):
        self.pc = addr

    def op_JZ(self, addr):
        if self.stack.pop() == 0:
            self.pc = addr

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.stack.pop()

    def op_PRINT(self):
        print(self.stack.pop())

    def op_HALT(self):
        self.running = False

class LiuShell(cmd.Cmd):
    intro = lang.SHELL_INTRO
    prompt = 'LiuOS: '
    file = None

    def do_runcmd(self, arg):
        logging.info("Running command using runcmd in shell")
        os.system(arg)

    def do_runline(self, arg):
        logging.info("Running Python code using runline in shell")
        exec(arg)

    def do_run(self, arg):
        logging.info(f"Running Python file using run in shell")
        runpy.run_path(path_name=f"programs/{arg}")

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

def vm_main():
    vm = VM()
    vm.load_instructions([
        ("PUSH", 0),  # attemps = 0
        ("STORE", 0),
        ("LOAD", 0),
        ("PUSH", 7),
        ("SUB",),
        ("JZ", 31),  # while attemps < 7
        ("LOAD", 0),
        ("PUSH", 1),
        ("ADD",),
        ("STORE", 0),
        ("PUSH", 1),  # call actualsys if attemps == 6
        ("LOAD", 0),
        ("PUSH", 6),
        ("SUB",),
        ("JZ", 29),
        ("JMP", 0),
        ("LOAD", 0),
        ("PUSH", 1),
        ("ADD",),
        ("STORE", 0),
        ("JMP", 0),
        ("HALT",)
    ])
    vm.run()

def parse(arg):
    return tuple(map(int, arg.split()))

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
    username = input(lang.ENTER_USERNAME_LOGIN)
    logging.debug('Entered username')
    password = getpass.getpass(lang.ENTER_PASSWD_LOGIN)
    logging.debug('Entered password')
    bytehash = hashlib.sha512(password.encode())
    pwdreshash = bytehash.hexdigest()
    logging.debug('Generated hash of password')
    if username == cred.loginname and pwdreshash == cred.loginpass:
        print(lang.SUCCESSFUL_LOGIN)
        logging.debug('Correct login credentials, logged in')
        LiuShell().cmdloop()
    else:
        print(lang.INCORRECT_LOGIN)
        logging.error("Incorrect login credentials")
        vm_main()