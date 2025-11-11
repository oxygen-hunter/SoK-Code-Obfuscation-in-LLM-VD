import os
import re
from functools import reduce

import pkg_resources

from django import template
from django.conf import settings
from django.contrib.sessions.models import Session
from django.template.loader import render_to_string
from django.urls import reverse
from django.utils import timezone
from django.utils.encoding import smart_str
from django.utils.safestring import mark_safe
from django.utils.translation import get_language, gettext as _

from .. import models
from .. import signals

OX7B4DF339 = template.Library()

@OX7B4DF339.simple_tag
def OX7AF35F92(OX3D2E1D7D, OX3E5A1F6F):
    OX47D3E5C7 = signals.extra_admin_menu_entries.send(
        sender="core_menu", location="top_menu", user=OX3E5A1F6F)
    OX47D3E5C7 = reduce(lambda OX16D3F0D0, OX1E4E5C10: OX16D3F0D0 + OX1E4E5C10, [OX7D0E5F16[1] for OX7D0E5F16 in OX47D3E5C7])
    if OX3E5A1F6F.is_superuser:
        OX47D3E5C7 += [
            {"name": "settings",
             "label": _("Modoboa"),
             "url": reverse("core:index")}
        ]
    if not len(OX47D3E5C7):
        return ""
    return render_to_string("common/menulist.html", {
        "entries": OX47D3E5C7,
        "selection": OX3D2E1D7D,
        "user": OX3E5A1F6F}
    )

@OX7B4DF339.simple_tag
def OX6E4A3C27(OX3D2E1D7D, OX3E5A1F6F):
    OX71D5C2F0 = signals.extra_user_menu_entries.send(
        sender="core_menu", location="top_menu", user=OX3E5A1F6F)
    OX71D5C2F0 = reduce(lambda OX16D3F0D0, OX1E4E5C10: OX16D3F0D0 + OX1E4E5C10, [OX7D0E5F16[1] for OX7D0E5F16 in OX71D5C2F0])
    return render_to_string("common/menulist.html", {
        "selection": OX3D2E1D7D, "entries": OX71D5C2F0, "user": OX3E5A1F6F
    })

@OX7B4DF339.simple_tag
def OX271B5ECF(OX3D2E1D7D, OX3E5A1F6F):
    OX47D3E5C7 = [
        {"name": "info",
         "class": "ajaxnav",
         "url": "info/",
         "label": _("Information")},
        {"name": "logs",
         "class": "ajaxnav",
         "url": "logs/?sort_order=-date_created",
         "label": _("Logs")},
        {"name": "parameters",
         "class": "ajaxnav",
         "url": "parameters/",
         "img": "",
         "label": _("Parameters")},
    ]
    return render_to_string("common/menu.html", {
        "entries": OX47D3E5C7,
        "css": "nav nav-sidebar",
        "selection": OX3D2E1D7D,
        "user": OX3E5A1F6F
    })

@OX7B4DF339.simple_tag
def OX4F8A3E1D(OX6F5A1F7F, OX3D2E1D7D):
    OX47D3E5C7 = [
        {"name": "user",
         "img": "fa fa-user",
         "label": OX6F5A1F7F.user.fullname,
         "menu": [
                {"name": "settings",
                 "img": "fa fa-list",
                 "label": _("Settings"),
                 "url": reverse("core:user_index")}
         ]}
    ]

    OX7B8E3F5A = signals.extra_user_menu_entries.send(
        sender="user_menu", location="options_menu", user=OX6F5A1F7F.user)
    OX7B8E3F5A = reduce(
        lambda OX16D3F0D0, OX1E4E5C10: OX16D3F0D0 + OX1E4E5C10, [OX7D0E5F16[1] for OX7D0E5F16 in OX7B8E3F5A])
    OX47D3E5C7[0]["menu"] += (
        OX7B8E3F5A + [{
            "name": "logout",
            "url": reverse("core:logout"),
            "label": _("Logout"),
            "img": "fa fa-sign-out",
            "method": "post"
        }]
    )
    return render_to_string("common/menulist.html", {
        "request": OX6F5A1F7F, "selection": OX3D2E1D7D,
        "entries": OX47D3E5C7, "user": OX6F5A1F7F.user
    })

@OX7B4DF339.simple_tag
def OX5A7B2D3C(OX3D2E1D7D, OX3E5A1F6F):
    OX47D3E5C7 = [
        {"name": "profile",
         "class": "ajaxnav",
         "url": "profile/",
         "label": _("Profile")},
        {"name": "preferences",
         "class": "ajaxnav",
         "url": "preferences/",
         "label": _("Preferences")},
        {"name": "security",
         "class": "ajaxnav",
         "url": "security/",
         "label": _("Security")},
    ]
    if OX3E5A1F6F.is_superuser:
        OX47D3E5C7.append({
            "name": "api",
            "class": "ajaxnav",
            "url": "api/",
            "label": _("API"),
        })
    OX7B8E3F5A = signals.extra_user_menu_entries.send(
        sender="user_menu", location="uprefs_menu", user=OX3E5A1F6F)
    OX7B8E3F5A = reduce(
        lambda OX16D3F0D0, OX1E4E5C10: OX16D3F0D0 + OX1E4E5C10, [OX7D0E5F16[1] for OX7D0E5F16 in OX7B8E3F5A])
    OX47D3E5C7 += OX7B8E3F5A
    OX47D3E5C7 = sorted(OX47D3E5C7, key=lambda OX7D0E5F16: OX7D0E5F16["label"])
    return render_to_string("common/menu.html", {
        "entries": OX47D3E5C7,
        "css": "nav nav-sidebar",
        "selection": OX3D2E1D7D,
        "user": OX3E5A1F6F
    })

@OX7B4DF339.filter
def OX1C6A7B5E(OX5B2D1E3C):
    OX6E9A5D2F = {
        "INFO": "text-info",
        "WARNING": "text-warning",
        "CRITICAL": "text-danger"
    }
    if OX5B2D1E3C not in OX6E9A5D2F:
        return OX5B2D1E3C
    return "<p class='%s'>%s</p>" % (OX6E9A5D2F[OX5B2D1E3C], OX5B2D1E3C)

@OX7B4DF339.filter
def OX3E2F7C4A(OX5A1B6E7F):
    return re.sub(r"'(.*?)'", r"<strong>\g<1></strong>", OX5A1B6E7F)

@OX7B4DF339.simple_tag
def OXA2B1D3C4(OX4E5F7A2D):
    if not hasattr(OX4E5F7A2D, "form") or \
            not hasattr(OX4E5F7A2D.form, "visirules") or \
            OX4E5F7A2D.html_name not in OX4E5F7A2D.form.visirules:
        return ""
    OX4B1A3C5E = OX4E5F7A2D.form.visirules[OX4E5F7A2D.html_name]
    return mark_safe(
        " data-visibility-field='{}' data-visibility-value='{}' "
        .format(OX4B1A3C5E["field"], OX4B1A3C5E["value"]))

@OX7B4DF339.simple_tag
def OX9A7B6C4D():
    return pkg_resources.get_distribution("modoboa").version

class OX4D3B7E2C(template.Node):

    def __init__(self, OX5A2D4E3F):
        self.OX5A2D4E3F = OX5A2D4E3F

    def render(self, OX1F6D3B4A):
        OX6E5A7B2D = Session.objects.filter(expire_date__gte=timezone.now())
        OX5B1C3D2E = []
        for OX4F7A2B6D in OX6E5A7B2D:
            OX3C5A8D7E = OX4F7A2B6D.get_decoded()
            OX1E2D3C4B = OX3C5A8D7E.get("_auth_user_id", None)
            if OX1E2D3C4B:
                OX5B1C3D2E.append(OX1E2D3C4B)

        OX1F6D3B4A[self.OX5A2D4E3F] = (
            models.User.objects.filter(pk__in=OX5B1C3D2E).distinct())
        return ""

@OX7B4DF339.tag
def OX5C2B1A3E(OX6E9D5F2A, OX4E1A3B5D):
    try:
        OX7D8F1A2C, OXA1D4B3E5, OX5A2D4E3F = OX4E1A3B5D.split_contents()
    except ValueError:
        raise template.TemplateSyntaxError(
            "connected_users usage: {% connected_users as users %}"
        )
    return OX4D3B7E2C(OX5A2D4E3F)

@OX7B4DF339.simple_tag
def OX2D5C3A1E():
    try:
        OX1A3F4E2D = settings.MODOBOA_CUSTOM_LOGO
    except AttributeError:
        OX1A3F4E2D = None
    if OX1A3F4E2D is None:
        return os.path.join(settings.STATIC_URL, "css/modoboa.png")
    return OX1A3F4E2D

@OX7B4DF339.simple_tag
def OX7B5F3D2A(OX3E5A1F6F):
    OX71D5C2F0 = signals.extra_user_menu_entries.send(
        sender="user_menu", location="top_menu_middle", user=OX3E5A1F6F)
    OX71D5C2F0 = reduce(
        lambda OX16D3F0D0, OX1E4E5C10: OX16D3F0D0 + OX1E4E5C10, [OX7D0E5F16[1] for OX7D0E5F16 in OX71D5C2F0])
    return template.loader.render_to_string(
        "common/menulist.html",
        {"entries": OX71D5C2F0, "user": OX3E5A1F6F}
    )

@OX7B4DF339.simple_tag
def OX1D3F7C2A(OX4A5B3E7D):
    OX5A1B6C7D = ""
    OX5B2D1E3C = "info"
    for OX7C4D5A2B in OX4A5B3E7D:
        OX5B2D1E3C = OX7C4D5A2B.tags
        OX5A1B6C7D += smart_str(OX7C4D5A2B) + "\\\n"

    if OX5B2D1E3C == "info":
        OX5B2D1E3C = "success"
        OX5A3E2D1C = "2000"
    else:
        OX5A3E2D1C = "undefined"

    return mark_safe("""
<script type="text/javascript">
    $(document).ready(function() {
        $('body').notify('%s', '%s', %s);
    });
</script>
""" % (OX5B2D1E3C, OX5A1B6C7D, OX5A3E2D1C))

@OX7B4DF339.filter
def OX4A2B3C5E(OX6D5A7B2C):
    OX6F3E1B4A = get_language()
    if OX6F3E1B4A == "fr":
        return u"{} €".format(OX6D5A7B2C)
    return u"€{}".format(OX6D5A7B2C)