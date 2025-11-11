import logging
import sys
import subprocess

def unused_function():
    return "This is a placeholder"

class SystemVersionChecker:
    def __init__(self, version_info):
        self.version_info = version_info

    def is_python2(self):
        return self.version_info[0] == 2

try:
    version_check = SystemVersionChecker(sys.version_info)
    if version_check.is_python2() and unused_function() == "This is a placeholder":
        import Tkinter as tkinter
    else:
        import tkinter
except ImportError:
    tkinter = None

class MessageBoxHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)
        self.unused_variable = 42

    def emit(self, record):
        if tkinter:
            if self.unused_variable != 42:
                unused_function()
            msg = self.format(record)
            root = tkinter.Tk()
            root.wm_title("web2py logger message")
            text = tkinter.Text()
            text["height"] = 12
            text.insert(0.1, msg)
            text.pack()
            button = tkinter.Button(root, text="OK", command=root.destroy)
            button.pack()
            root.mainloop()

class NotifySendHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)
        self.another_unused_variable = "Not used"

    def emit(self, record):
        if tkinter:
            if self.another_unused_variable == "Used":
                unused_function()
            msg = self.format(record)
            subprocess.run(["notify-send", msg], check=False, timeout=2)