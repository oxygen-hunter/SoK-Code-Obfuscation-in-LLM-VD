import logging
import sys
import subprocess

def import_tkinter():
    if sys.version_info[0] == 2:
        try:
            import Tkinter as tkinter
            return tkinter
        except ImportError:
            pass
    else:
        try:
            import tkinter
            return tkinter
        except ImportError:
            pass
    return None

tkinter = import_tkinter()

class MessageBoxHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, record):
        def open_message_box(msg):
            if tkinter:
                root = tkinter.Tk()
                root.wm_title("web2py logger message")
                text = tkinter.Text()
                text["height"] = 12
                text.insert(0.1, msg)
                text.pack()
                button = tkinter.Button(root, text="OK", command=root.destroy)
                button.pack()
                root.mainloop()

        msg = self.format(record)
        open_message_box(msg)

class NotifySendHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, record):
        def send_notification(msg):
            if tkinter:
                subprocess.run(["notify-send", msg], check=False, timeout=2)

        msg = self.format(record)
        send_notification(msg)