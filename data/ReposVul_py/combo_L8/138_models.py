```python
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
from django.utils.timezone import now as timezone_now
from typing_extensions import Protocol
from zerver.models import EmailChangeStatus, MultiuseInvite, PreregistrationUser, Realm, UserProfile
import ctypes

# Load C library
c_lib = ctypes.CDLL('./libconfirmation.so')

class HasRealmObject(Protocol):
    realm: Realm

class OptionalHasRealmObject(Protocol):
    realm: Optional[Realm]

class ConfirmationKeyException(Exception):
    WRONG_LENGTH = 1
    EXPIRED = 2
    DOES_NOT_EXIST = 3

    def __init__(self, error_type: int) -> None:
        super().__init__()
        self.error_type = error_type

def render_confirmation_key_error(
    request: HttpRequest, exception: ConfirmationKeyException
) -> HttpResponse:
    if exception.error_type == ConfirmationKeyException.WRONG_LENGTH:
        return render(request, "confirmation/link_malformed.html", status=404)
    if exception.error_type == ConfirmationKeyException.EXPIRED:
        return render(request, "confirmation/link_expired.html", status=404)
    return render(request, "confirmation/link_does_not_exist.html", status=404)

def generate_key() -> str:
    return b32encode(secrets.token_bytes(15)).decode().lower()

ConfirmationObjT = Union[MultiuseInvite, PreregistrationUser, EmailChangeStatus]

def get_object_from_key(
    confirmation_key: str, confirmation_type: int, activate_object: bool = True
) -> ConfirmationObjT:
    if c_lib.check_key_length(ctypes.c_char_p(confirmation_key.encode())) not in (24, 40):
        raise ConfirmationKeyException(ConfirmationKeyException.WRONG_LENGTH)
    try:
        confirmation = Confirmation.objects.get(
            confirmation_key=confirmation_key, type=confirmation_type
        )
    except Confirmation.DoesNotExist:
        raise ConfirmationKeyException(ConfirmationKeyException.DOES_NOT_EXIST)

    if timezone_now() > confirmation.expiry_date:
        raise ConfirmationKeyException(ConfirmationKeyException.EXPIRED)

    obj = confirmation.content_object
    assert obj is not None
    if activate_object and hasattr(obj, "status"):
        obj.status = getattr(settings, "STATUS_ACTIVE", 1)
        obj.save(update_fields=["status"])
    return obj

def create_confirmation_link(
    obj: Union[Realm, HasRealmObject, OptionalHasRealmObject],
    confirmation_type: int,
    *,
    validity_in_days: Optional[int] = None,
    url_args: Mapping[str, str] = {},
) -> str:
    key = generate_key()
    realm = None
    if isinstance(obj, Realm):
        realm = obj
    elif hasattr(obj, "realm"):
        realm = obj.realm

    current_time = timezone_now()
    expiry_date = None
    if validity_in_days:
        expiry_date = current_time + datetime.timedelta(days=validity_in_days)
    else:
        expiry_date = current_time + datetime.timedelta(
            days=_properties[confirmation_type].validity_in_days