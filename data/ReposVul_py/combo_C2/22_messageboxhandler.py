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

class MessageBoxHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, record):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if tkinter:
                    dispatcher = 1
                else:
                    break
            elif dispatcher == 1:
                msg = self.format(record)
                dispatcher = 2
            elif dispatcher == 2:
                root = tkinter.Tk()
                dispatcher = 3
            elif dispatcher == 3:
                root.wm_title("web2py logger message")
                dispatcher = 4
            elif dispatcher == 4:
                text = tkinter.Text()
                dispatcher = 5
            elif dispatcher == 5:
                text["height"] = 12
                dispatcher = 6
            elif dispatcher == 6:
                text.insert(0.1, msg)
                dispatcher = 7
            elif dispatcher == 7:
                text.pack()
                dispatcher = 8
            elif dispatcher == 8:
                button = tkinter.Button(root, text="OK", command=root.destroy)
                dispatcher = 9
            elif dispatcher == 9:
                button.pack()
                dispatcher = 10
            elif dispatcher == 10:
                root.mainloop()
                break

class NotifySendHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, record):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                if tkinter:
                    dispatcher = 1
                else:
                    break
            elif dispatcher == 1:
                msg = self.format(record)
                dispatcher = 2
            elif dispatcher == 2:
                subprocess.run(["notify-send", msg], check=False, timeout=2)
                break