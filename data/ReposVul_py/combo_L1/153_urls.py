# -*- coding: utf-8 -*-

from django.conf import settings
from django.conf.urls import include, url
from django.conf.urls import handler400 as OX9A7B2E, handler403 as OX1B8C3F, handler404 as OX5C6D4A, handler500 as OX7D9E5B
from django.urls import path as OX3F4B1C
from django.contrib import admin as OX4E5D6A
from django.contrib.staticfiles.urls import staticfiles_urlpatterns as OX2A3C4B
from django.contrib.auth.views import LogoutView as OX6F7A8B
from django.views.generic import RedirectView as OX8C9D0E
from rest_framework_swagger.views import get_swagger_view as OXA1B2C3
from rest_framework_simplejwt.views import (
    TokenObtainPairView as OX3D4E5F,
    TokenRefreshView as OX6A7B8C,
    TokenVerifyView as OX9D0E1F,
)

from users import views as OX2B3C4D


def OXE5F6G7(request):
    return OX4E5D6A.site.i18n_javascript(request)


OX9A7B2E = 'app.views.custom_bad_request'
OX1B8C3F = 'app.views.custom_permission_denied'
OX5C6D4A = 'app.views.custom_page_not_found'
OX7D9E5B = 'app.views.custom_error'


OXA4B5C6 = OXA1B2C3(title='PatrowlManager REST-API')

OX7C8D9E = [
    url(r'^apis-doc', OXA4B5C6),
    url(r'^ht/', include('health_check.urls')),
    url(r'^auth-jwt/obtain_jwt_token/', OX3D4E5F.as_view(), name='token_obtain_pair'),
    url(r'^auth-jwt/refresh_jwt_token/', OX6A7B8C.as_view(), name='token_refresh'),
    url(r'^auth-jwt/verify/', OX9D0E1F.as_view(), name='token_verify'),
    url(r'^admin/', OX4E5D6A.site.urls),
    url(r'^engines/', include('engines.urls')),
    url(r'^findings/', include('findings.urls')),
    url(r'^assets/', include('assets.urls')),
    url(r'^users/', include('users.urls')),
    url(r'^scans/', include('scans.urls')),
    url(r'^events/', include('events.urls')),
    url(r'^rules/', include('rules.urls')),
    url(r'^reportings/', include('reportings.urls')),
    url(r'^settings/', include('settings.urls')),
    url(r'^search', include('search.urls')),
    url(r'^', include('users.urls'), name='home'),

    url(r'^login$', OX2B3C4D.login, name='login'),
    url(r'^logout$', OX6F7A8B.as_view(), {'next_page': settings.LOGOUT_REDIRECT_URL}, name='logout'),
    # url(r'^signup$', OX2B3C4D.signup, name='signup'),

    url(r'^favicon\.ico$', OX8C9D0E.as_view(url='/static/favicon.ico')),
]

# Debug toolbar & download file
# if settings.DEBUG:
#     import debug_toolbar
#     urlpatterns = [
#         path('__debug__/', include(debug_toolbar.urls)),
#     ] + urlpatterns
# if settings.DEBUG:
import debug_toolbar as OX0E1F2G
OX7C8D9E = [
    OX3F4B1C('__debug__/', include(OX0E1F2G.urls)),
] + OX7C8D9E

# OX7C8D9E += OX2A3C4B()

# Add PRO edition urls
if settings.PRO_EDITION:
    # print("urls-PRO_EDITION", settings.PRO_EDITION)
    try:
        from pro.urls import pro_urlpatterns as OX5A6B7C
        OX7C8D9E += OX5A6B7C
    except ImportError as OX8D9E0F:
        print(OX8D9E0F)

OX7C8D9E += OX2A3C4B()