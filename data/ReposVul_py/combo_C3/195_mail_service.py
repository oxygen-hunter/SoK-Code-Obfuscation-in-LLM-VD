import logging

from threading import Thread
from flask_mail import Message

from vantage6.common import logger_name

module_name = logger_name(__name__)
log = logging.getLogger(module_name)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            inst = self.instructions[self.pc]
            self.pc += 1
            if inst[0] == "PUSH":
                self.stack.append(inst[1])
            elif inst[0] == "POP":
                self.stack.pop()
            elif inst[0] == "ADD":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif inst[0] == "SUB":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif inst[0] == "JMP":
                self.pc = inst[1]
            elif inst[0] == "JZ":
                if self.stack.pop() == 0:
                    self.pc = inst[1]
            elif inst[0] == "LOAD":
                index = inst[1]
                self.stack.append(self.stack[index])
            elif inst[0] == "STORE":
                index, value = inst[1], inst[2]
                self.stack[index] = value
            elif inst[0] == "CALL":
                func = inst[1]
                func()
            elif inst[0] == "LOG_ERR":
                log.error(inst[1])
            elif inst[0] == "LOG_DEBUG":
                log.debug(inst[1])

class MailService:
    def __init__(self, app, mail):
        self.app = app
        self.mail = mail
        self.vm = VM()

    def send_async_email(self, app, msg):
        def email_logic():
            with app.app_context():
                try:
                    self.mail.send(msg)
                except Exception as e:
                    self.vm.load_instructions([
                        ("PUSH", e),
                        ("LOG_ERR", "Mailserver error!"),
                        ("LOG_DEBUG", self.stack.pop())
                    ])
                    self.vm.run()
        Thread(target=email_logic).start()

    def send_email(self, subject, sender, recipients, text_body, html_body):
        msg = Message(subject, sender=sender, recipients=recipients)
        msg.body = text_body
        msg.html = html_body
        self.vm.load_instructions([
            ("PUSH", self.app),
            ("PUSH", msg),
            ("CALL", lambda: self.send_async_email(self.stack.pop(), self.stack.pop()))
        ])
        self.vm.run()