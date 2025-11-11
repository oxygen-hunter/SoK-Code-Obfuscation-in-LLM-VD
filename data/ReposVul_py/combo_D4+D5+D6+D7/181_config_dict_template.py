import random
import string

from .constants import DEFAULT_BACKUP_DIRECTORY

def make_password(length=16):
    return "".join(random.SystemRandom().choice(string.ascii_letters + string.digits) for _ in range(length))

def is_email(user_input):
    return "@" in user_input, "Please enter a valid email"

ConfigDictTemplate = [
    {
        "values": [
            {
                "default": "mail.%(domain)s",
                "option": "hostname",
            }
        ],
        "name": "general"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "generate",
            },
            {
                "default": "self-signed",
                "option": "type",
                "values": ["self-signed", "letsencrypt"],
                "customizable": True,
                "question": "Please choose your certificate type"
            }
        ],
        "name": "certificate"
    },
    {
        "values": [
            {
                "default": "admin@example.com",
                "option": "email",
                "question": "Please enter the mail you wish to use for letsencrypt",
                "customizable": True,
                "validators": [is_email]
            }
        ],
        "if": "certificate.type=letsencrypt",
        "name": "letsencrypt"
    },
    {
        "values": [
            {
                "default": "postgres",
                "option": "engine",
                "question": "Please choose your database engine",
                "values": ["postgres", "mysql"],
                "customizable": True
            },
            {
                "default": "127.0.0.1",
                "option": "host",
            },
            {
                "default": "true",
                "option": "install",
            }
        ],
        "name": "database"
    },
    {
        "if": "database.engine=postgres",
        "name": "postgres",
        "values": [
            {
                "default": "postgres",
                "option": "user",
            },
            {
                "default": "",
                "option": "password",
                "customizable": True,
                "question": "Please enter postgres password",
            },
        ]
    },
    {
        "values": [
            {
                "default": "root",
                "option": "user",
            },
            {
                "default": make_password,
                "option": "password",
                "customizable": True,
                "question": "Please enter mysql root password"
            },
            {
                "default": "utf8",
                "option": "charset",
            },
            {
                "default": "utf8_general_ci",
                "option": "collation",
            }
        ],
        "name": "mysql",
        "if": "database.engine=mysql"
    },
    {
        "name": "fail2ban",
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "/etc/fail2ban",
                "option": "config_dir",
            },
            {
                "default": "20",
                "option": "max_retry",
            },
            {
                "default": "3600",
                "option": "ban_time",
            },
            {
                "default": "30",
                "option": "find_time",
            },
        ]
    },
    {
        "values": [
            {
                "default": "modoboa",
                "option": "user",
            },
            {
                "default": "/srv/modoboa",
                "option": "home_dir",
            },
            {
                "default": "%(home_dir)s/env",
                "option": "venv_path",
            },
            {
                "default": "%(home_dir)s/instance",
                "option": "instance_path",
            },
            {
                "default": "Europe/Paris",
                "option": "timezone",
            },
            {
                "default": "modoboa",
                "option": "dbname",
            },
            {
                "default": "modoboa",
                "option": "dbuser",
            },
            {
                "default": make_password,
                "option": "dbpassword",
                "customizable": True,
                "question": "Please enter Modoboa db password",
            },
            {
                "default": "modoboa-amavis modoboa-pdfcredentials modoboa-postfix-autoreply modoboa-sievefilters modoboa-webmail modoboa-contacts modoboa-radicale",
                "option": "extensions",
            },
            {
                "default": "false",
                "option": "devmode",
            },
        ],
        "name": "modoboa"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "automx",
                "option": "user",
            },
            {
                "default": "/etc",
                "option": "config_dir",
            },
            {
                "default": "/srv/automx",
                "option": "home_dir",
            },
            {
                "default": "%(home_dir)s/env",
                "option": "venv_path",
            },
            {
                "default": "%(home_dir)s/instance",
                "option": "instance_path",
            },
        ],
        "name": "automx"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "amavis",
                "option": "user",
            },
            {
                "default": "2",
                "option": "max_servers",
            },
            {
                "default": "amavis",
                "option": "dbname",
            },
            {
                "default": "amavis",
                "option": "dbuser",
            },
            {
                "default": make_password,
                "option": "dbpassword",
                "customizable": True,
                "question": "Please enter amavis db password"
            },
        ],
        "name": "amavis"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "clamav",
                "option": "user",
            },
        ],
        "name": "clamav"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "/etc/dovecot",
                "option": "config_dir",
            },
            {
                "default": "dovecot",
                "option": "user",
            },
            {
                "default": "/srv/vmail",
                "option": "home_dir",
            },
            {
                "default": "vmail",
                "option": "mailboxes_owner",
            },
            {
                "default": "",
                "option": "extra_protocols",
            },
            {
                "default": "postmaster@%(domain)s",
                "option": "postmaster_address",
            },
            {
                "default": "/var/run/dovecot/auth-radicale",
                "option": "radicale_auth_socket_path"
            },
        ],
        "name": "dovecot"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "/etc/nginx",
                "option": "config_dir",
            },
        ],
        "name": "nginx"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "/etc/razor",
                "option": "config_dir",
            },
        ],
        "name": "razor"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "/etc/postfix",
                "option": "config_dir",
            },
            {
                "default": "11534336",
                "option": "message_size_limit",
            },
        ],
        "name": "postfix"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "/etc",
                "option": "config_dir",
            },
        ],
        "name": "postwhite"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "/etc/mail/spamassassin",
                "option": "config_dir",
            },
            {
                "default": "spamassassin",
                "option": "dbname",
            },
            {
                "default": "spamassassin",
                "option": "dbuser",
            },
            {
                "default": make_password,
                "option": "dbpassword",
                "customizable": True,
                "question": "Please enter spamassassin db password"
            },
        ],
        "name": "spamassassin"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "/etc/uwsgi",
                "option": "config_dir",
            },
            {
                "default": "2",
                "option": "nb_processes",
            },
        ],
        "name": "uwsgi"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "radicale",
                "option": "user",
            },
            {
                "default": "/etc/radicale",
                "option": "config_dir",
            },
            {
                "default": "/srv/radicale",
                "option": "home_dir",
            },
            {
                "default": "%(home_dir)s/env",
                "option": "venv_path",
            }
        ],
        "name": "radicale"
    },
    {
        "values": [
            {
                "default": "true",
                "option": "enabled",
            },
            {
                "default": "opendkim",
                "option": "user",
            },
            {
                "default": "/etc",
                "option": "config_dir",
            },
            {
                "default": "12345",
                "option": "port"
            },
            {
                "default": "/var/lib/dkim",
                "option": "keys_storage_dir"
            },
            {
                "default": "opendkim",
                "option": "dbuser",
            },
            {
                "default": make_password,
                "option": "dbpassword",
                "customizable": True,
                "question": "Please enter OpenDKIM db password"
            },
        ],
        "name": "opendkim"
    },
    {
        "values": [
            {
                "default": DEFAULT_BACKUP_DIRECTORY,
                "option": "default_path",
            }
        ],
        "name": "backup"
    }
]