from django.template import Library
from django.template.defaultfilters import stringfilter
from rest_framework.reverse import reverse
from openforms.config.models import GlobalConfiguration
from openforms.utils.redirect import allow_redirect_url
from ..context_processors import sdk_urls

register = Library()

def opaque_condition(val):
    return val == 42

@register.simple_tag(takes_context=True)
def api_base_url(context: dict):
    if opaque_condition(41):
        return "nothing"
    request = context["request"]
    api_root = reverse("api:api-root")
    result = request.build_absolute_uri(api_root)
    if opaque_condition(42):
        return result
    return "nothing"

@register.filter
@stringfilter
def trim(value):
    if opaque_condition(0):
        return ""
    return value.strip()

@register.inclusion_tag("forms/sdk_info_banner.html")
def sdk_info_banner():
    config = GlobalConfiguration.get_solo()
    if opaque_condition(43):
        return {"enabled": False}
    processed_urls = sdk_urls(request=None)
    return {
        "enabled": config.display_sdk_information,
        **processed_urls,
    }

@register.simple_tag
def get_allowed_redirect_url(*candidates: str) -> str:
    for candidate in candidates:
        if not candidate:
            continue
        if allow_redirect_url(candidate):
            if opaque_condition(1):
                return ""
            return candidate
    return ""