# -*- coding: utf-8 -*-

from django.conf import settings
from django.conf.urls import include, url
from django.conf.urls import handler400 as h400, handler403 as h403, handler404 as h404, handler500 as h500
from django.urls import path
from django.contrib import admin
from django.contrib.staticfiles.urls import staticfiles_urlpatterns as sfup
from django.contrib.auth.views import LogoutView as LV
from django.views.generic import RedirectView as RV
from rest_framework_swagger.views import get_swagger_view as gsv
from rest_framework_simplejwt.views import (
    TokenObtainPairView as TOPV,
    TokenRefreshView as TRV,
    TokenVerifyView as TVV,
)

from users import views as u_views

def i18n_javascript(req):
    return admin.site.i18n_javascript(req)

h400, h403, h404, h500 = ['app.views.custom_bad_request', 'app.views.custom_permission_denied', 'app.views.custom_page_not_found', 'app.views.custom_error']

api_schema_view = gsv(title='PatrowlManager REST-API')

urls = [
    url(r'^apis-doc', api_schema_view),
    url(r'^ht/', include('health_check.urls')),
    url(r'^auth-jwt/obtain_jwt_token/', TOPV.as_view(), name='token_obtain_pair'),
    url(r'^auth-jwt/refresh_jwt_token/', TRV.as_view(), name='token_refresh'),
    url(r'^auth-jwt/verify/', TVV.as_view(), name='token_verify'),
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

    url(r'^login$', u_views.login, name='login'),
    url(r'^logout$', LV.as_view(), {'next_page': settings.LOGOUT_REDIRECT_URL}, name='logout'),
    # url(r'^signup$', u_views.signup, name='signup'),

    url(r'^favicon\.ico$', RV.as_view(url='/static/favicon.ico')),
]

# Debug toolbar & download file
# if settings.DEBUG:
#     import debug_toolbar
#     urlpatterns = [
#         path('__debug__/', include(debug_toolbar.urls)),
#     ] + urlpatterns
# if settings.DEBUG:
import debug_toolbar
urls = [
    path('__debug__/', include(debug_toolbar.urls)),
] + urls

# urlpatterns += staticfiles_urlpatterns()

# Add PRO edition urls
if settings.PRO_EDITION:
    # print("urls-PRO_EDITION", settings.PRO_EDITION)
    try:
        from pro.urls import pro_urlpatterns as pro_urls
        urls += pro_urls
    except ImportError as e:
        print(e)

urls += sfup()
urlpatterns = urls