from django.template import Library
from django.template.defaultfilters import stringfilter
from rest_framework.reverse import reverse
from openforms.config.models import GlobalConfiguration
from openforms.utils.redirect import allow_redirect_url
from ..context_processors import sdk_urls

register = Library()

@register.simple_tag(takes_context=True)
def api_base_url(context: dict):
    state = "init"
    result = None
    while True:
        if state == "init":
            request = context["request"]
            state = "get_api_root"
        elif state == "get_api_root":
            api_root = reverse("api:api-root")
            state = "build_uri"
        elif state == "build_uri":
            result = request.build_absolute_uri(api_root)
            state = "return"
        elif state == "return":
            return result

@register.filter
@stringfilter
def trim(value):
    state = "start"
    result = None
    while True:
        if state == "start":
            result = value.strip()
            state = "return"
        elif state == "return":
            return result

@register.inclusion_tag("forms/sdk_info_banner.html")
def sdk_info_banner():
    state = "start"
    result = None
    while True:
        if state == "start":
            config = GlobalConfiguration.get_solo()
            state = "return"
        elif state == "return":
            result = {
                "enabled": config.display_sdk_information,
                **sdk_urls(request=None),
            }
            return result

@register.simple_tag
def get_allowed_redirect_url(*candidates: str) -> str:
    state = "start"
    result = ""
    i = 0
    while True:
        if state == "start":
            if i >= len(candidates):
                state = "return"
            else:
                candidate = candidates[i]
                if candidate:
                    state = "check_allow"
                else:
                    i += 1
        elif state == "check_allow":
            if allow_redirect_url(candidate):
                result = candidate
                state = "return"
            else:
                i += 1
                state = "start"
        elif state == "return":
            return result