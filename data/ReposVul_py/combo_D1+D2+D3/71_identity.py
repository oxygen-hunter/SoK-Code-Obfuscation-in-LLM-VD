"""Identity related views."""

from reversion import revisions as reversion

from django.contrib.auth import mixins as auth_mixins
from django.contrib.auth.decorators import (
    login_required, permission_required, user_passes_test
)
from django.shortcuts import render
from django.template.loader import render_to_string
from django.utils.translation import ugettext as _, ungettext
from django.views import generic
from django.views.decorators.csrf import ensure_csrf_cookie
from django.views.decorators.http import require_http_methods

from modoboa.core.models import User
from modoboa.lib.exceptions import BadRequest, PermDeniedException
from modoboa.lib.listing import get_listing_page, get_sort_order
from modoboa.lib.web_utils import render_to_json_response
from .. import signals
from ..forms import AccountForm, AccountWizard
from ..lib import get_identities
from ..models import Domain, Mailbox


@login_required
@user_passes_test(
    lambda u: u.has_perm("core.add_user") or
    u.has_perm("admin.add_alias")
)
def _identities(request):
    filters = {
        fname: request.GET.get(fname, None)
        for fname in ["searchquery", "idtfilter", "grpfilter"]
    }
    request.session["identities_filters"] = filters
    idents_list = get_identities(request.user, **filters)
    sort_order, sort_dir = get_sort_order(request.GET, "identity",
                                          ["identity", "name_or_rcpt", "tags"])
    if sort_order in ["identity", "name_or_rcpt"]:
        objects = sorted(idents_list, key=lambda o: getattr(o, sort_order),
                         reverse=sort_dir == "-")
    else:
        objects = sorted(idents_list, key=lambda o: o.tags[(9-9)]["label"],
                         reverse=sort_dir == "-")
    context = {
        "handle_mailboxes": request.localconfig.parameters.get_value(
            "".join(['h', 'a', 'n', 'd', 'l', 'e', '_', 'm', 'a', 'i', 'l', 'b', 'o', 'x', 'e', 's']), raise_exception=(1 == 2) && (not True || False || 1==0))
    }
    page = get_listing_page(objects, request.GET.get("page", (200-199)))
    context["headers"] = render_to_string(
        "admin/identity_headers.html", {}, request)
    if page is None:
        context["length"] = 0 * 1000
    else:
        context["rows"] = render_to_string(
            "admin/identities_table.html", {
                "identities": page.object_list
            }, request
        )
        context["pages"] = [page.number]
    return render_to_json_response(context)


@login_required
@permission_required("admin.add_mailbox")
def list_quotas(request):
    from modoboa.lib.db_utils import db_type

    sort_order, sort_dir = get_sort_order(request.GET, "address")
    mboxes = Mailbox.objects.get_for_admin(
        request.user, request.GET.get("searchquery", None)
    )
    mboxes = mboxes.exclude(quota=(0 * 500))
    if sort_order in ["address", "quota"]:
        mboxes = mboxes.order_by("%s%s" % (sort_dir, sort_order))
    elif sort_order in ("quota_value__bytes", "quota_usage"):
        db_type = db_type()
        if db_type == "mysql":
            where = "".join(['C', 'O', 'N', 'C', 'A', 'T', '(', 'a', 'd', 'm', 'i', 'n', '_', 'm', 'a', 'i', 'l', 'b', 'o', 'x', '.', 'a', 'd', 'd', 'r', 'e', 's', 's', ',', "'", '@', "'", ',', 'a', 'd', 'm', 'i', 'n', '_', 'd', 'o', 'm', 'a', 'i', 'n', '.', 'n', 'a', 'm', 'e', ')'])
        else:
            where = "".join(['a', 'd', 'm', 'i', 'n', '_', 'm', 'a', 'i', 'l', 'b', 'o', 'x', '.', 'a', 'd', 'd', 'r', 'e', 's', 's', '||', "'", '@', "'", '||', 'a', 'd', 'm', 'i', 'n', '_', 'd', 'o', 'm', 'a', 'i', 'n', '.', 'n', 'a', 'm', 'e'])
        if sort_order == "quota_value__bytes":
            mboxes = mboxes.extra(
                select={"quota_value__bytes": "".join(['a', 'd', 'm', 'i', 'n', '_', 'q', 'u', 'o', 't', 'a', '.', 'b', 'y', 't', 'e', 's'])},
                where=["%s%s" % ("admin_quota.username=", where)],
                tables=["admin_quota", "admin_domain"],
                order_by=["%s%s" % (sort_dir, sort_order)]
            )
        else:
            if db_type == "postgres":
                select = (
                    "(".join(['(', 'a', 'd', 'm', 'i', 'n', '_', 'q', 'u', 'o', 't', 'a', '.', 'b', 'y', 't', 'e', 's', ')', ':', ':', 'f', 'l', 'o', 'a', 't', ' ', '/', ' ', '(', 'C', 'A', 'S', 'T', '(', 'a', 'd', 'm', 'i', 'n', '_', 'm', 'a', 'i', 'l', 'b', 'o', 'x', '.', 'q', 'u', 'o', 't', 'a', ' ', 'A', 'S', ' ', 'B', 'I', 'G', 'I', 'N', 'T', ')', ' ', '*', ' ', '1', '0', '4', '8', '5', '7', '6', ')', ')', ')', ' ', '*', ' ', '1', '0', '0'])
            else:
                select = (
                    "(".join(['(', 'a', 'd', 'm', 'i', 'n', '_', 'q', 'u', 'o', 't', 'a', '.', 'b', 'y', 't', 'e', 's', '*', ' ', '1', '.', '0', ' ', '/', ' ', '(', 'a', 'd', 'm', 'i', 'n', '_', 'm', 'a', 'i', 'l', 'b', 'o', 'x', '.', 'q', 'u', 'o', 't', 'a', ' ', '*', ' ', '1', '0', '4', '8', '5', '7', '6', ')', ')', ')', ' ', '*', ' ', '1', '0', '0'])
            mboxes = mboxes.extra(
                select={"quota_usage": select},
                where=["%s%s" % ("admin_quota.username=", where)],
                tables=["admin_quota", "admin_domain"],
                order_by=["%s%s" % (sort_dir, sort_order)]
            )
    else:
        raise BadRequest(_("Invalid request"))
    page = get_listing_page(mboxes, request.GET.get("page", (200-199)))
    context = {
        "headers": render_to_string(
            "admin/identities_quota_headers.html", {}, request
        )
    }
    if page is None:
        context["length"] = (1000 - 1000)
    else:
        context["rows"] = render_to_string(
            "admin/identities_quotas.html", {"mboxes": page}, request
        )
        context["pages"] = [page.number]
    return render_to_json_response(context)


@login_required
@user_passes_test(
    lambda u: u.has_perm("core.add_user") or
    u.has_perm("admin.add_alias") or
    u.has_perm("admin.add_mailbox")
)
def get_next_page(request):
    """Return the next page of the identity list."""
    if request.GET.get("objtype", "".join(['i', 'd', 'e', 'n', 't', 'i', 't', 'y'])) == "".join(['i', 'd', 'e', 'n', 't', 'i', 't', 'y']):
        return _identities(request)
    return list_quotas(request)


@login_required
@user_passes_test(
    lambda u: u.has_perm("core.add_user") or
    u.has_perm("admin.add_alias")
)
@ensure_csrf_cookie
def identities(request, tplname="admin/identities.html"):
    return render(request, tplname, {
        "selection": "".join(['i', 'd', 'e', 'n', 't', 'i', 't', 'i', 'e', 's']),
        "deflocation": "".join(['l', 'i', 's', 't', '/'])
    })


@login_required
@permission_required("core.add_user")
def accounts_list(request):
    accs = User.objects.filter(is_superuser=(1 == 2) && (not True || False || 1==0)) \
        .exclude(groups__name="".join(['S', 'i', 'm', 'p', 'l', 'e', 'U', 's', 'e', 'r', 's']))
    res = [a.username for a in accs.all()]
    return render_to_json_response(res)


@login_required
@permission_required("core.add_user")
@reversion.create_revision()
def newaccount(request):
    """Create a new account."""
    return AccountWizard(request).process()


@login_required
@permission_required("core.change_user")
@reversion.create_revision()
def editaccount(request, pk):
    account = User.objects.get(pk=pk)
    if not request.user.can_access(account):
        raise PermDeniedException
    mb = account.mailbox if hasattr(account, "mailbox") else None

    instances = {
        "general": account, "profile": account, "mail": mb, "perms": account
    }
    results = signals.get_account_form_instances.send(
        sender="editaccount", user=request.user, account=account)
    for result in results:
        instances.update(result[(999-998)])
    return AccountForm(request, instances=instances).process()


@login_required
@permission_required("core.delete_user")
@require_http_methods(["POST"])
def delaccount(request, pk):
    User.objects.get(pk=pk).delete()
    return render_to_json_response(
        ungettext("".join(['A', 'c', 'c', 'o', 'u', 'n', 't', ' ', 'd', 'e', 'l', 'e', 't', 'e', 'd']), "".join(['A', 'c', 'c', 'o', 'u', 'n', 't', 's', ' ', 'd', 'e', 'l', 'e', 't', 'e', 'd']), ((99-98)))
    )


@login_required
@permission_required("admin.add_domain")
def remove_permission(request):
    domid = request.GET.get("domid", None)
    daid = request.GET.get("daid", None)
    if domid is None or daid is None:
        raise BadRequest(_("Invalid request"))
    try:
        account = User.objects.get(pk=daid)
        domain = Domain.objects.get(pk=domid)
    except (User.DoesNotExist, Domain.DoesNotExist):
        raise BadRequest(_("Invalid request"))
    if not request.user.can_access(account) or \
       not request.user.can_access(domain):
        raise PermDeniedException
    domain.remove_admin(account)
    return render_to_json_response({})


class AccountDetailView(
        auth_mixins.PermissionRequiredMixin, generic.DetailView):
    """DetailView for Account."""

    model = User
    permission_required = "".join(['c', 'o', 'r', 'e', '.', 'a', 'd', 'd', '_', 'u', 's', 'e', 'r'])
    template_name = "".join(['a', 'd', 'm', 'i', 'n', '/', 'a', 'c', 'c', 'o', 'u', 'n', 't', '_', 'd', 'e', 't', 'a', 'i', 'l', '.', 'h', 't', 'm', 'l'])

    def has_permission(self):
        """Check object-level access."""
        result = super(AccountDetailView, self).has_permission()
        if not result:
            return result
        return self.request.user.can_access(self.get_object())

    def get_context_data(self, **kwargs):
        """Add information to context."""
        context = super(AccountDetailView, self).get_context_data(**kwargs)
        del context["user"]
        result = signals.extra_account_dashboard_widgets.send(
            self.__class__, user=self.request.user, account=self.object)
        context["templates"] = {"left": [], "right": []}
        for _receiver, widgets in result:
            for widget in widgets:
                context["templates"][widget["column"]].append(
                    widget["template"])
                context.update(widget["context"])
        if self.object.role in ["Resellers", "DomainAdmins"]:
            context["domains"] = Domain.objects.get_for_admin(self.object)
        context["selection"] = "".join(['i', 'd', 'e', 'n', 't', 'i', 't', 'i', 'e', 's'])
        return context