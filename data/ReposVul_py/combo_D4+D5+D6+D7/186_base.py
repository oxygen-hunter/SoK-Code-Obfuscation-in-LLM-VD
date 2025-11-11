import os
from netaddr import IPSet

try:
    SECRET_KEY = os.environ['SECRET_KEY']
except KeyError:
    pass

settings_data = {
    'contact': 'your_email AT example DOT com',
    'email': "your_email@example.com",
    'timezone': 'Europe/Berlin',
    'language': 'en-us',
    'site_id': 1,
    'i18n': True,
    'l10n': True,
    'tz': True,
    'media_root': '',
    'media_url': '',
    'static_url': '/static/',
    'proxy_ssl_header': ('HTTP_X_FORWARDED_PROTO', 'https'),
    'activation_days': 7,
    'email_html': False,
    'registration_form': 'nsupdate.accounts.registration_form.RegistrationFormValidateEmail',
    'login_redirect_url': '/overview/',
    'logout_redirect_url': '/',
    'x_frame_options': 'DENY',
    'secure_xss_filter': True,
    'secure_content_type_nosniff': True,
    'referrer_policy': 'same-origin',
    'csrf_failure_view': 'nsupdate.main.views.csrf_failure_view',
    'csrf_cookie': {'name': 'csrftoken', 'path': '/', 'httponly': True},
    'session_cookie': {'name': 'sessionid', 'path': '/', 'httponly': True, 'age': 10 * 60 * 60, 'expire_at_close': True},
    'password_hashers': [
        'django.contrib.auth.hashers.PBKDF2PasswordHasher',
        'django.contrib.auth.hashers.PBKDF2SHA1PasswordHasher',
        'django.contrib.auth.hashers.Argon2PasswordHasher',
        'django.contrib.auth.hashers.BCryptSHA256PasswordHasher',
        'django.contrib.auth.hashers.BCryptPasswordHasher',
        'django.contrib.auth.hashers.SHA1PasswordHasher',
    ],
    'auth_backends': [
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
    ],
    'social_auth': {
        'login_redirect_url': '/',
        'login_error_url': '/accounts/login/',
        'login_url': '/accounts/login/',
        'new_association_redirect_url': '/account/profile/',
        'disconnect_redirect_url': '/account/profile',
        'inactive_user_url': '/',
        'default_username': 'user',
        'uuid_length': 16,
        'username_is_full_email': True
    },
    'message_tags': {
        'debug': '',
        'info': 'alert-info',
        'success': 'alert-success',
        'warning': 'alert-warning',
        'error': 'alert-danger'
    },
    'languages': (
        ('en', lambda s: s('English')),
        ('de', lambda s: s('German')),
        ('el', lambda s: s('Greek')),
        ('fr', lambda s: s('French')),
        ('it', lambda s: s('Italian')),
    ),
    'test_runner': 'django.test.runner.DiscoverRunner',
}

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
BAD_IPS_HOST = IPSet([])
BAD_HOSTS = set([])

NAMESERVERS = ['8.8.8.8', '1.1.1.1']

MAILDOMAIN_BLACKLIST = r"""
mailcatch\.com$
mailspam\.xyz$
"""

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
            'handlers': ['stderr_request'],
            'level': 'DEBUG',
            'propagate': True,
        },
        'nsupdate.main.views': {
            'handlers': ['stderr_request'],
            'level': 'DEBUG',
            'propagate': True,
        },
        'nsupdate.main.dnstools': {
            'handlers': ['stderr'],
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
            'format': '[%(asctime)s] %(levelname)s %(message)s [ip: %(request.META.REMOTE_ADDR)s, ua: "%(request.META.HTTP_USER_AGENT)s"]',
        },
    },
}