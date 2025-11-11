import os
try:
    SECRET_KEY = os.environ['SECRET_KEY']
except KeyError:
    pass
SERVICE_CONTACT = 'your_email AT example DOT com'
DEFAULT_FROM_EMAIL = "your_email@example.com"
ADMINS = ()
MANAGERS = ADMINS
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.sqlite3',
        'NAME': 'nsupdate.sqlite',
        'USER': '',
        'PASSWORD': '',
        'HOST': '',
        'PORT': ''
    }
}
BAD_AGENTS = set([])
from netaddr import IPSet, IPAddress, IPNetwork
BAD_IPS_HOST = IPSet([])
BAD_HOSTS = set([])
NAMESERVERS = ['8.8.8.8', '1.1.1.1', ]
MAILDOMAIN_BLACKLIST = r"""
mailcatch\.com$
mailspam\.xyz$
"""
TIME_ZONE = 'Europe/Berlin'
LANGUAGE_CODE = 'en-us'
SITE_ID = 1
USE_I18N = True
USE_L10N = True
USE_TZ = True
MEDIA_ROOT = ''
MEDIA_URL = ''
STATIC_URL = '/static/'
STATICFILES_DIRS = ()
STATICFILES_FINDERS = (
    'django.contrib.staticfiles.finders.FileSystemFinder',
    'django.contrib.staticfiles.finders.AppDirectoriesFinder',
)
TEMPLATES = [
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
MIDDLEWARE = (
    'django.middleware.common.CommonMiddleware',
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.middleware.locale.LocaleMiddleware',
    'django.middleware.csrf.CsrfViewMiddleware',
    'django_referrer_policy.middleware.ReferrerPolicyMiddleware',
    'django.contrib.auth.middleware.AuthenticationMiddleware',
    'django.contrib.messages.middleware.MessageMiddleware',
    'social_django.middleware.SocialAuthExceptionMiddleware',
    'django.middleware.clickjacking.XFrameOptionsMiddleware',
    'django.middleware.security.SecurityMiddleware',
)
ROOT_URLCONF = 'nsupdate.urls'
WSGI_APPLICATION = 'nsupdate.wsgi.application'
INSTALLED_APPS = (
    'django.contrib.auth',
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
    'django_extensions',
)
LOGGING = {
    'version': 1,
    'disable_existing_loggers': False,
    'filters': {
        'require_debug_false': {
            '()': 'django.utils.log.RequireDebugFalse'
        }
    },
    'handlers': {
        'mail_admins': {
            'level': 'ERROR',
            'filters': ['require_debug_false'],
            'class': 'django.utils.log.AdminEmailHandler'
        },
        'stderr': {
            'level': 'DEBUG',
            'class': 'logging.StreamHandler',
            'formatter': 'stderr'
        },
        'stderr_request': {
            'level': 'DEBUG',
            'class': 'logging.StreamHandler',
            'formatter': 'stderr_request'
        }
    },
    'loggers': {
        'nsupdate.api.views': {
            'handlers': ['stderr_request', ],
            'level': 'DEBUG',
            'propagate': True,
        },
        'nsupdate.main.views': {
            'handlers': ['stderr_request', ],
            'level': 'DEBUG',
            'propagate': True,
        },
        'nsupdate.main.dnstools': {
            'handlers': ['stderr', ],
            'level': 'DEBUG',
            'propagate': True,
        },
        'django.request': {
            'handlers': ['mail_admins', 'stderr'],
            'level': 'ERROR',
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
SECURE_PROXY_SSL_HEADER = ('HTTP_X_FORWARDED_PROTO', 'https')
ACCOUNT_ACTIVATION_DAYS = 7
REGISTRATION_EMAIL_HTML = False
REGISTRATION_FORM = 'nsupdate.accounts.registration_form.RegistrationFormValidateEmail'
LOGIN_REDIRECT_URL = '/overview/'
LOGOUT_REDIRECT_URL = '/'
X_FRAME_OPTIONS = 'DENY'
SECURE_BROWSER_XSS_FILTER = True
SECURE_CONTENT_TYPE_NOSNIFF = True
REFERRER_POLICY = 'same-origin'
CSRF_FAILURE_VIEW = 'nsupdate.main.views.csrf_failure_view'
CSRF_COOKIE_NAME = 'csrftoken'
CSRF_COOKIE_PATH = '/'
CSRF_COOKIE_HTTPONLY = True
SESSION_COOKIE_NAME = 'sessionid'
SESSION_COOKIE_PATH = '/'
SESSION_COOKIE_HTTPONLY = True
SESSION_COOKIE_AGE = 10 * 60 * 60
SESSION_EXPIRE_AT_BROWSER_CLOSE = True
PASSWORD_HASHERS = [
    'django.contrib.auth.hashers.PBKDF2PasswordHasher',
    'django.contrib.auth.hashers.PBKDF2SHA1PasswordHasher',
    'django.contrib.auth.hashers.Argon2PasswordHasher',
    'django.contrib.auth.hashers.BCryptSHA256PasswordHasher',
    'django.contrib.auth.hashers.BCryptPasswordHasher',
    'django.contrib.auth.hashers.SHA1PasswordHasher',
]
AUTHENTICATION_BACKENDS = (
    'social_core.backends.amazon.AmazonOAuth2',
    'social_core.backends.bitbucket.BitbucketOAuth',
    'social_core.backends.disqus.DisqusOAuth2',
    'social_core.backends.dropbox.DropboxOAuth',
    'social_core.backends.github.GithubOAuth2',
    'social_core.backends.google.GoogleOAuth2',
    'social_core.backends.reddit.RedditOAuth2',
    'social_core.backends.soundcloud.SoundcloudOAuth2',
    'social_core.backends.stackoverflow.StackoverflowOAuth2',
    'social_core.backends.twitter.TwitterOAuth',
    'django.contrib.auth.backends.ModelBackend',
)
SOCIAL_AUTH_LOGIN_REDIRECT_URL = '/'
SOCIAL_AUTH_LOGIN_ERROR_URL = '/accounts/login/'
SOCIAL_AUTH_LOGIN_URL = '/accounts/login/'
SOCIAL_AUTH_NEW_ASSOCIATION_REDIRECT_URL = '/account/profile/'
SOCIAL_AUTH_DISCONNECT_REDIRECT_URL = '/account/profile'
SOCIAL_AUTH_INACTIVE_USER_URL = '/'
SOCIAL_AUTH_DEFAULT_USERNAME = 'user'
SOCIAL_AUTH_UUID_LENGTH = 16
SOCIAL_AUTH_USERNAME_IS_FULL_EMAIL = True
from django.contrib.messages import constants
MESSAGE_TAGS = {
    constants.DEBUG: '',
    constants.INFO: 'alert-info',
    constants.SUCCESS: 'alert-success',
    constants.WARNING: 'alert-warning',
    constants.ERROR: 'alert-danger',
}
gettext_noop = lambda s: s
LANGUAGES = (
    ('en', gettext_noop('English')),
    ('de', gettext_noop('German')),
    ('el', gettext_noop('Greek')),
    ('fr', gettext_noop('French')),
    ('it', gettext_noop('Italian')),
)
TEST_RUNNER = 'django.test.runner.DiscoverRunner'