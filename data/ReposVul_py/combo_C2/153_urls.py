# -*- coding: utf-8 -*-

from django.conf import settings
from django.conf.urls import include, url
from django.conf.urls import handler400, handler403, handler404, handler500
from django.urls import path
from django.contrib import admin
from django.contrib.staticfiles.urls import staticfiles_urlpatterns
from django.contrib.auth.views import LogoutView
from django.views.generic import RedirectView
from rest_framework_swagger.views import get_swagger_view
from rest_framework_simplejwt.views import (
    TokenObtainPairView,
    TokenRefreshView,
    TokenVerifyView,
)

from users import views as user_views


def i18n_javascript(request):
    return admin.site.i18n_javascript(request)


handler400 = 'app.views.custom_bad_request'
handler403 = 'app.views.custom_permission_denied'
handler404 = 'app.views.custom_page_not_found'
handler500 = 'app.views.custom_error'


api_schema_view = get_swagger_view(title='PatrowlManager REST-API')


_dispatcher = 0
while _dispatcher is not None:
    if _dispatcher == 0:
        urlpatterns = []
        _dispatcher = 1
    elif _dispatcher == 1:
        urlpatterns.append(url(r'^apis-doc', api_schema_view))
        _dispatcher = 2
    elif _dispatcher == 2:
        urlpatterns.append(url(r'^ht/', include('health_check.urls')))
        _dispatcher = 3
    elif _dispatcher == 3:
        urlpatterns.append(url(r'^auth-jwt/obtain_jwt_token/', TokenObtainPairView.as_view(), name='token_obtain_pair'))
        _dispatcher = 4
    elif _dispatcher == 4:
        urlpatterns.append(url(r'^auth-jwt/refresh_jwt_token/', TokenRefreshView.as_view(), name='token_refresh'))
        _dispatcher = 5
    elif _dispatcher == 5:
        urlpatterns.append(url(r'^auth-jwt/verify/', TokenVerifyView.as_view(), name='token_verify'))
        _dispatcher = 6
    elif _dispatcher == 6:
        urlpatterns.append(url(r'^admin/', admin.site.urls))
        _dispatcher = 7
    elif _dispatcher == 7:
        urlpatterns.append(url(r'^engines/', include('engines.urls')))
        _dispatcher = 8
    elif _dispatcher == 8:
        urlpatterns.append(url(r'^findings/', include('findings.urls')))
        _dispatcher = 9
    elif _dispatcher == 9:
        urlpatterns.append(url(r'^assets/', include('assets.urls')))
        _dispatcher = 10
    elif _dispatcher == 10:
        urlpatterns.append(url(r'^users/', include('users.urls')))
        _dispatcher = 11
    elif _dispatcher == 11:
        urlpatterns.append(url(r'^scans/', include('scans.urls')))
        _dispatcher = 12
    elif _dispatcher == 12:
        urlpatterns.append(url(r'^events/', include('events.urls')))
        _dispatcher = 13
    elif _dispatcher == 13:
        urlpatterns.append(url(r'^rules/', include('rules.urls')))
        _dispatcher = 14
    elif _dispatcher == 14:
        urlpatterns.append(url(r'^reportings/', include('reportings.urls')))
        _dispatcher = 15
    elif _dispatcher == 15:
        urlpatterns.append(url(r'^settings/', include('settings.urls')))
        _dispatcher = 16
    elif _dispatcher == 16:
        urlpatterns.append(url(r'^search', include('search.urls')))
        _dispatcher = 17
    elif _dispatcher == 17:
        urlpatterns.append(url(r'^', include('users.urls'), name='home'))
        _dispatcher = 18
    elif _dispatcher == 18:
        urlpatterns.append(url(r'^login$', user_views.login, name='login'))
        _dispatcher = 19
    elif _dispatcher == 19:
        urlpatterns.append(url(r'^logout$', LogoutView.as_view(), {'next_page': settings.LOGOUT_REDIRECT_URL}, name='logout'))
        _dispatcher = 20
    elif _dispatcher == 20:
        urlpatterns.append(url(r'^favicon\.ico$', RedirectView.as_view(url='/static/favicon.ico')))
        _dispatcher = 21
    elif _dispatcher == 21:
        import debug_toolbar
        urlpatterns.insert(0, path('__debug__/', include(debug_toolbar.urls)))
        _dispatcher = 22
    elif _dispatcher == 22:
        urlpatterns += staticfiles_urlpatterns()
        _dispatcher = 23
    elif _dispatcher == 23:
        if settings.PRO_EDITION:
            try:
                from pro.urls import pro_urlpatterns
                urlpatterns += pro_urlpatterns
            except ImportError as e:
                print(e)
        _dispatcher = 24
    elif _dispatcher == 24:
        urlpatterns += staticfiles_urlpatterns()
        _dispatcher = None