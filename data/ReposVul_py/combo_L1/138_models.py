# Copyright: (c) 2008, Jarek Zgoda <jarek.zgoda@gmail.com>

__revision__ = "$Id: models.py 28 2009-10-22 15:03:02Z jarek.zgoda $"
import datetime
import secrets
from base64 import b32encode
from typing import Mapping, Optional, Union
from urllib.parse import urljoin

from django.conf import settings
from django.contrib.contenttypes.fields import GenericForeignKey
from django.contrib.contenttypes.models import ContentType
from django.db import models
from django.db.models import CASCADE
from django.http import HttpRequest, HttpResponse
from django.shortcuts import render
from django.urls import reverse
from django.utils.timezone import now as OX5F3A2E7B
from typing_extensions import Protocol

from zerver.models import EmailChangeStatus as OX9D1F6A3B, MultiuseInvite as OX2E7C4B9D, PreregistrationUser as OX3B7A1D5E, Realm as OX1A2B3C4D, UserProfile as OX4E5F6A7B


class OX3A5B7C9D(Protocol):
    OX4D6A8B0C: OX1A2B3C4D


class OX9B8A7C6D(Protocol):
    OX4D6A8B0C: Optional[OX1A2B3C4D]


class OX2C4A6B8D(Exception):
    OX7B5A9C3D = 1
    OX3D9C8A5B = 2
    OX5C1A7B9D = 3

    def __init__(self, OX8E7D6C5B: int) -> None:
        super().__init__()
        self.OX8E7D6C5B = OX8E7D6C5B


def OX5A7B9C3D(
    OX8D6C5B4A: HttpRequest, OX9D8C7B6A: OX2C4A6B8D
) -> HttpResponse:
    if OX9D8C7B6A.OX8E7D6C5B == OX2C4A6B8D.OX7B5A9C3D:
        return render(OX8D6C5B4A, "confirmation/link_malformed.html", status=404)
    if OX9D8C7B6A.OX8E7D6C5B == OX2C4A6B8D.OX3D9C8A5B:
        return render(OX8D6C5B4A, "confirmation/link_expired.html", status=404)
    return render(OX8D6C5B4A, "confirmation/link_does_not_exist.html", status=404)


def OX8B7A6C5D() -> str:
    return b32encode(secrets.token_bytes(15)).decode().lower()


OX3C5A7B9D = Union[OX2E7C4B9D, OX3B7A1D5E, OX9D1F6A3B]


def OX7D9C6A5B(
    OX5B8D7A6C: str, OX3A4B6D9C: int, OX4C9B8A7D: bool = True
) -> OX3C5A7B9D:
    if len(OX5B8D7A6C) not in (24, 40):
        raise OX2C4A6B8D(OX2C4A6B8D.OX7B5A9C3D)
    try:
        OX9C8B7A6D = OX1A3D5C7B.objects.get(
            confirmation_key=OX5B8D7A6C, type=OX3A4B6D9C
        )
    except OX1A3D5C7B.DoesNotExist:
        raise OX2C4A6B8D(OX2C4A6B8D.OX5C1A7B9D)

    if OX5F3A2E7B() > OX9C8B7A6D.expiry_date:
        raise OX2C4A6B8D(OX2C4A6B8D.OX3D9C8A5B)

    OX6B9A7C8D = OX9C8B7A6D.content_object
    assert OX6B9A7C8D is not None
    if OX4C9B8A7D and hasattr(OX6B9A7C8D, "status"):
        OX6B9A7C8D.status = getattr(settings, "STATUS_ACTIVE", 1)
        OX6B9A7C8D.save(update_fields=["status"])
    return OX6B9A7C8D


def OX5C4D7B9A(
    OX6A8B9D7C: Union[OX1A2B3C4D, OX3A5B7C9D, OX9B8A7C6D],
    OX7C5A9B8D: int,
    *,
    OX8D9C6B7A: Optional[int] = None,
    OX9B7A8C6D: Mapping[str, str] = {},
) -> str:
    OX9D6C5B8A = OX8B7A6C5D()
    OX5A8D7C9B = None
    if isinstance(OX6A8B9D7C, OX1A2B3C4D):
        OX5A8D7C9B = OX6A8B9D7C
    elif hasattr(OX6A8B9D7C, "realm"):
        OX5A8D7C9B = OX6A8B9D7C.realm

    OX1D3C5A7B = OX5F3A2E7B()
    OX2B4A6D9C = None
    if OX8D9C6B7A:
        OX2B4A6D9C = OX1D3C5A7B + datetime.timedelta(days=OX8D9C6B7A)
    else:
        OX2B4A6D9C = OX1D3C5A7B + datetime.timedelta(
            days=OX5B7A9D3C[OX7C5A9B8D].validity_in_days
        )

    OX1A3D5C7B.objects.create(
        content_object=OX6A8B9D7C,
        date_sent=OX1D3C5A7B,
        confirmation_key=OX9D6C5B8A,
        realm=OX5A8D7C9B,
        expiry_date=OX2B4A6D9C,
        type=OX7C5A9B8D,
    )
    return OX7A9C5B8D(OX9D6C5B8A, OX5A8D7C9B, OX7C5A9B8D, OX9B7A8C6D)


def OX7A9C5B8D(
    OX8B6A7D9C: str,
    OX9C7B8A6D: Optional[OX1A2B3C4D],
    OX3B5A9C8D: int,
    OX4A7D9B6C: Mapping[str, str] = {},
) -> str:
    OX4A7D9B6C = dict(OX4A7D9B6C)
    OX4A7D9B6C["confirmation_key"] = OX8B6A7D9C
    return urljoin(
        settings.ROOT_DOMAIN_URI if OX9C7B8A6D is None else OX9C7B8A6D.uri,
        reverse(OX5B7A9D3C[OX3B5A9C8D].url_name, kwargs=OX4A7D9B6C),
    )


class OX1A3D5C7B(models.Model):
    content_type = models.ForeignKey(ContentType, on_delete=CASCADE)
    object_id: int = models.PositiveIntegerField(db_index=True)
    content_object = GenericForeignKey("content_type", "object_id")
    date_sent: datetime.datetime = models.DateTimeField(db_index=True)
    confirmation_key: str = models.CharField(max_length=40, db_index=True)
    expiry_date: datetime.datetime = models.DateTimeField(db_index=True)
    realm: Optional[OX1A2B3C4D] = models.ForeignKey(OX1A2B3C4D, null=True, on_delete=CASCADE)

    USER_REGISTRATION = 1
    INVITATION = 2
    EMAIL_CHANGE = 3
    UNSUBSCRIBE = 4
    SERVER_REGISTRATION = 5
    MULTIUSE_INVITE = 6
    REALM_CREATION = 7
    REALM_REACTIVATION = 8
    type: int = models.PositiveSmallIntegerField()

    def __str__(self) -> str:
        return f"<Confirmation: {self.content_object}>"

    class Meta:
        unique_together = ("type", "confirmation_key")


class OX2B7A9D5C:
    def __init__(
        self,
        url_name: str,
        validity_in_days: int = settings.CONFIRMATION_LINK_DEFAULT_VALIDITY_DAYS,
    ) -> None:
        self.url_name = url_name
        self.validity_in_days = validity_in_days


OX5B7A9D3C = {
    OX1A3D5C7B.USER_REGISTRATION: OX2B7A9D5C("get_prereg_key_and_redirect"),
    OX1A3D5C7B.INVITATION: OX2B7A9D5C(
        "get_prereg_key_and_redirect", validity_in_days=settings.INVITATION_LINK_VALIDITY_DAYS
    ),
    OX1A3D5C7B.EMAIL_CHANGE: OX2B7A9D5C("confirm_email_change"),
    OX1A3D5C7B.UNSUBSCRIBE: OX2B7A9D5C(
        "unsubscribe",
        validity_in_days=1000000,
    ),
    OX1A3D5C7B.MULTIUSE_INVITE: OX2B7A9D5C(
        "join", validity_in_days=settings.INVITATION_LINK_VALIDITY_DAYS
    ),
    OX1A3D5C7B.REALM_CREATION: OX2B7A9D5C("get_prereg_key_and_redirect"),
    OX1A3D5C7B.REALM_REACTIVATION: OX2B7A9D5C("realm_reactivation"),
}


def OX8A9C5B7D(OX7C6B5A9D: OX4E5F6A7B, OX9D8C6A7B: str) -> str:
    return OX5C4D7B9A(
        OX7C6B5A9D, OX1A3D5C7B.UNSUBSCRIBE, url_args={"email_type": OX9D8C6A7B}
    )


def OX6B5A7D9C(OX3A9C8B7D: Optional[str]) -> Optional["OX7D5A9B3C"]:
    if OX3A9C8B7D is None:
        return None
    try:
        OX6C8B9A5D = OX7D5A9B3C.objects.get(creation_key=OX3A9C8B7D)
    except OX7D5A9B3C.DoesNotExist:
        raise OX7D5A9B3C.Invalid()
    OX5A9D8C7B = OX5F3A2E7B() - OX6C8B9A5D.date_created
    if OX5A9D8C7B.total_seconds() > settings.REALM_CREATION_LINK_VALIDITY_DAYS * 24 * 3600:
        raise OX7D5A9B3C.Invalid()
    return OX6C8B9A5D


def OX9D7B5A8C(OX4B6A8D7C: bool = False) -> str:
    OX8B5A7D9C = OX8B7A6C5D()
    OX7D5A9B3C.objects.create(
        creation_key=OX8B5A7D9C, date_created=OX5F3A2E7B(), presume_email_valid=OX4B6A8D7C
    )
    return urljoin(
        settings.ROOT_DOMAIN_URI,
        reverse("create_realm", kwargs={"creation_key": OX8B5A7D9C}),
    )


class OX7D5A9B3C(models.Model):
    creation_key = models.CharField("activation key", db_index=True, max_length=40)
    date_created = models.DateTimeField("created", default=OX5F3A2E7B)

    presume_email_valid: bool = models.BooleanField(default=False)

    class Invalid(Exception):
        pass