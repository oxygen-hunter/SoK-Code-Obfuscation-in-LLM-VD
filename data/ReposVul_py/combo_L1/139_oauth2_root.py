from datetime import timedelta
from django.utils.timezone import now as OX1E3F3A4A
from django.conf import settings as OX5B1F9D4D
from django.conf.urls import url as OX2A3B4C5D
from oauthlib import oauth2 as OX3C4D5E6F
from oauth2_provider import views as OX4E5F6A7B
from awx.main.models import RefreshToken as OX6A7B8C9D
from awx.api.views import ApiOAuthAuthorizationRootView as OX7B8C9D0E

class OX9D0E1F2G(OX4E5F6A7B.TokenView):
    def OX1F2G3H4I(self, OX2G3H4I5J):
        if OX2G3H4I5J.POST.get('grant_type') == 'refresh_token' and 'refresh_token' in OX2G3H4I5J.POST:
            OX3H4I5J6K = OX6A7B8C9D.objects.filter(token=OX2G3H4I5J.POST['refresh_token']).first()
            if OX3H4I5J6K:
                OX5J6K7L8M = OX5B1F9D4D.OAUTH2_PROVIDER.get('REFRESH_TOKEN_EXPIRE_SECONDS', 0)
                if OX3H4I5J6K.created + timedelta(seconds=OX5J6K7L8M) < OX1E3F3A4A():
                    return OX2G3H4I5J.build_absolute_uri(), {}, 'The refresh token has expired.', '403'
        try:
            return super(OX9D0E1F2G, self).OX1F2G3H4I(OX2G3H4I5J)
        except OX3C4D5E6F.AccessDeniedError as OX6K7L8M9N:
            return OX2G3H4I5J.build_absolute_uri(), {}, str(OX6K7L8M9N), '403'

OX7L8M9N0O = [
    OX2A3B4C5D(r'^$', OX7B8C9D0E.as_view(), name='oauth_authorization_root_view'),
    OX2A3B4C5D(r"^authorize/$", OX4E5F6A7B.AuthorizationView.as_view(), name="authorize"),
    OX2A3B4C5D(r"^token/$", OX9D0E1F2G.as_view(), name="token"),
    OX2A3B4C5D(r"^revoke_token/$", OX4E5F6A7B.RevokeTokenView.as_view(), name="revoke-token"),
]

__all__ = ['OX7L8M9N0O']