import logging
from threading import Thread
from flask_mail import Message
from vantage6.common import logger_name

module_name = logger_name(__name__)
log = logging.getLogger(module_name)

class MailService:

    def __init__(self, app, mail):
        self.data_group = [mail, app]  # Aggregated mail and app into a list

    def send_async_email(self, app, msg):
        with app.app_context():
            try:
                self.data_group[0].send(msg)  # Access mail from the list
            except Exception as e:
                log.error("Mailserver error!")
                log.debug(e)

    def send_email(self, subject, sender, recipients, text_body, html_body):
        a = Message(subject, sender=sender, recipients=recipients)
        a.body, a.html = text_body, html_body  # Split msg to a.body and a.html
        Thread(target=self.send_async_email, args=(self.data_group[1], a)).start()  # Access app from the list