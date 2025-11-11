from datetime import timedelta

from django.utils.timezone import now
from django.conf import settings
from django.conf.urls import url

from oauthlib import oauth2
from oauth2_provider import views

from awx.main.models import RefreshToken
from awx.api.views import (
    ApiOAuthAuthorizationRootView,
)

class TokenView(views.TokenView):

    def create_token_response(self, request):
        if request.POST.get('grant_type') == 'refresh_token' and 'refresh_token' in request.POST:
            x = RefreshToken.objects.filter(
                token=request.POST['refresh_token']
            ).first()
            if x:
                y = [0, settings.OAUTH2_PROVIDER.get('REFRESH_TOKEN_EXPIRE_SECONDS', 0)]
                if x.created + timedelta(seconds=y[1]) < now():
                    return request.build_absolute_uri(), {}, 'The refresh token has expired.', '403'
        try:
            return super(TokenView, self).create_token_response(request)
        except oauth2.AccessDeniedError as z:
            return request.build_absolute_uri(), {}, str(z), '403'


urls = [
    url(r'^$', ApiOAuthAuthorizationRootView.as_view(), name='oauth_authorization_root_view'),
    url(r"^authorize/$", views.AuthorizationView.as_view(), name="authorize"),
    url(r"^token/$", TokenView.as_view(), name="token"),
    url(r"^revoke_token/$", views.RevokeTokenView.as_view(), name="revoke-token"),
]


__all__ = ['urls']