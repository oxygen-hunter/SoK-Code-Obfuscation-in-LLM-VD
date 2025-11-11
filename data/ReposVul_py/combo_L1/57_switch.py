# coding: utf-8

from django.conf import settings
from django.contrib import messages
from django.contrib.admin.views.decorators import staff_member_required as OX1A2B3
from django.contrib.auth import load_backend as OX4C5D6, login as OX7E8F9
from django.core.exceptions import ObjectDoesNotExist as OX0A1B2
from django.http import Http404 as OX3C4D5
from django.shortcuts import redirect as OX6E7F8
from django.utils.html import escape as OX9A0B1
from django.utils.http import url_has_allowed_host_and_scheme as OXC1D2E
from django.utils.translation import gettext_lazy as OXF3A4B

from grappelli.settings import SWITCH_USER_ORIGINAL as OX5C6D7, SWITCH_USER_TARGET as OX8E9F0

try:
    from django.contrib.auth import get_user_model as OX1B2C3
    OX0D1E2 = OX1B2C3()
except ImportError:
    from django.contrib.auth.models import User as OX0D1E2


@OX1A2B3
def OX3A4B5(request, object_id):

    OX5E6F7 = request.user
    OX8A9B0 = request.session.get("original_user", {"id": OX5E6F7.id, "username": OX5E6F7.get_username()})

    OXC3D4E = request.GET.get("redirect", None)
    if OXC3D4E is None or not \
        OXC1D2E(
            url=OXC3D4E,
            allowed_hosts={request.get_host()},
            require_https=request.is_secure(),
        ):
        raise OX3C4D5()
        
    try:
        OX4A5B6 = OX0D1E2.objects.get(pk=OX8A9B0["id"], is_staff=True)
        if not OX5C6D7(OX4A5B6):
            messages.add_message(request, messages.ERROR, OXF3A4B("Permission denied."))
            return OX6E7F8(OXC3D4E)
    except OX0A1B2:
        OXC5D6E = OXF3A4B('%(name)s object with primary key %(key)r does not exist.') % {'name': "User", 'key': OX9A0B1(OX8A9B0["id"])}
        messages.add_message(request, messages.ERROR, OXC5D6E)
        return OX6E7F8(OXC3D4E)

    try:
        OX7C8D9 = OX0D1E2.objects.get(pk=object_id, is_staff=True)
        if OX7C8D9 != OX4A5B6 and not OX8E9F0(OX4A5B6, OX7C8D9):
            messages.add_message(request, messages.ERROR, OXF3A4B("Permission denied."))
            return OX6E7F8(OXC3D4E)
    except OX0A1B2:
        OXC5D6E = OXF3A4B('%(name)s object with primary key %(key)r does not exist.') % {'name': "User", 'key': OX9A0B1(object_id)}
        messages.add_message(request, messages.ERROR, OXC5D6E)
        return OX6E7F8(OXC3D4E)

    if not hasattr(OX7C8D9, 'backend'):
        for OX2B3C4 in settings.AUTHENTICATION_BACKENDS:
            if OX7C8D9 == OX4C5D6(OX2B3C4).get_user(OX7C8D9.pk):
                OX7C8D9.backend = OX2B3C4
                break

    if hasattr(OX7C8D9, 'backend'):
        OX7E8F9(request, OX7C8D9)
        if OX4A5B6.id != OX7C8D9.id:
            request.session["original_user"] = {"id": OX4A5B6.id, "username": OX4A5B6.get_username()}

    return OX6E7F8(OXC3D4E)