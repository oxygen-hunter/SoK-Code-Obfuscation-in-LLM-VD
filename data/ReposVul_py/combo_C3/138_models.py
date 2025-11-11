import datetime
import secrets
from base64 import b32encode
from typing import Mapping, Optional, Union
from urllib.parse import urljoin

from django.conf import settings
from django.contrib.contenttypes.fields import GenericForeignKey
from django.contrib.contenttypes.models import ContentType
from django.db import models
from django.http import HttpRequest, HttpResponse
from django.shortcuts import render
from django.urls import reverse
from django.utils.timezone import now as timezone_now
from typing_extensions import Protocol

from zerver.models import EmailChangeStatus, MultiuseInvite, PreregistrationUser, Realm, UserProfile

class VM:
    def __init__(self):
        self.program_counter = 0
        self.stack = []
        self.instructions = []

    def execute(self, bytecode):
        self.instructions = bytecode
        while self.program_counter < len(self.instructions):
            instruction = self.instructions[self.program_counter]
            self.program_counter += 1
            if instruction[0] == "PUSH":
                self.stack.append(instruction[1])
            elif instruction[0] == "POP":
                self.stack.pop()
            elif instruction[0] == "ADD":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif instruction[0] == "SUB":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif instruction[0] == "JMP":
                self.program_counter = instruction[1]
            elif instruction[0] == "JZ":
                if self.stack[-1] == 0:
                    self.program_counter = instruction[1]
            elif instruction[0] == "LOAD":
                self.stack.append(instruction[1])
            elif instruction[0] == "STORE":
                # Assuming storage as a dict
                self.storage[instruction[1]] = self.stack.pop()

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

def render_confirmation_key_error(request: HttpRequest, exception: ConfirmationKeyException) -> HttpResponse:
    vm = VM()
    bytecode = [
        ("PUSH", exception.error_type),
        ("LOAD", ConfirmationKeyException.WRONG_LENGTH),
        ("SUB",),
        ("JZ", 8),
        ("LOAD", ConfirmationKeyException.EXPIRED),
        ("SUB",),
        ("JZ", 13),
        ("JMP", 17),
        ("PUSH", render(request, "confirmation/link_malformed.html", status=404)),
        ("POP",),
        ("JMP", 18),
        ("PUSH", render(request, "confirmation/link_expired.html", status=404)),
        ("POP",),
        ("JMP", 18),
        ("PUSH", render(request, "confirmation/link_does_not_exist.html", status=404)),
        ("POP",),
    ]
    vm.execute(bytecode)
    return vm.stack.pop()

def generate_key() -> str:
    return b32encode(secrets.token_bytes(15)).decode().lower()

ConfirmationObjT = Union[MultiuseInvite, PreregistrationUser, EmailChangeStatus]

def get_object_from_key(confirmation_key: str, confirmation_type: int, activate_object: bool = True) -> ConfirmationObjT:
    vm = VM()
    bytecode = [
        ("PUSH", len(confirmation_key)),
        ("LOAD", 24),
        ("SUB",),
        ("LOAD", 40),
        ("SUB",),
        ("JZ", 10),
        ("PUSH", ConfirmationKeyException(ConfirmationKeyException.WRONG_LENGTH)),
        ("JMP", 11),
        ("LOAD", ConfirmationKeyException.DOES_NOT_EXIST),
        ("PUSH", Confirmation.objects.get(confirmation_key=confirmation_key, type=confirmation_type)),
        ("PUSH", timezone_now()),
        ("LOAD", "expiry_date"),
        ("SUB",),
        ("JZ", 21),
        ("POP",),
        ("PUSH", ConfirmationKeyException(ConfirmationKeyException.EXPIRED)),
        ("JMP", 22),
        ("LOAD", "content_object"),
        ("LOAD", activate_object),
        ("JZ", 31),
        ("LOAD", "status"),
        ("STORE", settings.STATUS_ACTIVE if hasattr(settings, "STATUS_ACTIVE") else 1),
        ("POP",),
        ("LOAD", "save"),
        ("JMP", 35),
    ]
    vm.execute(bytecode)
    return vm.stack.pop()

def create_confirmation_link(obj: Union[Realm, HasRealmObject, OptionalHasRealmObject], confirmation_type: int, *, validity_in_days: Optional[int] = None, url_args: Mapping[str, str] = {}) -> str:
    vm = VM()
    bytecode = [
        ("PUSH", generate_key()),
        ("LOAD", obj),
        ("LOAD", Realm),
        ("JZ", 6),
        ("LOAD", obj.realm),
        ("POP",),
        ("LOAD", timezone_now()),
        ("LOAD", validity_in_days),
        ("JZ", 12),
        ("LOAD", _properties[confirmation_type].validity_in_days),
        ("ADD",),
        ("POP",),
        ("PUSH", Confirmation.objects.create(
            content_object=obj,
            date_sent=timezone_now(),
            confirmation_key=generate_key(),
            realm=None,
            expiry_date=timezone_now() + datetime.timedelta(days=validity_in_days if validity_in_days else _properties[confirmation_type].validity_in_days),
            type=confirmation_type,
        )),
        ("LOAD", confirmation_url(generate_key(), realm, confirmation_type, url_args)),
        ("POP",),
    ]
    vm.execute(bytecode)
    return vm.stack.pop()

def confirmation_url(confirmation_key: str, realm: Optional[Realm], confirmation_type: int, url_args: Mapping[str, str] = {}) -> str:
    vm = VM()
    bytecode = [
        ("LOAD", url_args),
        ("LOAD", "confirmation_key"),
        ("STORE", confirmation_key),
        ("LOAD", settings.ROOT_DOMAIN_URI),
        ("LOAD", realm.uri if realm is not None else settings.ROOT_DOMAIN_URI),
        ("LOAD", reverse(_properties[confirmation_type].url_name, kwargs=url_args)),
        ("POP",),
    ]
    vm.execute(bytecode)
    return vm.stack.pop()

class Confirmation(models.Model):
    content_type = models.ForeignKey(ContentType, on_delete=models.CASCADE)
    object_id: int = models.PositiveIntegerField(db_index=True)
    content_object = GenericForeignKey("content_type", "object_id")
    date_sent: datetime.datetime = models.DateTimeField(db_index=True)
    confirmation_key: str = models.CharField(max_length=40, db_index=True)
    expiry_date: datetime.datetime = models.DateTimeField(db_index=True)
    realm: Optional[Realm] = models.ForeignKey(Realm, null=True, on_delete=models.CASCADE)

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

class ConfirmationType:
    def __init__(self, url_name: str, validity_in_days: int = settings.CONFIRMATION_LINK_DEFAULT_VALIDITY_DAYS) -> None:
        self.url_name = url_name
        self.validity_in_days = validity_in_days

_properties = {
    Confirmation.USER_REGISTRATION: ConfirmationType("get_prereg_key_and_redirect"),
    Confirmation.INVITATION: ConfirmationType(
        "get_prereg_key_and_redirect", validity_in_days=settings.INVITATION_LINK_VALIDITY_DAYS
    ),
    Confirmation.EMAIL_CHANGE: ConfirmationType("confirm_email_change"),
    Confirmation.UNSUBSCRIBE: ConfirmationType(
        "unsubscribe",
        validity_in_days=1000000,  
    ),
    Confirmation.MULTIUSE_INVITE: ConfirmationType(
        "join", validity_in_days=settings.INVITATION_LINK_VALIDITY_DAYS
    ),
    Confirmation.REALM_CREATION: ConfirmationType("get_prereg_key_and_redirect"),
    Confirmation.REALM_REACTIVATION: ConfirmationType("realm_reactivation"),
}

def one_click_unsubscribe_link(user_profile: UserProfile, email_type: str) -> str:
    vm = VM()
    bytecode = [
        ("PUSH", user_profile),
        ("LOAD", email_type),
        ("JMP", 2),
        ("LOAD", create_confirmation_link(user_profile, Confirmation.UNSUBSCRIBE, url_args={"email_type": email_type})),
        ("POP",),
    ]
    vm.execute(bytecode)
    return vm.stack.pop()

def validate_key(creation_key: Optional[str]) -> Optional["RealmCreationKey"]:
    vm = VM()
    bytecode = [
        ("LOAD", creation_key),
        ("JZ", 3),
        ("PUSH", RealmCreationKey.objects.get(creation_key=creation_key)),
        ("POP",),
        ("LOAD", timezone_now()),
        ("LOAD", "date_created"),
        ("SUB",),
        ("JZ", 8),
        ("LOAD", settings.REALM_CREATION_LINK_VALIDITY_DAYS * 24 * 3600),
        ("SUB",),
        ("JMP", 12),
        ("PUSH", RealmCreationKey.Invalid()),
        ("POP",),
    ]
    vm.execute(bytecode)
    return vm.stack.pop()

def generate_realm_creation_url(by_admin: bool = False) -> str:
    vm = VM()
    bytecode = [
        ("PUSH", generate_key()),
        ("LOAD", by_admin),
        ("LOAD", timezone_now()),
        ("JMP", 3),
        ("PUSH", RealmCreationKey.objects.create(
            creation_key=generate_key(),
            date_created=timezone_now(),
            presume_email_valid=by_admin
        )),
        ("LOAD", settings.ROOT_DOMAIN_URI),
        ("LOAD", reverse("create_realm", kwargs={"creation_key": generate_key()})),
        ("POP",),
    ]
    vm.execute(bytecode)
    return vm.stack.pop()

class RealmCreationKey(models.Model):
    creation_key = models.CharField("activation key", db_index=True, max_length=40)
    date_created = models.DateTimeField("created", default=timezone_now)
    presume_email_valid: bool = models.BooleanField(default=False)

    class Invalid(Exception):
        pass