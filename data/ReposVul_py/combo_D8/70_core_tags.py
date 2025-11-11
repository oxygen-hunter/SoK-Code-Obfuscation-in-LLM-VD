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

def getSelection(selection):
    return selection

def getUser(user):
    return user

def getSender(sender):
    return sender

def getLocation(location):
    return location

def getMenuEntries(entries):
    return entries

def getEntries(entries):
    return entries

def getMenu(menu):
    return menu

def getRequest(request):
    return request

def getField(field):
    return field

def getRule(rule):
    return rule

def getLogo(logo):
    return logo

def getMsgs(msgs):
    return msgs

def getAmount(amount):
    return amount

@register.simple_tag
def core_menu(selection, user):
    entries = signals.extra_admin_menu_entries.send(
        sender=getSender("core_menu"), location=getLocation("top_menu"), user=getUser(user))
    entries = reduce(lambda a, b: a + b, [entry[1] for entry in getMenuEntries(entries)])
    if user.is_superuser:
        entries += [
            {"name": "settings",
             "label": _("Modoboa"),
             "url": reverse("core:index")}
        ]
    if not len(getEntries(entries)):
        return ""
    return render_to_string("common/menulist.html", {
        "entries": getEntries(entries),
        "selection": getSelection(selection),
        "user": getUser(user)}
    )


@register.simple_tag
def extensions_menu(selection, user):
    menu = signals.extra_user_menu_entries.send(
        sender=getSender("core_menu"), location=getLocation("top_menu"), user=getUser(user))
    menu = reduce(lambda a, b: a + b, [entry[1] for entry in getMenu(menu)])
    return render_to_string("common/menulist.html", {
        "selection": getSelection(selection), "entries": getEntries(menu), "user": getUser(user)
    })


@register.simple_tag
def admin_menu(selection, user):
    entries = [
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
        "entries": getEntries(entries),
        "css": "nav nav-sidebar",
        "selection": getSelection(selection),
        "user": getUser(user)
    })


@register.simple_tag
def user_menu(request, selection):
    entries = [
        {"name": "user",
         "img": "fa fa-user",
         "label": request.user.fullname,
         "menu": [
                {"name": "settings",
                 "img": "fa fa-list",
                 "label": _("Settings"),
                 "url": reverse("core:user_index")}
         ]}
    ]

    extra_entries = signals.extra_user_menu_entries.send(
        sender=getSender("user_menu"), location=getLocation("options_menu"), user=request.user)
    extra_entries = reduce(
        lambda a, b: a + b, [entry[1] for entry in getMenuEntries(extra_entries)])
    entries[0]["menu"] += (
        extra_entries + [{
            "name": "logout",
            "url": reverse("core:logout"),
            "label": _("Logout"),
            "img": "fa fa-sign-out",
            "method": "post"
        }]
    )
    return render_to_string("common/menulist.html", {
        "request": getRequest(request), "selection": getSelection(selection),
        "entries": getEntries(entries), "user": request.user
    })


@register.simple_tag
def uprefs_menu(selection, user):
    entries = [
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
    if user.is_superuser:
        entries.append({
            "name": "api",
            "class": "ajaxnav",
            "url": "api/",
            "label": _("API"),
        })
    extra_entries = signals.extra_user_menu_entries.send(
        sender=getSender("user_menu"), location=getLocation("uprefs_menu"), user=getUser(user))
    extra_entries = reduce(
        lambda a, b: a + b, [entry[1] for entry in getMenuEntries(extra_entries)])
    entries += extra_entries
    entries = sorted(getEntries(entries), key=lambda e: e["label"])
    return render_to_string("common/menu.html", {
        "entries": getEntries(entries),
        "css": "nav nav-sidebar",
        "selection": getSelection(selection),
        "user": getUser(user)
    })


@register.filter
def colorize_level(level):
    classes = {
        "INFO": "text-info",
        "WARNING": "text-warning",
        "CRITICAL": "text-danger"
    }
    if level not in classes:
        return level
    return "<p class='%s'>%s</p>" % (classes[level], level)


@register.filter
def tohtml(message):
    return re.sub(r"'(.*?)'", r"<strong>\g<1></strong>", message)


@register.simple_tag
def visirule(field):
    if not hasattr(getField(field), "form") or \
            not hasattr(getField(field).form, "visirules") or \
            getField(field).html_name not in getField(field).form.visirules:
        return ""
    rule = getField(field).form.visirules[getField(field).html_name]
    return mark_safe(
        " data-visibility-field='{}' data-visibility-value='{}' "
        .format(getRule(rule)["field"], getRule(rule)["value"]))


@register.simple_tag
def get_version():
    return pkg_resources.get_distribution("modoboa").version


class ConnectedUsers(template.Node):

    def __init__(self, varname):
        self.varname = varname

    def render(self, context):
        sessions = Session.objects.filter(expire_date__gte=timezone.now())
        uid_list = []
        for session in sessions:
            data = session.get_decoded()
            uid = data.get("_auth_user_id", None)
            if uid:
                uid_list.append(uid)

        context[self.varname] = (
            models.User.objects.filter(pk__in=uid_list).distinct())
        return ""


@register.tag
def connected_users(parser, token):
    try:
        tag, a, varname = token.split_contents()
    except ValueError:
        raise template.TemplateSyntaxError(
            "connected_users usage: {% connected_users as users %}"
        )
    return ConnectedUsers(varname)


@register.simple_tag
def get_modoboa_logo():
    try:
        logo = settings.MODOBOA_CUSTOM_LOGO
    except AttributeError:
        logo = None
    if getLogo(logo) is None:
        return os.path.join(settings.STATIC_URL, "css/modoboa.png")
    return getLogo(logo)


@register.simple_tag
def load_optionalmenu(user):
    menu = signals.extra_user_menu_entries.send(
        sender=getSender("user_menu"), location=getLocation("top_menu_middle"), user=getUser(user))
    menu = reduce(
        lambda a, b: a + b, [entry[1] for entry in getMenu(menu)])
    return template.loader.render_to_string(
        "common/menulist.html",
        {"entries": getEntries(menu), "user": getUser(user)}
    )


@register.simple_tag
def display_messages(msgs):
    text = ""
    level = "info"
    for m in getMsgs(msgs):
        level = m.tags
        text += smart_str(m) + "\\\n"

    if level == "info":
        level = "success"
        timeout = "2000"
    else:
        timeout = "undefined"

    return mark_safe("""
<script type="text/javascript">
    $(document).ready(function() {
        $('body').notify('%s', '%s', %s);
    });
</script>
""" % (level, text, timeout))


@register.filter
def currencyfmt(amount):
    lang = get_language()
    if lang == "fr":
        return u"{} €".format(getAmount(amount))
    return u"€{}".format(getAmount(amount))