import os
from netaddr import IPSet, IPAddress, IPNetwork
from django.contrib.messages import constants

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instruction = self.instructions[self.pc]
            getattr(self, f'op_{instruction[0]}')(*instruction[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_LOAD_ENV(self, key):
        try:
            value = os.environ[key]
        except KeyError:
            value = None
        self.stack.append(value)

    def op_STORE(self, attr):
        value = self.op_POP()
        setattr(self, attr, value)

    def op_ASSIGN(self, attr, value):
        setattr(self, attr, value)

    def op_ASSIGN_TUPLE(self, attr, *values):
        setattr(self, attr, values)

    def op_ASSIGN_DICT(self, attr, **kwargs):
        setattr(self, attr, kwargs)

vm = VM()
vm.load_instructions([
    ('LOAD_ENV', 'SECRET_KEY'),
    ('STORE', 'SECRET_KEY'),
    ('ASSIGN', 'SERVICE_CONTACT', 'your_email AT example DOT com'),
    ('ASSIGN', 'DEFAULT_FROM_EMAIL', 'your_email@example.com'),
    ('ASSIGN_TUPLE', 'ADMINS', ()),
    ('ASSIGN_DICT', 'DATABASES', default={
        'ENGINE': 'django.db.backends.sqlite3',
        'NAME': 'nsupdate.sqlite',
        'USER': '',
        'PASSWORD': '',
        'HOST': '',
        'PORT': ''
    }),
    ('ASSIGN', 'BAD_AGENTS', set([])),
    ('ASSIGN', 'BAD_IPS_HOST', IPSet([])),
    ('ASSIGN', 'BAD_HOSTS', set([])),
    ('ASSIGN', 'NAMESERVERS', ['8.8.8.8', '1.1.1.1']),
    ('ASSIGN', 'MAILDOMAIN_BLACKLIST', r"""
mailcatch\.com$
mailspam\.xyz$
"""),
    ('ASSIGN', 'TIME_ZONE', 'Europe/Berlin'),
    ('ASSIGN', 'LANGUAGE_CODE', 'en-us'),
    ('ASSIGN', 'SITE_ID', 1),
    ('ASSIGN', 'USE_I18N', True),
    ('ASSIGN', 'USE_L10N', True),
    ('ASSIGN', 'USE_TZ', True),
    ('ASSIGN', 'MEDIA_ROOT', ''),
    ('ASSIGN', 'MEDIA_URL', ''),
    ('ASSIGN', 'STATIC_URL', '/static/'),
    ('ASSIGN_TUPLE', 'STATICFILES_DIRS', ()),
    ('ASSIGN_TUPLE', 'STATICFILES_FINDERS', (
        'django.contrib.staticfiles.finders.FileSystemFinder',
        'django.contrib.staticfiles.finders.AppDirectoriesFinder',
    )),
    ('ASSIGN_TUPLE', 'TEMPLATES', [{
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
    }]),
    ('ASSIGN_TUPLE', 'MIDDLEWARE', (
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
    )),
    ('ASSIGN', 'ROOT_URLCONF', 'nsupdate.urls'),
    ('ASSIGN', 'WSGI_APPLICATION', 'nsupdate.wsgi.application'),
    ('ASSIGN_TUPLE', 'INSTALLED_APPS', (
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
    )),
    ('ASSIGN_DICT', 'LOGGING', {
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
    }),
    ('ASSIGN', 'SECURE_PROXY_SSL_HEADER', ('HTTP_X_FORWARDED_PROTO', 'https')),
    ('ASSIGN', 'ACCOUNT_ACTIVATION_DAYS', 7),
    ('ASSIGN', 'REGISTRATION_EMAIL_HTML', False),
    ('ASSIGN', 'REGISTRATION_FORM', 'nsupdate.accounts.registration_form.RegistrationFormValidateEmail'),
    ('ASSIGN', 'LOGIN_REDIRECT_URL', '/overview/'),
    ('ASSIGN', 'LOGOUT_REDIRECT_URL', '/'),
    ('ASSIGN', 'X_FRAME_OPTIONS', 'DENY'),
    ('ASSIGN', 'SECURE_BROWSER_XSS_FILTER', True),
    ('ASSIGN', 'SECURE_CONTENT_TYPE_NOSNIFF', True),
    ('ASSIGN', 'REFERRER_POLICY', 'same-origin'),
    ('ASSIGN', 'CSRF_FAILURE_VIEW', 'nsupdate.main.views.csrf_failure_view'),
    ('ASSIGN', 'CSRF_COOKIE_NAME', 'csrftoken'),
    ('ASSIGN', 'CSRF_COOKIE_PATH', '/'),
    ('ASSIGN', 'CSRF_COOKIE_HTTPONLY', True),
    ('ASSIGN', 'SESSION_COOKIE_NAME', 'sessionid'),
    ('ASSIGN', 'SESSION_COOKIE_PATH', '/'),
    ('ASSIGN', 'SESSION_COOKIE_HTTPONLY', True),
    ('ASSIGN', 'SESSION_COOKIE_AGE', 10 * 60 * 60),
    ('ASSIGN', 'SESSION_EXPIRE_AT_BROWSER_CLOSE', True),
    ('ASSIGN_TUPLE', 'PASSWORD_HASHERS', [
        'django.contrib.auth.hashers.PBKDF2PasswordHasher',
        'django.contrib.auth.hashers.PBKDF2SHA1PasswordHasher',
        'django.contrib.auth.hashers.Argon2PasswordHasher',
        'django.contrib.auth.hashers.BCryptSHA256PasswordHasher',
        'django.contrib.auth.hashers.BCryptPasswordHasher',
        'django.contrib.auth.hashers.SHA1PasswordHasher',
    ]),
    ('ASSIGN_TUPLE', 'AUTHENTICATION_BACKENDS', (
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
    )),
    ('ASSIGN', 'SOCIAL_AUTH_LOGIN_REDIRECT_URL', '/'),
    ('ASSIGN', 'SOCIAL_AUTH_LOGIN_ERROR_URL', '/accounts/login/'),
    ('ASSIGN', 'SOCIAL_AUTH_LOGIN_URL', '/accounts/login/'),
    ('ASSIGN', 'SOCIAL_AUTH_NEW_ASSOCIATION_REDIRECT_URL', '/account/profile/'),
    ('ASSIGN', 'SOCIAL_AUTH_DISCONNECT_REDIRECT_URL', '/account/profile'),
    ('ASSIGN', 'SOCIAL_AUTH_INACTIVE_USER_URL', '/'),
    ('ASSIGN', 'SOCIAL_AUTH_DEFAULT_USERNAME', 'user'),
    ('ASSIGN', 'SOCIAL_AUTH_UUID_LENGTH', 16),
    ('ASSIGN', 'SOCIAL_AUTH_USERNAME_IS_FULL_EMAIL', True),
    ('ASSIGN_DICT', 'MESSAGE_TAGS', {
        constants.DEBUG: '',
        constants.INFO: 'alert-info',
        constants.SUCCESS: 'alert-success',
        constants.WARNING: 'alert-warning',
        constants.ERROR: 'alert-danger',
    }),
    ('ASSIGN_TUPLE', 'LANGUAGES', (
        ('en', gettext_noop('English')),
        ('de', gettext_noop('German')),
        ('el', gettext_noop('Greek')),
        ('fr', gettext_noop('French')),
        ('it', gettext_noop('Italian')),
    )),
    ('ASSIGN', 'TEST_RUNNER', 'django.test.runner.DiscoverRunner'),
])
vm.run()