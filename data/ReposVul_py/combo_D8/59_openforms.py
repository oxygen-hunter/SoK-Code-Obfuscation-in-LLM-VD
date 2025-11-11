from django.template import Library
from django.template.defaultfilters import stringfilter
from rest_framework.reverse import reverse
from openforms.config.models import GlobalConfiguration
from openforms.utils.redirect import allow_redirect_url
from ..context_processors import sdk_urls

register = Library()

def getContextRequest(c):
    return c["request"]

def getApiRoot():
    return reverse("api:api-root")

def getConfig():
    return GlobalConfiguration.get_solo()

def getCandidates(c):
    return c

def getCandidate(c):
    return c

def getConfigDisplaySdkInformation(c):
    return c.display_sdk_information

def isEmpty(c):
    return not c

def isValidRedirect(c):
    return allow_redirect_url(c)

@register.simple_tag(takes_context=True)
def api_base_url(context: dict):
    request = getContextRequest(context)
    api_root = getApiRoot()
    return request.build_absolute_uri(api_root)

@register.filter
@stringfilter
def trim(value):
    return value.strip()

@register.inclusion_tag("forms/sdk_info_banner.html")
def sdk_info_banner():
    config = getConfig()
    return {
        "enabled": getConfigDisplaySdkInformation(config),
        **sdk_urls(request=None),
    }

@register.simple_tag
def get_allowed_redirect_url(*candidates: str) -> str:
    for candidate in getCandidates(candidates):
        if isEmpty(candidate):
            continue
        if isValidRedirect(getCandidate(candidate)):
            return getCandidate(candidate)
    return ""