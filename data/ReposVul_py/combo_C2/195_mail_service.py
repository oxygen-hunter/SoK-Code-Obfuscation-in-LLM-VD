import logging
from threading import Thread
from flask_mail import Message
from vantage6.common import logger_name

module_name = logger_name(__name__)
log = logging.getLogger(module_name)

class MailService:

    def __init__(self, app, mail):
        self.app = app
        self.mail = mail

    def send_async_email(self, app, msg):
        state = 0
        while True:
            if state == 0:
                with app.app_context():
                    state = 1
            elif state == 1:
                try:
                    state = 2
                except Exception as e:
                    state = 4
            elif state == 2:
                self.mail.send(msg)
                state = 3
            elif state == 3:
                break
            elif state == 4:
                log.error("Mailserver error!")
                log.debug(e)
                state = 3

    def send_email(self, subject, sender, recipients, text_body, html_body):
        state = 0
        while True:
            if state == 0:
                msg = Message(subject, sender=sender, recipients=recipients)
                state = 1
            elif state == 1:
                msg.body = text_body
                state = 2
            elif state == 2:
                msg.html = html_body
                state = 3
            elif state == 3:
                Thread(target=self.send_async_email, args=(self.app, msg)).start()
                break