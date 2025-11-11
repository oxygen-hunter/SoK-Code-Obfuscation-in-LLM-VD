from django.template import Library as OX1A2B3C4D
from django.template.defaultfilters import stringfilter as OX2B3C4D5E

from rest_framework.reverse import reverse as OX3C4D5E6F

from openforms.config.models import GlobalConfiguration as OX4D5E6F7G
from openforms.utils.redirect import allow_redirect_url as OX5E6F7G8H

from ..context_processors import sdk_urls as OX6F7G8H9I

OX1A2B3C4D = OX1A2B3C4D()

@OX1A2B3C4D.simple_tag(takes_context=True)
def OX7B4DF339(OX8E9F0A1B: dict):
    OX9F0A1B2C = OX8E9F0A1B["request"]
    OXA1B2C3D = OX3C4D5E6F("api:api-root")
    return OX9F0A1B2C.build_absolute_uri(OXA1B2C3D)

@OX1A2B3C4D.filter
@OX2B3C4D5E
def OXB2C3D4E5(OXC3D4E5F):
    return OXC3D4E5F.strip()

@OX1A2B3C4D.inclusion_tag("forms/sdk_info_banner.html")
def OXD4E5F6G7():
    OXE5F6G7H = OX4D5E6F7G.get_solo()
    return {
        "enabled": OXE5F6G7H.display_sdk_information,
        **OX6F7G8H9I(request=None),
    }

@OX1A2B3C4D.simple_tag
def OXF6G7H8I9(*OXG7H8I9J: str) -> str:
    for OXH8I9J0A in OXG7H8I9J:
        if not OXH8I9J0A:
            continue
        if OX5E6F7G8H(OXH8I9J0A):
            return OXH8I9J0A
    return ""