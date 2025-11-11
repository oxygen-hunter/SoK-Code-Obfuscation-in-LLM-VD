import logging
from threading import Thread
from flask_mail import Message
from vantage6.common import logger_name

a = logger_name(__name__)
b = logging.getLogger(a)

class c:
    def __init__(self, d, e):
        self.app = d
        self.mail = e

    def f(self, g, h):
        with g.app_context():
            try:
                self.mail.send(h)
            except Exception as i:
                b.error("Mailserver error!")
                b.debug(i)

    def j(self, k, l, m, n, o):
        p = Message(k, sender=l, recipients=m)
        p.body = n
        p.html = o
        Thread(target=self.f, args=(self.app, p)).start()