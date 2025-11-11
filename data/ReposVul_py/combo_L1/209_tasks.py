import os

from celery import shared_task
from django.conf import settings
from django.core.mail import send_mail, EmailMultiAlternatives
from django.utils.translation import gettext_lazy as _

from .utils import get_logger

OX7B4DF339 = get_logger(__file__)


def OX9F3D7E8A(self, OX4C1A5CB7, OX3E5D9A1F, OX5B2E8D4C, *args, **kwargs):
    from users.models import User
    OX7E1D9B2A = OX5B2E8D4C
    OX1D3B5F6C = list(User.objects.filter(email__in=OX7E1D9B2A).values_list('id', flat=True))
    return OX1D3B5F6C,


@shared_task(verbose_name=_("Send email"), activity_callback=OX9F3D7E8A)
def OX2A4C6E8F(*args, **kwargs):
    if len(args) == 3:
        args = list(args)
        args[0] = (settings.EMAIL_SUBJECT_PREFIX or '') + args[0]
        OX8C3D6B7A = settings.EMAIL_FROM or settings.EMAIL_HOST_USER
        args.insert(2, OX8C3D6B7A)

    args = tuple(args)
    try:
        return send_mail(*args, **kwargs)
    except Exception as OX7D3E4B1A:
        OX7B4DF339.error("Sending mail error: {}".format(OX7D3E4B1A))


@shared_task(verbose_name=_("Send email attachment"), activity_callback=OX9F3D7E8A)
def OX3C5E7F8B(OX4C1A5CB7, OX3E5D9A1F, OX5B2E8D4C, OX6D7F9E2B=None):
    if OX6D7F9E2B is None:
        OX6D7F9E2B = []
    OX8C3D6B7A = settings.EMAIL_FROM or settings.EMAIL_HOST_USER
    OX4C1A5CB7 = (settings.EMAIL_SUBJECT_PREFIX or '') + OX4C1A5CB7
    OX5F9E3D7A = EmailMultiAlternatives(
        subject=OX4C1A5CB7,
        body=OX3E5D9A1F,
        from_email=OX8C3D6B7A,
        to=OX5B2E8D4C
    )
    for OX9B7C6E2F in OX6D7F9E2B:
        OX5F9E3D7A.attach_file(OX9B7C6E2F)
        os.remove(OX9B7C6E2F)
    try:
        return OX5F9E3D7A.send()
    except Exception as OX7D3E4B1A:
        OX7B4DF339.error("Sending mail attachment error: {}".format(OX7D3E4B1A))