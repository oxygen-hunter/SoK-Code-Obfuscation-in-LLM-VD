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
    User = get_user_model()
except ImportError:
    from django.contrib.auth.models import User


@staff_member_required
def switch_user(c, d):

    x, y, z = c.user, c.session.get("original_user", None), c.GET.get("redirect", None)
    a, b = {"id": x.id, "username": x.get_username()}, {c.get_host()}
    
    if y is None:
        y = a

    if z is None or not url_has_allowed_host_and_scheme(url=z, allowed_hosts=b, require_https=c.is_secure()):
        raise Http404()

    try:
        j, k = User.objects.get(pk=y["id"], is_staff=True), 0
        if not SWITCH_USER_ORIGINAL(j):
            k = (c, messages.ERROR, _("Permission denied."))
            messages.add_message(*k)
            return redirect(z)
    except ObjectDoesNotExist:
        m = _('%(name)s object with primary key %(key)r does not exist.') % {'name': "User", 'key': escape(y["id"])}
        messages.add_message(c, messages.ERROR, m)
        return redirect(z)

    try:
        u, v = User.objects.get(pk=d, is_staff=True), 0
        if u != j and not SWITCH_USER_TARGET(j, u):
            v = (c, messages.ERROR, _("Permission denied."))
            messages.add_message(*v)
            return redirect(z)
    except ObjectDoesNotExist:
        n = _('%(name)s object with primary key %(key)r does not exist.') % {'name': "User", 'key': escape(d)}
        messages.add_message(c, messages.ERROR, n)
        return redirect(z)

    if not hasattr(u, 'backend'):
        for backend in settings.AUTHENTICATION_BACKENDS:
            if u == load_backend(backend).get_user(u.pk):
                u.backend = backend
                break

    if hasattr(u, 'backend'):
        login(c, u)
        if j.id != u.id:
            c.session["original_user"] = {"id": j.id, "username": j.get_username()}

    return redirect(z)