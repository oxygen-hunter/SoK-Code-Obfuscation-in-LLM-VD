from reversion import revisions as OX2F4D4C2D

from django.contrib.auth import mixins as OX6DC1D5A9
from django.contrib.auth.decorators import (
    login_required as OX5D8F8B6A, permission_required as OX5E1D3E4C, user_passes_test as OX7B8C3F2E
)
from django.shortcuts import render as OX3C7D1A9F
from django.template.loader import render_to_string as OX7F3E1B2D
from django.utils.translation import ugettext as OX5A9C7B3E, ungettext as OX8D2A3F4B
from django.views import generic as OX4E5A9B3C
from django.views.decorators.csrf import ensure_csrf_cookie as OX3D7A1F9B
from django.views.decorators.http import require_http_methods as OX7B4F9C2A

from modoboa.core.models import User as OX9C3A1B4D
from modoboa.lib.exceptions import BadRequest as OX2A4B5C7D, PermDeniedException as OX8B9D3F1C
from modoboa.lib.listing import get_listing_page as OX3F1D9A6B, get_sort_order as OX4B7C1E2D
from modoboa.lib.web_utils import render_to_json_response as OX7C2D9A3F
from .. import signals as OX5E9D3B1C
from ..forms import AccountForm as OX8C2A4B7D, AccountWizard as OX3D6E9C2B
from ..lib import get_identities as OX1D4B7F9C
from ..models import Domain as OX9F3A2B1D, Mailbox as OX2C7E4D3B


@OX5D8F8B6A
@OX7B8C3F2E(
    lambda OX4A5B6C7D: OX4A5B6C7D.has_perm("core.add_user") or
    OX4A5B6C7D.has_perm("admin.add_alias")
)
def OX5E7A9B3C(OX9D1C3E4F):
    OX4E1D5B7C = {
        OX3A2B5C7D: OX9D1C3E4F.GET.get(OX3A2B5C7D, None)
        for OX3A2B5C7D in ["searchquery", "idtfilter", "grpfilter"]
    }
    OX9D1C3E4F.session["identities_filters"] = OX4E1D5B7C
    OX2A3B4C5D = OX1D4B7F9C(OX9D1C3E4F.user, **OX4E1D5B7C)
    OX8B3C5D1E, OX9A2C6E4F = OX4B7C1E2D(OX9D1C3E4F.GET, "identity",
                                          ["identity", "name_or_rcpt", "tags"])
    if OX8B3C5D1E in ["identity", "name_or_rcpt"]:
        OX3F8D1A2B = sorted(OX2A3B4C5D, key=lambda OX8C9D2A3B: getattr(OX8C9D2A3B, OX8B3C5D1E),
                         reverse=OX9A2C6E4F == "-")
    else:
        OX3F8D1A2B = sorted(OX2A3B4C5D, key=lambda OX8C9D2A3B: OX8C9D2A3B.tags[0]["label"],
                         reverse=OX9A2C6E4F == "-")
    OX7A1B3C5D = {
        "handle_mailboxes": OX9D1C3E4F.localconfig.parameters.get_value(
            "handle_mailboxes", raise_exception=False)
    }
    OX5E9C1A3B = OX3F1D9A6B(OX3F8D1A2B, OX9D1C3E4F.GET.get("page", 1))
    OX7A1B3C5D["headers"] = OX7F3E1B2D(
        "admin/identity_headers.html", {}, OX9D1C3E4F)
    if OX5E9C1A3B is None:
        OX7A1B3C5D["length"] = 0
    else:
        OX7A1B3C5D["rows"] = OX7F3E1B2D(
            "admin/identities_table.html", {
                "identities": OX5E9C1A3B.object_list
            }, OX9D1C3E4F
        )
        OX7A1B3C5D["pages"] = [OX5E9C1A3B.number]
    return OX7C2D9A3F(OX7A1B3C5D)


@OX5D8F8B6A
@OX5E1D3E4C("admin.add_mailbox")
def OX9D8B1C3E(OX4A7E2D9F):
    from modoboa.lib.db_utils import db_type as OX3D5B8C1A

    OX3D7C6B1E, OX2A4E9F3B = OX4B7C1E2D(OX4A7E2D9F.GET, "address")
    OX1B3C5E7D = OX2C7E4D3B.objects.get_for_admin(
        OX4A7E2D9F.user, OX4A7E2D9F.GET.get("searchquery", None)
    )
    OX1B3C5E7D = OX1B3C5E7D.exclude(quota=0)
    if OX3D7C6B1E in ["address", "quota"]:
        OX1B3C5E7D = OX1B3C5E7D.order_by("%s%s" % (OX2A4E9F3B, OX3D7C6B1E))
    elif OX3D7C6B1E in ("quota_value__bytes", "quota_usage"):
        OX3D5B8C1A = OX3D5B8C1A()
        if OX3D5B8C1A == "mysql":
            OX5C7E1A2D = "CONCAT(admin_mailbox.address,'@',admin_domain.name)"
        else:
            OX5C7E1A2D = "admin_mailbox.address||'@'||admin_domain.name"
        if OX3D7C6B1E == "quota_value__bytes":
            OX1B3C5E7D = OX1B3C5E7D.extra(
                select={"quota_value__bytes": "admin_quota.bytes"},
                where=["admin_quota.username=%s" % OX5C7E1A2D],
                tables=["admin_quota", "admin_domain"],
                order_by=["%s%s" % (OX2A4E9F3B, OX3D7C6B1E)]
            )
        else:
            if OX3D5B8C1A == "postgres":
                OX7F1C3D8B = (
                    "(admin_quota.bytes::float / (CAST(admin_mailbox.quota "
                    "AS BIGINT) * 1048576)) * 100"
                )
            else:
                OX7F1C3D8B = (
                    "(admin_quota.bytes * 1.0 / (admin_mailbox.quota "
                    "* 1048576)) * 100"
                )
            OX1B3C5E7D = OX1B3C5E7D.extra(
                select={"quota_usage": OX7F1C3D8B},
                where=["admin_quota.username=%s" % OX5C7E1A2D],
                tables=["admin_quota", "admin_domain"],
                order_by=["%s%s" % (OX2A4E9F3B, OX3D7C6B1E)]
            )
    else:
        raise OX2A4B5C7D(OX5A9C7B3E("Invalid request"))
    OX5C7A1D9B = OX3F1D9A6B(OX1B3C5E7D, OX4A7E2D9F.GET.get("page", 1))
    OX8D9C1A3B = {
        "headers": OX7F3E1B2D(
            "admin/identities_quota_headers.html", {}, OX4A7E2D9F
        )
    }
    if OX5C7A1D9B is None:
        OX8D9C1A3B["length"] = 0
    else:
        OX8D9C1A3B["rows"] = OX7F3E1B2D(
            "admin/identities_quotas.html", {"mboxes": OX5C7A1D9B}, OX4A7E2D9F
        )
        OX8D9C1A3B["pages"] = [OX5C7A1D9B.number]
    return OX7C2D9A3F(OX8D9C1A3B)


@OX5D8F8B6A
@OX7B8C3F2E(
    lambda OX3A5D7E9C: OX3A5D7E9C.has_perm("core.add_user") or
    OX3A5D7E9C.has_perm("admin.add_alias") or
    OX3A5D7E9C.has_perm("admin.add_mailbox")
)
def OX3B2D6F8C(OX6A9C8D3E):
    if OX6A9C8D3E.GET.get("objtype", "identity") == "identity":
        return OX5E7A9B3C(OX6A9C8D3E)
    return OX9D8B1C3E(OX6A9C8D3E)


@OX5D8F8B6A
@OX7B8C3F2E(
    lambda OX5E9C1B2D: OX5E9C1B2D.has_perm("core.add_user") or
    OX5E9C1B2D.has_perm("admin.add_alias")
)
@OX3D7A1F9B
def OX2A9B3C5D(OX4B7C1E2D, OX1F2D3A5B="admin/identities.html"):
    return OX3C7D1A9F(OX4B7C1E2D, OX1F2D3A5B, {
        "selection": "identities",
        "deflocation": "list/"
    })


@OX5D8F8B6A
@OX5E1D3E4C("core.add_user")
def OX1A4C7D9F(OX5B8D3E1C):
    OX8D2B3C5A = OX9C3A1B4D.objects.filter(is_superuser=False) \
        .exclude(groups__name="SimpleUsers")
    OX2A5B7D9C = [OX8B1C3D5E.username for OX8B1C3D5E in OX8D2B3C5A.all()]
    return OX7C2D9A3F(OX2A5B7D9C)


@OX5D8F8B6A
@OX5E1D3E4C("core.add_user")
@OX2F4D4C2D.create_revision()
def OX9E7A3C1B(OX8C9D2A3B):
    return OX3D6E9C2B(OX8C9D2A3B).process()


@OX5D8F8B6A
@OX5E1D3E4C("core.change_user")
@OX2F4D4C2D.create_revision()
def OX6E1C4A9D(OX7B3C5D2A, pk):
    OX9A8B1D3C = OX9C3A1B4D.objects.get(pk=pk)
    if not OX7B3C5D2A.user.can_access(OX9A8B1D3C):
        raise OX8B9D3F1C
    OX7E3D2A5B = OX9A8B1D3C.mailbox if hasattr(OX9A8B1D3C, "mailbox") else None

    OX4B5C7D1E = {
        "general": OX9A8B1D3C, "profile": OX9A8B1D3C, "mail": OX7E3D2A5B, "perms": OX9A8B1D3C
    }
    OX3C6D1A9B = OX5E9D3B1C.get_account_form_instances.send(
        sender="editaccount", user=OX7B3C5D2A.user, account=OX9A8B1D3C)
    for OX1D2E4B3C in OX3C6D1A9B:
        OX4B5C7D1E.update(OX1D2E4B3C[1])
    return OX8C2A4B7D(OX7B3C5D2A, instances=OX4B5C7D1E).process()


@OX5D8F8B6A
@OX5E1D3E4C("core.delete_user")
@OX7B4F9C2A(["POST"])
def OX5E8A1D4C(OX8B3C5D1E, pk):
    OX9C3A1B4D.objects.get(pk=pk).delete()
    return OX7C2D9A3F(
        OX8D2A3F4B("Account deleted", "Accounts deleted", 1)
    )


@OX5D8F8B6A
@OX5E1D3E4C("admin.add_domain")
def OX7C4A1B9D(OX3D9A6E1C):
    OX5B8C3D1E = OX3D9A6E1C.GET.get("domid", None)
    OX2A7E4F1C = OX3D9A6E1C.GET.get("daid", None)
    if OX5B8C3D1E is None or OX2A7E4F1C is None:
        raise OX2A4B5C7D(OX5A9C7B3E("Invalid request"))
    try:
        OX8C9D4A1B = OX9C3A1B4D.objects.get(pk=OX2A7E4F1C)
        OX7B1C5D8E = OX9F3A2B1D.objects.get(pk=OX5B8C3D1E)
    except (OX9C3A1B4D.DoesNotExist, OX9F3A2B1D.DoesNotExist):
        raise OX2A4B5C7D(OX5A9C7B3E("Invalid request"))
    if not OX3D9A6E1C.user.can_access(OX8C9D4A1B) or \
       not OX3D9A6E1C.user.can_access(OX7B1C5D8E):
        raise OX8B9D3F1C
    OX7B1C5D8E.remove_admin(OX8C9D4A1B)
    return OX7C2D9A3F({})


class OX2E9C1D4B(
        OX6DC1D5A9.PermissionRequiredMixin, OX4E5A9B3C.DetailView):

    model = OX9C3A1B4D
    permission_required = "core.add_user"
    template_name = "admin/account_detail.html"

    def OX7D2A3C5B(self):
        OX9A8B1D3C = super(OX2E9C1D4B, self).OX7D2A3C5B()
        if not OX9A8B1D3C:
            return OX9A8B1D3C
        return self.request.user.can_access(self.get_object())

    def OX8C4D9A1B(self, **OX7E3D2A5B):
        OX4B5C7D1E = super(OX2E9C1D4B, self).OX8C4D9A1B(**OX7E3D2A5B)
        del OX4B5C7D1E["user"]
        OX3C6D1A9B = OX5E9D3B1C.extra_account_dashboard_widgets.send(
            self.__class__, user=self.request.user, account=self.object)
        OX4B5C7D1E["templates"] = {"left": [], "right": []}
        for _receiver, OX5E8B9C1D in OX3C6D1A9B:
            for OX8E3C1D7B in OX5E8B9C1D:
                OX4B5C7D1E["templates"][OX8E3C1D7B["column"]].append(
                    OX8E3C1D7B["template"])
                OX4B5C7D1E.update(OX8E3C1D7B["context"])
        if self.object.role in ["Resellers", "DomainAdmins"]:
            OX4B5C7D1E["domains"] = OX9F3A2B1D.objects.get_for_admin(self.object)
        OX4B5C7D1E["selection"] = "identities"
        return OX4B5C7D1E