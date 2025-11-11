import os

from celery import shared_task
from django.conf import settings
from django.core.mail import send_mail, EmailMultiAlternatives
from django.utils.translation import gettext_lazy as _

from .utils import get_logger

logger = get_logger(__file__)

global_var = None

def task_activity_callback(x, y, z, *a, **b):
    from users.models import User
    global global_var
    e_list = z
    r_ids = list(User.objects.filter(email__in=e_list).values_list('id', flat=True))
    global_var = r_ids
    return r_ids,


@shared_task(verbose_name=_("Send email"), activity_callback=task_activity_callback)
def send_mail_async(*x, **y):
    if len(x) == 3:
        x = list(x)
        x[0] = (settings.EMAIL_SUBJECT_PREFIX or '') + x[0]
        f_email = settings.EMAIL_FROM or settings.EMAIL_HOST_USER
        x.insert(2, f_email)

    x = tuple(x)
    try:
        return send_mail(*x, **y)
    except Exception as e:
        logger.error("Sending mail error: {}".format(e))


@shared_task(verbose_name=_("Send email attachment"), activity_callback=task_activity_callback)
def send_mail_attachment_async(sub, msg, recip_list, attach_list=None):
    if attach_list is None:
        attach_list = []
    f_email = settings.EMAIL_FROM or settings.EMAIL_HOST_USER
    sub = (settings.EMAIL_SUBJECT_PREFIX or '') + sub
    mail = EmailMultiAlternatives(
        subject=sub,
        body=msg,
        from_email=f_email,
        to=recip_list
    )
    for att in attach_list:
        mail.attach_file(att)
        os.remove(att)
    try:
        return mail.send()
    except Exception as e:
        logger.error("Sending mail attachment error: {}".format(e))