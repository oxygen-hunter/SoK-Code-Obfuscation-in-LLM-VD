# -*- coding: utf-8 -*-

from django.conf import settings
from django.conf.urls import include, url
from django.conf.urls import handler400 as h400, handler403 as h403, handler404 as h404, handler500 as h500
from django.urls import path
from django.contrib import admin
from django.contrib.staticfiles.urls import staticfiles_urlpatterns as sfup
from django.contrib.auth.views import LogoutView as lv
from django.views.generic import RedirectView as rv
from rest_framework_swagger.views import get_swagger_view as gsv
from rest_framework_simplejwt.views import (
    TokenObtainPairView as topv,
    TokenRefreshView as trv,
    TokenVerifyView as tvv,
)

from users import views as uv

def i18n_javascript(req):
    return admin.site.i18n_javascript(req)

h400 = 'app.views.custom_bad_request'
h403 = 'app.views.custom_permission_denied'
h404 = 'app.views.custom_page_not_found'
h500 = 'app.views.custom_error'

def getApiSchemaView():
    return gsv(title='PatrowlManager REST-API')

def getUrlPatterns():
    return [
        url(r'^apis-doc', getApiSchemaView()),
        url(r'^ht/', include('health_check.urls')),
        url(r'^auth-jwt/obtain_jwt_token/', topv.as_view(), name='token_obtain_pair'),
        url(r'^auth-jwt/refresh_jwt_token/', trv.as_view(), name='token_refresh'),
        url(r'^auth-jwt/verify/', tvv.as_view(), name='token_verify'),
        url(r'^admin/', admin.site.urls),
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

        url(r'^login$', uv.login, name='login'),
        url(r'^logout$', lv.as_view(), {'next_page': settings.LOGOUT_REDIRECT_URL}, name='logout'),
        # url(r'^signup$', uv.signup, name='signup'),

        url(r'^favicon\.ico$', rv.as_view(url='/static/favicon.ico')),
    ]

def getDebugUrlPatterns():
    return [
        path('__debug__/', include(debug_toolbar.urls)),
    ]

urlp = getUrlPatterns()
urlp = getDebugUrlPatterns() + urlp

if settings.PRO_EDITION:
    try:
        from pro.urls import pro_urlpatterns as pup
        urlp += pup
    except ImportError as e:
        print(e)

urlp += sfup()
urlpatterns = urlp