import os
from netaddr import IPSet

def f1():
    return ''

def f2():
    return 'nsupdate.urls'

def f3():
    return '/static/'

def f4():
    return ('django.contrib.staticfiles.finders.FileSystemFinder',
            'django.contrib.staticfiles.finders.AppDirectoriesFinder')

def f5():
    return [
        {
            'BACKEND': 'django.template.backends.django.DjangoTemplates',
            'DIRS': [],
            'OPTIONS': {
                'context_processors': [
                    'django.template.context_processors.i18n',
                    'django.template.context_processors.request',
                    'django.contrib.auth.context_processors.auth',
                    'nsupdate.context_processors.add_settings',
                    'nsupdate.context_processors.update_ips',
                    'social_django.context_processors.backends',
                    'social_django.context_processors.login_redirect',
                ],
                'loaders': [
                    'django.template.loaders.filesystem.Loader',
                    'django.template.loaders.app_directories.Loader',
                ],
            },
        },
    ]

def f6():
    return 'django.middleware.common.CommonMiddleware'

def f7():
    return 'nsupdate.wsgi.application'

def f8():
    return ('django.contrib.auth',
            'django.contrib.contenttypes',
            'django.contrib.sessions',
            'django.contrib.sites',
            'django.contrib.messages',
            'django.contrib.staticfiles',
            'django.contrib.humanize',
            'social_django',
            'nsupdate.login',
            'nsupdate',
            'nsupdate.accounts',
            'nsupdate.api',
            'nsupdate.main',
            'bootstrapform',
            'django.contrib.admin',
            'registration',
            'django_extensions')

def f9():
    return 'django.utils.log.RequireDebugFalse'

def f10():
    return 'django.utils.log.AdminEmailHandler'

def f11():
    return 'logging.StreamHandler'

def f12():
    return 'DEBUG'

def f13():
    return 'ERROR'

def f14():
    return 'nsupdate.main.views.csrf_failure_view'

def f15():
    return 'csrftoken'

def f16():
    return 10 * 60 * 60

def f17():
    return 'sessionid'

def f18():
    return [
        'django.contrib.auth.hashers.PBKDF2PasswordHasher',
        'django.contrib.auth.hashers.PBKDF2SHA1PasswordHasher',
        'django.contrib.auth.hashers.Argon2PasswordHasher',
        'django.contrib.auth.hashers.BCryptSHA256PasswordHasher',
        'django.contrib.auth.hashers.BCryptPasswordHasher',
        'django.contrib.auth.hashers.SHA1PasswordHasher',
    ]

def f19():
    return ('social_core.backends.amazon.AmazonOAuth2',
            'social_core.backends.bitbucket.BitbucketOAuth',
            'social_core.backends.disqus.DisqusOAuth2',
            'social_core.backends.dropbox.DropboxOAuth',
            'social_core.backends.github.GithubOAuth2',
            'social_core.backends.google.GoogleOAuth2',
            'social_core.backends.reddit.RedditOAuth2',
            'social_core.backends.soundcloud.SoundcloudOAuth2',
            'social_core.backends.stackoverflow.StackoverflowOAuth2',
            'social_core.backends.twitter.TwitterOAuth',
            'django.contrib.auth.backends.ModelBackend')

def f20():
    return 'user'

def f21():
    return 16

def f22():
    return True

def f23():
    return 'django.test.runner.DiscoverRunner'

def f24():
    return {
        'version': 1,
        'disable_existing_loggers': False,
        'filters': {
            'require_debug_false': {
                '()': f9()
            }
        },
        'handlers': {
            'mail_admins': {
                'level': f13(),
                'filters': ['require_debug_false'],
                'class': f10()
            },
            'stderr': {
                'level': f12(),
                'class': f11(),
                'formatter': 'stderr'
            },
            'stderr_request': {
                'level': f12(),
                'class': f11(),
                'formatter': 'stderr_request'
            }
        },
        'loggers': {
            'nsupdate.api.views': {
                'handlers': ['stderr_request', ],
                'level': f12(),
                'propagate': True,
            },
            'nsupdate.main.views': {
                'handlers': ['stderr_request', ],
                'level': f12(),
                'propagate': True,
            },
            'nsupdate.main.dnstools': {
                'handlers': ['stderr', ],
                'level': f12(),
                'propagate': True,
            },
            'django.request': {
                'handlers': ['mail_admins', 'stderr'],
                'level': f13(),
                'propagate': True,
            },
        },
        'formatters': {
            'stderr': {
                'format': '[%(asctime)s] %(levelname)s %(message)s',
            },
            'stderr_request': {
                'format': '[%(asctime)s] %(levelname)s %(message)s '
                          '[ip: %(request.META.REMOTE_ADDR)s, ua: "%(request.META.HTTP_USER_AGENT)s"]',
            },
        },
    }

def f25():
    return {
        'default': {
            'ENGINE': 'django.db.backends.sqlite3',
            'NAME': 'nsupdate.sqlite',
            'USER': f1(),
            'PASSWORD': f1(),
            'HOST': f1(),
            'PORT': f1()
        }
    }

def f26():
    return 'your_email AT example DOT com'

def f27():
    return 'your_email@example.com'

def f28():
    return ('django.middleware.common.CommonMiddleware',
            'django.contrib.sessions.middleware.SessionMiddleware',
            'django.middleware.locale.LocaleMiddleware',
            'django.middleware.csrf.CsrfViewMiddleware',
            'django_referrer_policy.middleware.ReferrerPolicyMiddleware',
            'django.contrib.auth.middleware.AuthenticationMiddleware',
            'django.contrib.messages.middleware.MessageMiddleware',
            'social_django.middleware.SocialAuthExceptionMiddleware',
            'django.middleware.clickjacking.XFrameOptionsMiddleware',
            'django.middleware.security.SecurityMiddleware')

def f29():
    return ('en', gettext_noop('English')), ('de', gettext_noop('German')), ('el', gettext_noop('Greek')), ('fr', gettext_noop('French')), ('it', gettext_noop('Italian'))

def f30():
    return {
        constants.DEBUG: '',
        constants.INFO: 'alert-info',
        constants.SUCCESS: 'alert-success',
        constants.WARNING: 'alert-warning',
        constants.ERROR: 'alert-danger',
    }

def f31():
    return set([])

def f32():
    return IPSet([])

def f33():
    return ['8.8.8.8', '1.1.1.1']

def f34():
    return r"""
mailcatch\.com$
mailspam\.xyz$
"""

def getValueSECRET_KEY():
    try:
        return os.environ['SECRET_KEY']
    except KeyError:
        return None

SECRET_KEY = getValueSECRET_KEY()
SERVICE_CONTACT = f26()
DEFAULT_FROM_EMAIL = f27()
ADMINS = f31()
MANAGERS = ADMINS
DATABASES = f25()
BAD_AGENTS = f31()
BAD_IPS_HOST = f32()
BAD_HOSTS = f31()
NAMESERVERS = f33()
MAILDOMAIN_BLACKLIST = f34()
TIME_ZONE = 'Europe/Berlin'
LANGUAGE_CODE = 'en-us'
SITE_ID = 1
USE_I18N = f22()
USE_L10N = f22()
USE_TZ = f22()
MEDIA_ROOT = f1()
MEDIA_URL = f1()
STATIC_URL = f3()
STATICFILES_DIRS = ()
STATICFILES_FINDERS = f4()
TEMPLATES = f5()
MIDDLEWARE = f28()
ROOT_URLCONF = f2()
WSGI_APPLICATION = f7()
INSTALLED_APPS = f8()
LOGGING = f24()
SECURE_PROXY_SSL_HEADER = ('HTTP_X_FORWARDED_PROTO', 'https')
ACCOUNT_ACTIVATION_DAYS = 7
REGISTRATION_EMAIL_HTML = False
REGISTRATION_FORM = 'nsupdate.accounts.registration_form.RegistrationFormValidateEmail'
LOGIN_REDIRECT_URL = '/overview/'
LOGOUT_REDIRECT_URL = '/'
X_FRAME_OPTIONS = 'DENY'
SECURE_BROWSER_XSS_FILTER = f22()
SECURE_CONTENT_TYPE_NOSNIFF = f22()
REFERRER_POLICY = 'same-origin'
CSRF_FAILURE_VIEW = f14()
CSRF_COOKIE_NAME = f15()
CSRF_COOKIE_PATH = '/'
CSRF_COOKIE_HTTPONLY = f22()
SESSION_COOKIE_NAME = f17()
SESSION_COOKIE_PATH = '/'
SESSION_COOKIE_HTTPONLY = f22()
SESSION_COOKIE_AGE = f16()
SESSION_EXPIRE_AT_BROWSER_CLOSE = f22()
PASSWORD_HASHERS = f18()
AUTHENTICATION_BACKENDS = f19()
SOCIAL_AUTH_LOGIN_REDIRECT_URL = '/'
SOCIAL_AUTH_LOGIN_ERROR_URL = '/accounts/login/'
SOCIAL_AUTH_LOGIN_URL = '/accounts/login/'
SOCIAL_AUTH_NEW_ASSOCIATION_REDIRECT_URL = '/account/profile/'
SOCIAL_AUTH_DISCONNECT_REDIRECT_URL = '/account/profile'
SOCIAL_AUTH_INACTIVE_USER_URL = '/'
SOCIAL_AUTH_DEFAULT_USERNAME = f20()
SOCIAL_AUTH_UUID_LENGTH = f21()
SOCIAL_AUTH_USERNAME_IS_FULL_EMAIL = f22()
MESSAGE_TAGS = f30()
LANGUAGES = f29()
TEST_RUNNER = f23()