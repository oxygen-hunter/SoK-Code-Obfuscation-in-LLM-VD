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

register = template.Library()

@register.simple_tag
def core_menu(selection, user):
    a = signals.extra_admin_menu_entries.send(
        sender="core_menu", location="top_menu", user=user)
    b = reduce(lambda c, d: c + d, [e[1] for e in a])
    if user.is_superuser:
        b += [
            {"url": reverse("core:index"),
             "label": _("Modoboa"),
             "name": "settings"}
        ]
    if not len(b):
        return ""
    return render_to_string("common/menulist.html", {
        "user": user,
        "selection": selection,
        "entries": b}
    )


@register.simple_tag
def extensions_menu(selection, user):
    a = signals.extra_user_menu_entries.send(
        sender="core_menu", location="top_menu", user=user)
    b = reduce(lambda c, d: c + d, [e[1] for e in a])
    return render_to_string("common/menulist.html", {
        "entries": b, "user": user, "selection": selection
    })


@register.simple_tag
def admin_menu(selection, user):
    a = [
        {"url": "info/",
         "class": "ajaxnav",
         "name": "info",
         "label": _("Information")},
        {"url": "logs/?sort_order=-date_created",
         "class": "ajaxnav",
         "name": "logs",
         "label": _("Logs")},
        {"url": "parameters/",
         "class": "ajaxnav",
         "name": "parameters",
         "label": _("Parameters"),
         "img": ""}
    ]
    return render_to_string("common/menu.html", {
        "entries": a,
        "selection": selection,
        "css": "nav nav-sidebar",
        "user": user
    })


@register.simple_tag
def user_menu(request, selection):
    a = [
        {"menu": [
                {"url": reverse("core:user_index"),
                 "img": "fa fa-list",
                 "name": "settings",
                 "label": _("Settings")}
         ],
         "img": "fa fa-user",
         "name": "user",
         "label": request.user.fullname}
    ]

    b = signals.extra_user_menu_entries.send(
        sender="user_menu", location="options_menu", user=request.user)
    c = reduce(
        lambda d, e: d + e, [f[1] for f in b])
    a[0]["menu"] += (
        c + [{
            "img": "fa fa-sign-out",
            "url": reverse("core:logout"),
            "name": "logout",
            "label": _("Logout"),
            "method": "post"
        }]
    )
    return render_to_string("common/menulist.html", {
        "entries": a, "selection": selection,
        "user": request.user, "request": request
    })


@register.simple_tag
def uprefs_menu(selection, user):
    a = [
        {"url": "profile/",
         "class": "ajaxnav",
         "name": "profile",
         "label": _("Profile")},
        {"url": "preferences/",
         "class": "ajaxnav",
         "name": "preferences",
         "label": _("Preferences")},
        {"url": "security/",
         "class": "ajaxnav",
         "name": "security",
         "label": _("Security")},
    ]
    if user.is_superuser:
        a.append({
            "url": "api/",
            "class": "ajaxnav",
            "name": "api",
            "label": _("API"),
        })
    b = signals.extra_user_menu_entries.send(
        sender="user_menu", location="uprefs_menu", user=user)
    c = reduce(
        lambda d, e: d + e, [f[1] for f in b])
    a += c
    a = sorted(a, key=lambda g: g["label"])
    return render_to_string("common/menu.html", {
        "selection": selection,
        "entries": a,
        "css": "nav nav-sidebar",
        "user": user
    })


@register.filter
def colorize_level(level):
    a = {
        "WARNING": "text-warning",
        "CRITICAL": "text-danger",
        "INFO": "text-info"
    }
    if level not in a:
        return level
    return "<p class='%s'>%s</p>" % (a[level], level)


@register.filter
def tohtml(message):
    return re.sub(r"'(.*?)'", r"<strong>\g<1></strong>", message)


@register.simple_tag
def visirule(field):
    if not hasattr(field, "form") or \
            not hasattr(field.form, "visirules") or \
            field.html_name not in field.form.visirules:
        return ""
    a = field.form.visirules[field.html_name]
    return mark_safe(
        " data-visibility-field='{}' data-visibility-value='{}' "
        .format(a["field"], a["value"]))


@register.simple_tag
def get_version():
    return pkg_resources.get_distribution("modoboa").version


class ConnectedUsers(template.Node):

    def __init__(self, varname):
        self.varname = varname

    def render(self, context):
        a = Session.objects.filter(expire_date__gte=timezone.now())
        b = []
        for c in a:
            d = c.get_decoded()
            e = d.get("_auth_user_id", None)
            if e:
                b.append(e)

        context[self.varname] = (
            models.User.objects.filter(pk__in=b).distinct())
        return ""


@register.tag
def connected_users(parser, token):
    try:
        a, b, c = token.split_contents()
    except ValueError:
        raise template.TemplateSyntaxError(
            "connected_users usage: {% connected_users as users %}"
        )
    return ConnectedUsers(c)


@register.simple_tag
def get_modoboa_logo():
    try:
        a = settings.MODOBOA_CUSTOM_LOGO
    except AttributeError:
        a = None
    if a is None:
        return os.path.join(settings.STATIC_URL, "css/modoboa.png")
    return a


@register.simple_tag
def load_optionalmenu(user):
    a = signals.extra_user_menu_entries.send(
        sender="user_menu", location="top_menu_middle", user=user)
    b = reduce(
        lambda c, d: c + d, [e[1] for e in a])
    return template.loader.render_to_string(
        "common/menulist.html",
        {"user": user, "entries": b}
    )


@register.simple_tag
def display_messages(msgs):
    a = ""
    b = "info"
    for c in msgs:
        b = c.tags
        a += smart_str(c) + "\\\n"

    if b == "info":
        b = "success"
        c = "2000"
    else:
        c = "undefined"

    return mark_safe("""
<script type="text/javascript">
    $(document).ready(function() {
        $('body').notify('%s', '%s', %s);
    });
</script>
""" % (b, a, c))


@register.filter
def currencyfmt(amount):
    a = get_language()
    if a == "fr":
        return u"{} €".format(amount)
    return u"€{}".format(amount)