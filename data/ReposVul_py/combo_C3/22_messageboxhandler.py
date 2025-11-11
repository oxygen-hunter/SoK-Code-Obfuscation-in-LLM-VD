import logging
import sys
import subprocess

try:
    if sys.version_info[0] == 2:
        import Tkinter as tkinter
    else:
        import tkinter
except ImportError:
    tkinter = None

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.handlers = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def execute(self):
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            if opcode == "PUSH":
                self.stack.append(args[0])
            elif opcode == "POP":
                self.stack.pop()
            elif opcode == "CALL":
                self.handlers[args[0]]()
            elif opcode == "LOAD":
                self.stack.append(self.handlers[args[0]])
            elif opcode == "JMP":
                self.pc = args[0] - 1
            self.pc += 1

    def register_handler(self, handler):
        self.handlers.append(handler)
        return len(self.handlers) - 1

vm = VirtualMachine()

def message_box_handler():
    if tkinter:
        msg = vm.stack.pop()
        root = tkinter.Tk()
        root.wm_title("web2py logger message")
        text = tkinter.Text()
        text["height"] = 12
        text.insert(0.1, msg)
        text.pack()
        button = tkinter.Button(root, text="OK", command=root.destroy)
        button.pack()
        root.mainloop()

def notify_send_handler():
    if tkinter:
        msg = vm.stack.pop()
        subprocess.run(["notify-send", msg], check=False, timeout=2)

class ObfuscatedMessageBoxHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)
        self.handler_index = vm.register_handler(message_box_handler)

    def emit(self, record):
        msg = self.format(record)
        vm.load_instructions([("PUSH", msg), ("CALL", self.handler_index)])
        vm.execute()

class ObfuscatedNotifySendHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)
        self.handler_index = vm.register_handler(notify_send_handler)

    def emit(self, record):
        msg = self.format(record)
        vm.load_instructions([("PUSH", msg), ("CALL", self.handler_index)])
        vm.execute()