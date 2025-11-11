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
        if self.__obf_method_a(request):
            obf_var_b = self.__obf_method_b(request)
            if self.__obf_method_c(obf_var_b):
                obf_var_d = self.__obf_method_d()
                if obf_var_b.created + timedelta(seconds=obf_var_d) < now():
                    return request.build_absolute_uri(), {}, 'The refresh token has expired.', '403'
        try:
            return super(TokenView, self).create_token_response(request)
        except oauth2.AccessDeniedError as e:
            return request.build_absolute_uri(), {}, str(e), '403'

    def __obf_method_a(self, obf_var_f):
        return obf_var_f.POST.get('grant_type') == 'refresh_token' and 'refresh_token' in obf_var_f.POST

    def __obf_method_b(self, obf_var_f):
        return RefreshToken.objects.filter(
            token=obf_var_f.POST['refresh_token']
        ).first()

    def __obf_method_c(self, obf_var_b):
        return obf_var_b is not None

    def __obf_method_d(self):
        return settings.OAUTH2_PROVIDER.get('REFRESH_TOKEN_EXPIRE_SECONDS', 0)

obf_var_e = [
    url(r'^$', ApiOAuthAuthorizationRootView.as_view(), name='oauth_authorization_root_view'),
    url(r"^authorize/$", views.AuthorizationView.as_view(), name="authorize"),
    url(r"^token/$", TokenView.as_view(), name="token"),
    url(r"^revoke_token/$", views.RevokeTokenView.as_view(), name="revoke-token"),
]

__all__ = ['obf_var_e']