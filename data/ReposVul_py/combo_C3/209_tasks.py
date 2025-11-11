import os

from celery import shared_task
from django.conf import settings
from django.core.mail import send_mail, EmailMultiAlternatives
from django.utils.translation import gettext_lazy as _

from .utils import get_logger

logger = get_logger(__file__)

# Define VM Instructions
PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT = range(11)

# Virtual Machine
class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.context = {}

    def execute(self, instructions):
        self.instructions = instructions
        self.pc = 0
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            op = instr[0]
            if op == PUSH:
                self.stack.append(instr[1])
            elif op == POP:
                self.stack.pop()
            elif op == ADD:
                self.stack.append(self.stack.pop() + self.stack.pop())
            elif op == SUB:
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif op == JMP:
                self.pc = instr[1]
                continue
            elif op == JZ:
                if self.stack.pop() == 0:
                    self.pc = instr[1]
                    continue
            elif op == LOAD:
                self.stack.append(self.context[instr[1]])
            elif op == STORE:
                self.context[instr[1]] = self.stack.pop()
            elif op == CALL:
                self.context[instr[1]](*self.stack)
            elif op == RET:
                return self.stack.pop()
            elif op == HALT:
                break
            self.pc += 1

# Compiled task_activity_callback logic into VM instructions
def task_activity_callback(*args, **kwargs):
    from users.models import User
    vm = VM()
    instructions = [
        (LOAD, 'recipient_list'),
        (CALL, 'filter_users'),
        (STORE, 'resource_ids'),
        (LOAD, 'resource_ids'),
        (RET, )
    ]
    vm.context.update({
        'recipient_list': kwargs['recipient_list'],
        'filter_users': lambda emails: list(User.objects.filter(email__in=emails).values_list('id', flat=True))
    })
    return vm.execute(instructions),

# Compiled send_mail_async logic into VM instructions
@shared_task(verbose_name=_("Send email"), activity_callback=task_activity_callback)
def send_mail_async(*args, **kwargs):
    vm = VM()
    instructions = [
        (LOAD, 'args'),
        (LOAD, 'kwargs'),
        (CALL, 'prepare_args'),
        (STORE, 'args'),
        (LOAD, 'args'),
        (LOAD, 'kwargs'),
        (CALL, 'send_mail_wrapper'),
        (RET, )
    ]
    vm.context.update({
        'args': args,
        'kwargs': kwargs,
        'prepare_args': lambda args: ((settings.EMAIL_SUBJECT_PREFIX or '') + args[0], *(args[1:2]), (settings.EMAIL_FROM or settings.EMAIL_HOST_USER), *args[2:]) if len(args) == 3 else args,
        'send_mail_wrapper': lambda *args, **kwargs: send_mail(*args, **kwargs)
    })
    try:
        return vm.execute(instructions)
    except Exception as e:
        logger.error("Sending mail error: {}".format(e))

# Compiled send_mail_attachment_async logic into VM instructions
@shared_task(verbose_name=_("Send email attachment"), activity_callback=task_activity_callback)
def send_mail_attachment_async(subject, message, recipient_list, attachment_list=None):
    vm = VM()
    instructions = [
        (LOAD, 'subject'),
        (LOAD, 'message'),
        (LOAD, 'recipient_list'),
        (LOAD, 'attachment_list'),
        (CALL, 'send_email_with_attachments'),
        (RET, )
    ]
    vm.context.update({
        'subject': subject,
        'message': message,
        'recipient_list': recipient_list,
        'attachment_list': attachment_list or [],
        'send_email_with_attachments': lambda subj, msg, to, attachments: send_email_with_attachments(subj, msg, to, attachments)
    })
    try:
        return vm.execute(instructions)
    except Exception as e:
        logger.error("Sending mail attachment error: {}".format(e))

def send_email_with_attachments(subject, message, recipient_list, attachment_list):
    from_email = settings.EMAIL_FROM or settings.EMAIL_HOST_USER
    subject = (settings.EMAIL_SUBJECT_PREFIX or '') + subject
    email = EmailMultiAlternatives(
        subject=subject,
        body=message,
        from_email=from_email,
        to=recipient_list
    )
    for attachment in attachment_list:
        email.attach_file(attachment)
        os.remove(attachment)
    return email.send()