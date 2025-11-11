import os

from celery import shared_task
from django.conf import settings
from django.core.mail import send_mail, EmailMultiAlternatives
from django.utils.translation import gettext_lazy as _

from .utils import get_logger

logger = get_logger(__file__)

def get_email_list(recipient_list):
    return recipient_list

def get_user_ids(email_list):
    from users.models import User
    return list(User.objects.filter(email__in=email_list).values_list('id', flat=True))

def task_activity_callback(self, subject, message, recipient_list, *args, **kwargs):
    email_list = get_email_list(recipient_list)
    resource_ids = get_user_ids(email_list)
    return resource_ids,


@shared_task(verbose_name=_("Send email"), activity_callback=task_activity_callback)
def send_mail_async(*args, **kwargs):
    def get_subject_prefix():
        return settings.EMAIL_SUBJECT_PREFIX or ''

    def get_from_email():
        return settings.EMAIL_FROM or settings.EMAIL_HOST_USER

    if len(args) == 3:
        args = list(args)
        args[0] = get_subject_prefix() + args[0]
        args.insert(2, get_from_email())

    args = tuple(args)
    try:
        return send_mail(*args, **kwargs)
    except Exception as e:
        logger.error("Sending mail error: {}".format(e))


@shared_task(verbose_name=_("Send email attachment"), activity_callback=task_activity_callback)
def send_mail_attachment_async(subject, message, recipient_list, attachment_list=None):
    def get_from_email():
        return settings.EMAIL_FROM or settings.EMAIL_HOST_USER

    def get_subject_prefix():
        return settings.EMAIL_SUBJECT_PREFIX or ''
    
    if attachment_list is None:
        attachment_list = []
    from_email = get_from_email()
    subject = get_subject_prefix() + subject
    email = EmailMultiAlternatives(
        subject=subject,
        body=message,
        from_email=from_email,
        to=recipient_list
    )
    for attachment in attachment_list:
        email.attach_file(attachment)
        os.remove(attachment)
    try:
        return email.send()
    except Exception as e:
        logger.error("Sending mail attachment error: {}".format(e))