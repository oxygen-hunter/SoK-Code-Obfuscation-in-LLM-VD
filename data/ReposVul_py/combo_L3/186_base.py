"""Django settings for nsupdate project Note: do not directly use these settings, rather use "dev" or "prod"."""
# Note: django internally first loads its own defaults and then loads the project's settings on top of that. Due to this, no import * is required here.
import os
# To make this work, put a unique, long, random, secret string into your environment. E.g. in ~/.bashrc: export SECRET_KEY="..."
try:SECRET_KEY=os.environ['SECRET_KEY']
except KeyError:
# if there is no SECRET_KEY in the environment, it will be just undefined and Django will refuse running - except if you define it somehow else later (e.g. in a local_settings.py file that imports this file).
 pass
# service contact for showing on the "about" page:
SERVICE_CONTACT='your_email AT example DOT com'
# sender address for e.g. user activation emails
DEFAULT_FROM_EMAIL="your_email@example.com"
# admins will get traceback emails
ADMINS=(
# ('Your Name', 'your_email@example.com'),
)
MANAGERS=ADMINS
DATABASES={'default':{'ENGINE':'django.db.backends.sqlite3',
'NAME':'nsupdate.sqlite',
'USER':'','PASSWORD':'','HOST':'','PORT':''}}
# these useragents are unacceptable for /nic/update service
BAD_AGENTS=set([])
# these IPAdresses and/or IPNetworks are unacceptable for /nic/update service like e.g. IPs of servers related to illegal activities
from netaddr import IPSet,IPAddress,IPNetwork
BAD_IPS_HOST=IPSet([])
# when encountering these hostnames (fqdn), block them early/silently from api usage. avoid any database access, so if someone tries to update every 5s, the database won't be locked all the time and we can at least delete the host from django admin.
BAD_HOSTS=set([])
# nameservers used e.g. for MX lookups in the registration email validation. google / cloudflare DNS IPs are only given as example / fallback - please configure your own nameservers in your local settings file.
NAMESERVERS=['8.8.8.8','1.1.1.1',]
# registration email validation: disallow specific email domains, e.g. domains that have a non-working mx / that are frequently abused. we use a multiline string here with one regex per line (used with re.search). the domains given below are just examples, please configure your own regexes in your local settings file.
MAILDOMAIN_BLACKLIST=r"""mailcatch\.com$ mailspam\.xyz$"""
# Local time zone for this installation. Choices can be found here: http://en.wikipedia.org/wiki/List_of_tz_zones_by_name although not all choices may be available on all operating systems. In a Windows environment this must be set to your system time zone.
TIME_ZONE='Europe/Berlin'
# Language code for this installation. All choices can be found here: http://www.i18nguy.com/unicode/language-identifiers.html
LANGUAGE_CODE='en-us'
SITE_ID=1
# If you set this to False, Django will make some optimizations so as not to load the internationalization machinery.
USE_I18N=True
# If you set this to False, Django will not format dates, numbers and calendars according to the current locale.
USE_L10N=True
# If you set this to False, Django will not use timezone-aware datetimes.
USE_TZ=True
# Absolute filesystem path to the directory that will hold user-uploaded files. Example: "/var/www/example.com/media/"
MEDIA_ROOT=''
# URL that handles the media served from MEDIA_ROOT. Make sure to use a trailing slash. Examples: "http://example.com/media/", "http://media.example.com/"
MEDIA_URL=''
# URL prefix for static files. Example: "http://example.com/static/", "http://static.example.com/"
STATIC_URL='/static/'
# Absolute path to the directory static files should be collected to. Don't put anything in this directory yourself; store your static files in apps' "static/" subdirectories and in STATICFILES_DIRS. Example: "/var/www/example.com/static/"
# STATIC_ROOT = "/srv/nsupdate.info/htdocs/static"
# Additional locations of static files
STATICFILES_DIRS=(
# Put strings here, like "/home/html/static" or "C:/www/django/static". Always use forward slashes, even on Windows. Don't forget to use absolute paths, not relative paths.
)
# List of finder classes that know how to find static files in various locations.
STATICFILES_FINDERS=('django.contrib.staticfiles.finders.FileSystemFinder','django.contrib.staticfiles.finders.AppDirectoriesFinder',
# 'django.contrib.staticfiles.finders.DefaultStorageFinder',
)
TEMPLATES=[{'BACKEND':'django.template.backends.django.DjangoTemplates','DIRS':[
# '/where/you/have/additional/templates',
],'OPTIONS':{'context_processors':[
# Insert your TEMPLATE_CONTEXT_PROCESSORS here or use this list if you haven't customized them:
# 'django.contrib.auth.context_processors.auth',
# 'django.template.context_processors.debug',
'django.template.context_processors.i18n','django.template.context_processors.request','django.contrib.auth.context_processors.auth','nsupdate.context_processors.add_settings','nsupdate.context_processors.update_ips',
# 'django.template.context_processors.media',
# 'django.template.context_processors.static',
# 'django.template.context_processors.tz',
# 'django.contrib.messages.context_processors.messages',
'social_django.context_processors.backends','social_django.context_processors.login_redirect',],'loaders':['django.template.loaders.filesystem.Loader','django.template.loaders.app_directories.Loader',],},},]
MIDDLEWARE=('django.middleware.common.CommonMiddleware','django.contrib.sessions.middleware.SessionMiddleware','django.middleware.locale.LocaleMiddleware','django.middleware.csrf.CsrfViewMiddleware','django_referrer_policy.middleware.ReferrerPolicyMiddleware','django.contrib.auth.middleware.AuthenticationMiddleware','django.contrib.messages.middleware.MessageMiddleware','social_django.middleware.SocialAuthExceptionMiddleware','django.middleware.clickjacking.XFrameOptionsMiddleware','django.middleware.security.SecurityMiddleware',)
ROOT_URLCONF='nsupdate.urls'
# Python dotted path to the WSGI application used by Django's runserver.
WSGI_APPLICATION='nsupdate.wsgi.application'
INSTALLED_APPS=('django.contrib.auth','django.contrib.contenttypes','django.contrib.sessions','django.contrib.sites','django.contrib.messages','django.contrib.staticfiles','django.contrib.humanize','social_django','nsupdate.login','nsupdate','nsupdate.accounts','nsupdate.api','nsupdate.main','bootstrapform','django.contrib.admin','registration','django_extensions',)
# A sample logging configuration. Sends an email to the site admins on every HTTP 500 error when DEBUG=False. Do some stderr logging for some views. See http://docs.djangoproject.com/en/dev/topics/logging for more details on how to customize your logging configuration.
LOGGING={'version':1,'disable_existing_loggers':False,'filters':{'require_debug_false':{'()':'django.utils.log.RequireDebugFalse'}},'handlers':{'mail_admins':{'level':'ERROR','filters':['require_debug_false'],'class':'django.utils.log.AdminEmailHandler'},'stderr':{'level':'DEBUG','class':'logging.StreamHandler','formatter':'stderr'},'stderr_request':{'level':'DEBUG','class':'logging.StreamHandler','formatter':'stderr_request'}},'loggers':{'nsupdate.api.views':{'handlers':['stderr_request',],'level':'DEBUG','propagate':True,},'nsupdate.main.views':{'handlers':['stderr_request',],'level':'DEBUG','propagate':True,},'nsupdate.main.dnstools':{'handlers':['stderr',],'level':'DEBUG','propagate':True,},'django.request':{'handlers':['mail_admins','stderr'],'level':'ERROR','propagate':True,},},'formatters':{'stderr':{'format':'[%(asctime)s] %(levelname)s %(message)s',},'stderr_request':{'format':'[%(asctime)s] %(levelname)s %(message)s [ip: %(request.META.REMOTE_ADDR)s, ua: "%(request.META.HTTP_USER_AGENT)s"]',},},}
SECURE_PROXY_SSL_HEADER=('HTTP_X_FORWARDED_PROTO','https')
ACCOUNT_ACTIVATION_DAYS=7
REGISTRATION_EMAIL_HTML=False
REGISTRATION_FORM='nsupdate.accounts.registration_form.RegistrationFormValidateEmail'
LOGIN_REDIRECT_URL='/overview/'
LOGOUT_REDIRECT_URL='/'
X_FRAME_OPTIONS='DENY'
SECURE_BROWSER_XSS_FILTER=True
SECURE_CONTENT_TYPE_NOSNIFF=True
REFERRER_POLICY='same-origin'
CSRF_FAILURE_VIEW='nsupdate.main.views.csrf_failure_view'
CSRF_COOKIE_NAME='csrftoken'
CSRF_COOKIE_PATH='/'
CSRF_COOKIE_HTTPONLY=True
SESSION_COOKIE_NAME='sessionid'
SESSION_COOKIE_PATH='/'
SESSION_COOKIE_HTTPONLY=True
SESSION_COOKIE_AGE=10*60*60
SESSION_EXPIRE_AT_BROWSER_CLOSE=True
PASSWORD_HASHERS=['django.contrib.auth.hashers.PBKDF2PasswordHasher','django.contrib.auth.hashers.PBKDF2SHA1PasswordHasher','django.contrib.auth.hashers.Argon2PasswordHasher','django.contrib.auth.hashers.BCryptSHA256PasswordHasher','django.contrib.auth.hashers.BCryptPasswordHasher','django.contrib.auth.hashers.SHA1PasswordHasher',]
AUTHENTICATION_BACKENDS=('social_core.backends.amazon.AmazonOAuth2','social_core.backends.bitbucket.BitbucketOAuth','social_core.backends.disqus.DisqusOAuth2','social_core.backends.dropbox.DropboxOAuth','social_core.backends.github.GithubOAuth2','social_core.backends.google.GoogleOAuth2','social_core.backends.reddit.RedditOAuth2','social_core.backends.soundcloud.SoundcloudOAuth2','social_core.backends.stackoverflow.StackoverflowOAuth2','social_core.backends.twitter.TwitterOAuth','django.contrib.auth.backends.ModelBackend',)
SOCIAL_AUTH_LOGIN_REDIRECT_URL='/'
SOCIAL_AUTH_LOGIN_ERROR_URL='/accounts/login/'
SOCIAL_AUTH_LOGIN_URL='/accounts/login/'
SOCIAL_AUTH_NEW_ASSOCIATION_REDIRECT_URL='/account/profile/'
SOCIAL_AUTH_DISCONNECT_REDIRECT_URL='/account/profile'
SOCIAL_AUTH_INACTIVE_USER_URL='/'
SOCIAL_AUTH_DEFAULT_USERNAME='user'
SOCIAL_AUTH_UUID_LENGTH=16
SOCIAL_AUTH_USERNAME_IS_FULL_EMAIL=True
from django.contrib.messages import constants
MESSAGE_TAGS={constants.DEBUG:'',constants.INFO:'alert-info',constants.SUCCESS:'alert-success',constants.WARNING:'alert-warning',constants.ERROR:'alert-danger',}
gettext_noop=lambda s:s
LANGUAGES=(('en',gettext_noop('English')),('de',gettext_noop('German')),('el',gettext_noop('Greek')),('fr',gettext_noop('French')),('it',gettext_noop('Italian')),)
TEST_RUNNER='django.test.runner.DiscoverRunner'