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

def run_dispatcher(dispatcher, *args):
    while True:
        if dispatcher == 0:
            entries = signals.extra_admin_menu_entries.send(
                sender="core_menu", location="top_menu", user=args[1])
            entries = reduce(lambda a, b: a + b, [entry[1] for entry in entries])
            if args[1].is_superuser:
                entries += [
                    {"name": "settings",
                     "label": _("Modoboa"),
                     "url": reverse("core:index")}
                ]
            if not len(entries):
                return ""
            return render_to_string("common/menulist.html", {
                "entries": entries,
                "selection": args[0],
                "user": args[1]}
            )
        elif dispatcher == 1:
            menu = signals.extra_user_menu_entries.send(
                sender="core_menu", location="top_menu", user=args[1])
            menu = reduce(lambda a, b: a + b, [entry[1] for entry in menu])
            return render_to_string("common/menulist.html", {
                "selection": args[0], "entries": menu, "user": args[1]
            })
        elif dispatcher == 2:
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
                "entries": entries,
                "css": "nav nav-sidebar",
                "selection": args[0],
                "user": args[1]
            })
        elif dispatcher == 3:
            entries = [
                {"name": "user",
                 "img": "fa fa-user",
                 "label": args[0].user.fullname,
                 "menu": [
                        {"name": "settings",
                         "img": "fa fa-list",
                         "label": _("Settings"),
                         "url": reverse("core:user_index")}
                 ]}
            ]

            extra_entries = signals.extra_user_menu_entries.send(
                sender="user_menu", location="options_menu", user=args[0].user)
            extra_entries = reduce(
                lambda a, b: a + b, [entry[1] for entry in extra_entries])
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
                "request": args[0], "selection": args[1],
                "entries": entries, "user": args[0].user
            })
        elif dispatcher == 4:
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
            if args[1].is_superuser:
                entries.append({
                    "name": "api",
                    "class": "ajaxnav",
                    "url": "api/",
                    "label": _("API"),
                })
            extra_entries = signals.extra_user_menu_entries.send(
                sender="user_menu", location="uprefs_menu", user=args[1])
            extra_entries = reduce(
                lambda a, b: a + b, [entry[1] for entry in extra_entries])
            entries += extra_entries
            entries = sorted(entries, key=lambda e: e["label"])
            return render_to_string("common/menu.html", {
                "entries": entries,
                "css": "nav nav-sidebar",
                "selection": args[0],
                "user": args[1]
            })
        elif dispatcher == 5:
            level, classes = args[0], {
                "INFO": "text-info",
                "WARNING": "text-warning",
                "CRITICAL": "text-danger"
            }
            if level not in classes:
                return level
            return "<p class='%s'>%s</p>" % (classes[level], level)
        elif dispatcher == 6:
            return re.sub(r"'(.*?)'", r"<strong>\g<1></strong>", args[0])
        elif dispatcher == 7:
            if not hasattr(args[0], "form") or \
                    not hasattr(args[0].form, "visirules") or \
                    args[0].html_name not in args[0].form.visirules:
                return ""
            rule = args[0].form.visirules[args[0].html_name]
            return mark_safe(
                " data-visibility-field='{}' data-visibility-value='{}' "
                .format(rule["field"], rule["value"]))
        elif dispatcher == 8:
            return pkg_resources.get_distribution("modoboa").version
        elif dispatcher == 9:
            sessions = Session.objects.filter(expire_date__gte=timezone.now())
            uid_list = []
            for session in sessions:
                data = session.get_decoded()
                uid = data.get("_auth_user_id", None)
                if uid:
                    uid_list.append(uid)
            args[1][args[0]] = (
                models.User.objects.filter(pk__in=uid_list).distinct())
            return ""
        elif dispatcher == 10:
            try:
                logo = settings.MODOBOA_CUSTOM_LOGO
            except AttributeError:
                logo = None
            if logo is None:
                return os.path.join(settings.STATIC_URL, "css/modoboa.png")
            return logo
        elif dispatcher == 11:
            menu = signals.extra_user_menu_entries.send(
                sender="user_menu", location="top_menu_middle", user=args[0])
            menu = reduce(
                lambda a, b: a + b, [entry[1] for entry in menu])
            return template.loader.render_to_string(
                "common/menulist.html",
                {"entries": menu, "user": args[0]}
            )
        elif dispatcher == 12:
            text = ""
            level = "info"
            for m in args[0]:
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
        elif dispatcher == 13:
            lang = get_language()
            if lang == "fr":
                return u"{} €".format(args[0])
            return u"€{}".format(args[0])
        else:
            break

@register.simple_tag
def core_menu(selection, user):
    return run_dispatcher(0, selection, user)

@register.simple_tag
def extensions_menu(selection, user):
    return run_dispatcher(1, selection, user)

@register.simple_tag
def admin_menu(selection, user):
    return run_dispatcher(2, selection, user)

@register.simple_tag
def user_menu(request, selection):
    return run_dispatcher(3, request, selection)

@register.simple_tag
def uprefs_menu(selection, user):
    return run_dispatcher(4, selection, user)

@register.filter
def colorize_level(level):
    return run_dispatcher(5, level)

@register.filter
def tohtml(message):
    return run_dispatcher(6, message)

@register.simple_tag
def visirule(field):
    return run_dispatcher(7, field)

@register.simple_tag
def get_version():
    return run_dispatcher(8)

class ConnectedUsers(template.Node):

    def __init__(self, varname):
        self.varname = varname

    def render(self, context):
        return run_dispatcher(9, self.varname, context)

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
    return run_dispatcher(10)

@register.simple_tag
def load_optionalmenu(user):
    return run_dispatcher(11, user)

@register.simple_tag
def display_messages(msgs):
    return run_dispatcher(12, msgs)

@register.filter
def currencyfmt(amount):
    return run_dispatcher(13, amount)