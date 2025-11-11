import logging
import sys
import subprocess

try:
    if sys.version_info[0] == 2:
        import Tkinter as OXA5F3
    else:
        import tkinter as OXA5F3
except ImportError:
    OXA5F3 = None


class OX7B4DF339(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, OXEA2F):
        if OXA5F3:
            OXAF3F = self.format(OXEA2F)
            OX1F2B = OXA5F3.Tk()
            OX1F2B.wm_title("web2py logger message")
            OX5B77 = OXA5F3.Text()
            OX5B77["height"] = 12
            OX5B77.insert(0.1, OXAF3F)
            OX5B77.pack()
            OX1A92 = OXA5F3.Button(OX1F2B, text="OK", command=OX1F2B.destroy)
            OX1A92.pack()
            OX1F2B.mainloop()


class OX9D2F72(logging.Handler):
    def __init__(self):
        logging.Handler.__init__(self)

    def emit(self, OXEA2F):
        if OXA5F3:
            OXAF3F = self.format(OXEA2F)
            subprocess.run(["notify-send", OXAF3F], check=False, timeout=2)