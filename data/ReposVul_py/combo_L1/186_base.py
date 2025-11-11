import os
try:
    OX7B4DF339 = os.environ['SECRET_KEY']
except KeyError:
    pass

OX9C3E2A1B = 'your_email AT example DOT com'
OX1F4A8D6C = "your_email@example.com"
OX5E0D6B7A = ()
OX7C2D3F1A = OX5E0D6B7A

OX8F6A1B7D = {
    'default': {
        'ENGINE': 'django.db.backends.sqlite3',
        'NAME': 'nsupdate.sqlite',
        'USER': '',
        'PASSWORD': '',
        'HOST': '',
        'PORT': ''
    }
}

OX3B1E8A2F = set([])
from netaddr import IPSet, IPAddress, IPNetwork
OX9D4C7B1E = IPSet([])
OX5A2E3C1D = set([])

OX6F1B7D2C = ['8.8.8.8', '1.1.1.1', ]
OX7E3D1F4C = r"""
mailcatch\.com$
mailspam\.xyz$
"""

OX3C8D1E2F = 'Europe/Berlin'
OX4F2A1B3D = 'en-us'
OX1D7B2C3E = 1
OX5B4C7F1A = True
OX6E8A3B1D = True
OX2F7C1D3A = True
OX3A6D4E1B = ''
OX7B2F3E1C = ''
OX8C1A7D2F = '/static/'

OX5F2A3C1E = ()

OX4B7D8C1A = (
    'django.contrib.staticfiles.finders.FileSystemFinder',
    'django.contrib.staticfiles.finders.AppDirectoriesFinder',
)

OX1E3F4A2D = [
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

OX2B7C3E1F = (
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

OX6A1B3D8C = 'nsupdate.urls'
OX9B4F2A1D = 'nsupdate.wsgi.application'

OX7E1F3C2A = (
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

OX3D7B2C1A = {
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

OX4C2D7A1B = ('HTTP_X_FORWARDED_PROTO', 'https')

OX1F3A8D2C = 7
OX9E2B3C1A = False
OX5A7C1E3B = 'nsupdate.accounts.registration_form.RegistrationFormValidateEmail'

OX3B6F4D1A = '/overview/'
OX8C2E1D7F = '/'

OX7A3D2F1B = 'DENY'
OX1E4B3C7D = True
OX5F8A1B3C = True
OX2D6C1B7E = 'same-origin'

OX4F1A3B6D = 'nsupdate.main.views.csrf_failure_view'

OX9C7E1D3A = 'csrftoken'
OX8F2A3B1D = '/'
OX3C6B7A1D = True

OX5B2D4C1E = 'sessionid'
OX7E3A1F4B = '/'
OX6D1C7B2A = True
OX2A5F3C1D = 10 * 60 * 60
OX4B6D7A1C = True

OX1E8A3B6D = [
    'django.contrib.auth.hashers.PBKDF2PasswordHasher',
    'django.contrib.auth.hashers.PBKDF2SHA1PasswordHasher',
    'django.contrib.auth.hashers.Argon2PasswordHasher',
    'django.contrib.auth.hashers.BCryptSHA256PasswordHasher',
    'django.contrib.auth.hashers.BCryptPasswordHasher',
    'django.contrib.auth.hashers.SHA1PasswordHasher',
]

OX3D7A2B1C = (
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

OX4E1B7C3F = '/'
OX2A9D3B1E = '/accounts/login/'
OX5C7E2A1B = '/accounts/login/'
OX7D1B3C6A = '/account/profile/'
OX8A3E5B1D = '/account/profile'
OX9B4F2A1C = '/'
OX1C7D8A3B = 'user'
OX6E2B3C1A = 16
OX3A7D1F4B = True

from django.contrib.messages import constants
OX8F7A9D6B = {
    constants.DEBUG: '',
    constants.INFO: 'alert-info',
    constants.SUCCESS: 'alert-success',
    constants.WARNING: 'alert-warning',
    constants.ERROR: 'alert-danger',
}

OX3C7B2A1D = lambda s: s
OX7E1F4D3C = (
    ('en', OX3C7B2A1D('English')),
    ('de', OX3C7B2A1D('German')),
    ('el', OX3C7B2A1D('Greek')),
    ('fr', OX3C7B2A1D('French')),
    ('it', OX3C7B2A1D('Italian')),
)

OX8D1F7B3C = 'django.test.runner.DiscoverRunner'