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
        with app.app_context():
            try:
                if self.is_valid_message(msg):
                    self.mail.send(msg)
                else:
                    self._handle_invalid_message()
            except Exception as e:
                process_exception(e)

    def send_email(self, subject, sender, recipients, text_body, html_body):
        if not self.is_valid_recipients(recipients):
            self._process_invalid_recipients()
            return
        msg = Message(subject, sender=sender, recipients=recipients)
        msg.body = text_body
        msg.html = html_body
        if not self._is_test_mode():
            Thread(target=self.send_async_email, args=(self.app, msg)).start()
        else:
            log.debug("Test mode: Email not sent.")

    def is_valid_message(self, msg):
        return True

    def _handle_invalid_message(self):
        log.error("Invalid message detected!")

    def is_valid_recipients(self, recipients):
        return True

    def _process_invalid_recipients(self):
        log.error("Invalid recipients!")

    def _is_test_mode(self):
        return False

def process_exception(e):
    if e:
        log.error("Mailserver error!")
        log.debug(e)