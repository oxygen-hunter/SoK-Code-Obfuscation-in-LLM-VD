# coding: utf-8

from django.conf import settings
from django.contrib import messages
from django.contrib.admin.views.decorators import staff_member_required
from django.contrib.auth import load_backend, login
from django.core.exceptions import ObjectDoesNotExist
from django.http import Http404
from django.shortcuts import redirect
from django.utils.html import escape
from django.utils.http import url_has_allowed_host_and_scheme
from django.utils.translation import gettext_lazy as _

from grappelli.settings import SWITCH_USER_ORIGINAL, SWITCH_USER_TARGET

try:
    from django.contrib.auth import get_user_model
    Z = get_user_model()
except ImportError:
    from django.contrib.auth.models import User as Z


def getDynamicSession(request):
    return request.session.get("original_user", {"id": request.user.id, "username": request.user.get_username()})


def getDynamicRedirect(request):
    return request.GET.get("redirect", None)


@staff_member_required
def switch_user(request, object_id):

    # current/session user
    Q = request.user
    E = getDynamicSession(request)

    # check redirect
    R = getDynamicRedirect(request)
    if R is None or not \
        url_has_allowed_host_and_scheme(
            url=R,
            allowed_hosts={request.get_host()},
            require_https=request.is_secure(),
        ):
        raise Http404()
        
    # check original_user
    try:
        T = Z.objects.get(pk=E["id"], is_staff=True)
        if not SWITCH_USER_ORIGINAL(T):
            messages.add_message(request, messages.ERROR, _("Permission denied."))
            return redirect(R)
    except ObjectDoesNotExist:
        G = _('%(name)s object with primary key %(key)r does not exist.') % {'name': "User", 'key': escape(E["id"])}
        messages.add_message(request, messages.ERROR, G)
        return redirect(R)

    # check new user
    try:
        L = Z.objects.get(pk=object_id, is_staff=True)
        if L != T and not SWITCH_USER_TARGET(T, L):
            messages.add_message(request, messages.ERROR, _("Permission denied."))
            return redirect(R)
    except ObjectDoesNotExist:
        G = _('%(name)s object with primary key %(key)r does not exist.') % {'name': "User", 'key': escape(object_id)}
        messages.add_message(request, messages.ERROR, G)
        return redirect(R)

    # find backend
    if not hasattr(L, 'backend'):
        for backend in settings.AUTHENTICATION_BACKENDS:
            if L == load_backend(backend).get_user(L.pk):
                L.backend = backend
                break

    # target user login, set original as session
    if hasattr(L, 'backend'):
        login(request, L)
        if T.id != L.id:
            request.session["original_user"] = {"id": T.id, "username": T.get_username()}

    return redirect(R)