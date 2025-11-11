import logging
import sys
import subprocess

try:
    if sys.version_info[0] == 2:
        import Tkinter as t
    else:
        import tkinter as t
except ImportError:
    t = None

class MessageBoxHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, record):
        if t:
            _ = self.format(record)
            _a = t.Tk()
            _a.wm_title("web2py logger message")
            _b = t.Text()
            _b["height"] = 12
            _b.insert(0.1, _)
            _b.pack()
            _c = t.Button(_a, text="OK", command=_a.destroy)
            _c.pack()
            _a.mainloop()

class NotifySendHandler(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, record):
        if t:
            _ = self.format(record)
            subprocess.run(["notify-send", _], check=False, timeout=2)