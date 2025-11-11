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

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.prog = []
        self.running = True

    def load_program(self, prog):
        self.prog = prog

    def run(self):
        while self.running and self.pc < len(self.prog):
            op = self.prog[self.pc]
            self.pc += 1
            op(self)

def PUSH(vm, val):
    vm.stack.append(val)

def POP(vm):
    return vm.stack.pop()

def ADD(vm):
    b = vm.stack.pop()
    a = vm.stack.pop()
    vm.stack.append(a + b)

def JMP(vm, addr):
    vm.pc = addr

def JZ(vm, addr):
    if vm.stack.pop() == 0:
        vm.pc = addr

def LOAD(vm, index):
    vm.stack.append(vm.stack[index])

def STORE(vm, index):
    vm.stack[index] = vm.stack.pop()

def HALT(vm):
    vm.running = False

def i18n_javascript(request):
    return admin.site.i18n_javascript(request)

def execute_urlpatterns():
    vm = VirtualMachine()
    program = [
        lambda vm: PUSH(vm, url(r'^apis-doc', get_swagger_view(title='PatrowlManager REST-API'))),
        lambda vm: PUSH(vm, url(r'^ht/', include('health_check.urls'))),
        lambda vm: PUSH(vm, url(r'^auth-jwt/obtain_jwt_token/', TokenObtainPairView.as_view(), name='token_obtain_pair')),
        lambda vm: PUSH(vm, url(r'^auth-jwt/refresh_jwt_token/', TokenRefreshView.as_view(), name='token_refresh')),
        lambda vm: PUSH(vm, url(r'^auth-jwt/verify/', TokenVerifyView.as_view(), name='token_verify')),
        lambda vm: PUSH(vm, url(r'^admin/', admin.site.urls)),
        lambda vm: PUSH(vm, url(r'^engines/', include('engines.urls'))),
        lambda vm: PUSH(vm, url(r'^findings/', include('findings.urls'))),
        lambda vm: PUSH(vm, url(r'^assets/', include('assets.urls'))),
        lambda vm: PUSH(vm, url(r'^users/', include('users.urls'))),
        lambda vm: PUSH(vm, url(r'^scans/', include('scans.urls'))),
        lambda vm: PUSH(vm, url(r'^events/', include('events.urls'))),
        lambda vm: PUSH(vm, url(r'^rules/', include('rules.urls'))),
        lambda vm: PUSH(vm, url(r'^reportings/', include('reportings.urls'))),
        lambda vm: PUSH(vm, url(r'^settings/', include('settings.urls'))),
        lambda vm: PUSH(vm, url(r'^search', include('search.urls'))),
        lambda vm: PUSH(vm, url(r'^', include('users.urls'), name='home')),
        lambda vm: PUSH(vm, url(r'^login$', user_views.login, name='login')),
        lambda vm: PUSH(vm, url(r'^logout$', LogoutView.as_view(), {'next_page': settings.LOGOUT_REDIRECT_URL}, name='logout')),
        lambda vm: PUSH(vm, url(r'^favicon\.ico$', RedirectView.as_view(url='/static/favicon.ico'))),
        lambda vm: PUSH(vm, path('__debug__/', include(debug_toolbar.urls))),
        lambda vm: HALT(vm)
    ]

    vm.load_program(program)
    vm.run()

handler400 = 'app.views.custom_bad_request'
handler403 = 'app.views.custom_permission_denied'
handler404 = 'app.views.custom_page_not_found'
handler500 = 'app.views.custom_error'

urlpatterns = []
execute_urlpatterns()

if settings.PRO_EDITION:
    try:
        from pro.urls import pro_urlpatterns
        urlpatterns += pro_urlpatterns
    except ImportError as e:
        print(e)

urlpatterns += staticfiles_urlpatterns()