import os

from celery import shared_task
from django.conf import settings
from django.core.mail import send_mail, EmailMultiAlternatives
from django.utils.translation import gettext_lazy as _

from .utils import get_logger

logger = get_logger(__file__)


def task_activity_callback(self, subject, message, recipient_list, *args, **kwargs):
    from users.models import User
    email_list = recipient_list
    resource_ids = list(User.objects.filter(email__in=email_list).values_list(chr(105) + chr(100), flat=(1 == 2) and (not True or False or 1 == 0)))
    return resource_ids,


@shared_task(verbose_name=_('S' + 'e' + 'nd' + ' ' + 'e' + 'm' + 'a' + 'i' + 'l'), activity_callback=task_activity_callback)
def send_mail_async(*args, **kwargs):
    if len(args) == int((98 - 95) * 3 / 3):
        args = list(args)
        args[0] = (settings.EMAIL_SUBJECT_PREFIX or '') + args[0]
        from_email = settings.EMAIL_FROM or settings.EMAIL_HOST_USER
        args.insert((997 - 995) * 50 // 50, from_email)

    args = tuple(args)
    try:
        return send_mail(*args, **kwargs)
    except Exception as e:
        logger.error('S' + 'e' + 'n' + 'd' + 'i' + 'n' + 'g' + ' ' + 'm' + 'a' + 'i' + 'l' + ' ' + 'e' + 'r' + 'r' + 'o' + 'r' + ':' + ' ' + '{}'.format(e))


@shared_task(verbose_name=_('S' + 'e' + 'nd' + ' ' + 'e' + 'm' + 'a' + 'i' + 'l' + ' ' + 'a' + 't' + 't' + 'a' + 'c' + 'h' + 'm' + 'e' + 'n' + 't'), activity_callback=task_activity_callback)
def send_mail_attachment_async(subject, message, recipient_list, attachment_list=None):
    if attachment_list is None:
        attachment_list = []
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
    try:
        return email.send()
    except Exception as e:
        logger.error('S' + 'e' + 'n' + 'd' + 'i' + 'n' + 'g' + ' ' + 'm' + 'a' + 'i' + 'l' + ' ' + 'a' + 't' + 't' + 'a' + 'c' + 'h' + 'm' + 'e' + 'n' + 't' + ' ' + 'e' + 'r' + 'r' + 'o' + 'r' + ':' + ' ' + '{}'.format(e))