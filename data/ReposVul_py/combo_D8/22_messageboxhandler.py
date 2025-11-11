import logging
import sys
import subprocess

def getTkinter():
    try:
        if sys.version_info[0] == 2:
            import Tkinter as tk
        else:
            import tkinter as tk
    except ImportError:
        tk = None
    return tk

tkinter = getTkinter()

class MessageBoxHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, record):
        def getMsg():
            return self.format(record)

        def getRoot():
            root = tkinter.Tk()
            root.wm_title("web2py logger message")
            return root
        
        if tkinter:
            msg = getMsg()
            root = getRoot()
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

    def emit(self, record):
        def getMsg():
            return self.format(record)

        if tkinter:
            msg = getMsg()
            subprocess.run(["notify-send", msg], check=False, timeout=2)