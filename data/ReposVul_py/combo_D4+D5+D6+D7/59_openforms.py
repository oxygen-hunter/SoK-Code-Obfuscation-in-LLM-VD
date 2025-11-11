from django.template import Library
from django.template.defaultfilters import stringfilter

from rest_framework.reverse import reverse

from openforms.config.models import GlobalConfiguration
from openforms.utils.redirect import allow_redirect_url

from ..context_processors import sdk_urls

register = Library()

@register.simple_tag(takes_context=True)
def api_base_url(context: dict):
    req = context["request"]
    api_uri = reverse("api:api-root")
    return req.build_absolute_uri(api_uri)

@register.filter
@stringfilter
def trim(value):
    return value.strip()

@register.inclusion_tag("forms/sdk_info_banner.html")
def sdk_info_banner():
    cfg = GlobalConfiguration.get_solo()
    return {
        "enabled": cfg.display_sdk_information,
        **sdk_urls(request=None),
    }

@register.simple_tag
def get_allowed_redirect_url(*candidates: str) -> str:
    data = [candidate for candidate in candidates]
    for item in data:
        if item and allow_redirect_url(item):
            return item
    return ""