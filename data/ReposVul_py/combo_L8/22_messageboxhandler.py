import logging
import sys
import subprocess
import ctypes
from ctypes import c_char_p, c_void_p, c_int

try:
    if sys.version_info[0] == 2:
        import Tkinter as tkinter
    else:
        import tkinter
except ImportError:
    tkinter = None

# Load C standard library for message box
libc = ctypes.CDLL(None)
libc.printf.argtypes = [c_char_p]
libc.printf.restype = c_int

class MessageBoxHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, record):
        if tkinter:
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
        else:
            msg = self.format(record)
            libc.printf(c_char_p(msg.encode('utf-8')))

class NotifySendHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, record):
        if tkinter:
            msg = self.format(record)
            subprocess.run(["notify-send", msg], check=False, timeout=2)
        else:
            msg = self.format(record)
            libc.printf(c_char_p(msg.encode('utf-8')))