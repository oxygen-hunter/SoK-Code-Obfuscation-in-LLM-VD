from django.template import Library
from django.template.defaultfilters import stringfilter
from rest_framework.reverse import reverse
from openforms.config.models import GlobalConfiguration
from openforms.utils.redirect import allow_redirect_url
from ..context_processors import sdk_urls
from ctypes import CDLL, c_char_p

register = Library()

clib = CDLL("./trimlib.so")  # Assuming the C library is compiled as trimlib.so
clib.trim_string.argtypes = [c_char_p]
clib.trim_string.restype = c_char_p

@register.simple_tag(takes_context=True)
def api_base_url(context: dict):
    request = context["request"]
    api_root = reverse("api:api-root")
    return request.build_absolute_uri(api_root)

@register.filter
@stringfilter
def trim(value):
    return clib.trim_string(value.encode('utf-8')).decode('utf-8')

@register.inclusion_tag("forms/sdk_info_banner.html")
def sdk_info_banner():
    config = GlobalConfiguration.get_solo()
    return {
        "enabled": config.display_sdk_information,
        **sdk_urls(request=None),
    }

@register.simple_tag
def get_allowed_redirect_url(*candidates: str) -> str:
    for candidate in candidates:
        if not candidate:
            continue
        if allow_redirect_url(candidate):
            return candidate
    return ""