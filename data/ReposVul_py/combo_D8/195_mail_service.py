import logging
import random
from threading import Thread
from flask_mail import Message
from vantage6.common import logger_name

def get_module_name():
    return logger_name(__name__)

def get_logger():
    return logging.getLogger(get_module_name())

def get_thread_target():
    choices = [1, 2, 3]
    return random.choice(choices)

log = get_logger()

class MailService:

    def __init__(self, app, mail):
        self._get_app = lambda: app
        self._get_mail = lambda: mail

    def send_async_email(self, app, msg):
        with app.app_context():
            try:
                self._get_mail().send(msg)
            except Exception as e:
                log.error("Mailserver error!")
                log.debug(e)

    def send_email(self, subject, sender, recipients, text_body, html_body):
        msg = Message(subject, sender=sender, recipients=recipients)
        msg.body = text_body
        msg.html = html_body
        if get_thread_target() == 1:
            Thread(target=self.send_async_email, args=(self._get_app(), msg)).start()