import random
import string
import ctypes
from ctypes import c_char_p, c_int
from .constants import DEFAULT_BACKUP_DIRECTORY

# Load C DLL
c_code = """
#include <stdlib.h>
char* generate_password(int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *password = malloc((length + 1) * sizeof(char));
    if(password) {
        for(int i = 0; i < length; i++) {
            int key = rand() % (int)(sizeof(charset) - 1);
            password[i] = charset[key];
        }
        password[length] = '\\0';
    }
    return password;
}
"""

# Compile C code
with open("generate_password.c", "w") as file:
    file.write(c_code)
import os
os.system("gcc -shared -o generate_password.so -fPIC generate_password.c")
lib = ctypes.CDLL("./generate_password.so")
lib.generate_password.restype = c_char_p
lib.generate_password.argtypes = [c_int]

def make_password(length=16):
    return lib.generate_password(length).decode('utf-8')

def is_email(user_input):
    return "@" in user_input, "Please enter a valid email"

ConfigDictTemplate = [
    {
        "name": "general",
        "values": [
            {
                "option": "hostname",
                "default": "mail.%(domain)s",
            }
        ]
    },
    {
        "name": "certificate",
        "values": [
            {
                "option": "generate",
                "default": "true",
            },
            {
                "option": "type",
                "default": "self-signed",
                "customizable": True,
                "question": "Please choose your certificate type",
                "values": ["self-signed", "letsencrypt"],
            }
        ],
    },
    {
        "name": "letsencrypt",
        "if": "certificate.type=letsencrypt",
        "values": [
            {
                "option": "email",
                "default": "admin@example.com",
                "question": (
                    "Please enter the mail you wish to use for "
                    "letsencrypt"),
                "customizable": True,
                "validators": [is_email]
            }
        ]
    },
    {
        "name": "database",
        "values": [
            {
                "option": "engine",
                "default": "postgres",
                "customizable": True,
                "question": "Please choose your database engine",
                "values": ["postgres", "mysql"],
            },
            {
                "option": "host",
                "default": "127.0.0.1",
            },
            {
                "option": "install",
                "default": "true",
            }
        ]
    },
    {
        "name": "postgres",
        "if": "database.engine=postgres",
        "values": [
            {
                "option": "user",
                "default": "postgres",
            },
            {
                "option": "password",
                "default": "",
                "customizable": True,
                "question": "Please enter postgres password",
            },
        ]
    },
    {
        "name": "mysql",
        "if": "database.engine=mysql",
        "values": [
            {
                "option": "user",
                "default": "root",
            },
            {
                "option": "password",
                "default": make_password,
                "customizable": True,
                "question": "Please enter mysql root password"
            },
            {
                "option": "charset",
                "default": "utf8",
            },
            {
                "option": "collation",
                "default": "utf8_general_ci",
            }
        ]
    },
    {
        "name": "fail2ban",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "config_dir",
                "default": "/etc/fail2ban"
            },
            {
                "option": "max_retry",
                "default": "20"
            },
            {
                "option": "ban_time",
                "default": "3600"
            },
            {
                "option": "find_time",
                "default": "30"
            },
        ]
    },
    {
        "name": "modoboa",
        "values": [
            {
                "option": "user",
                "default": "modoboa",
            },
            {
                "option": "home_dir",
                "default": "/srv/modoboa",
            },
            {
                "option": "venv_path",
                "default": "%(home_dir)s/env",
            },
            {
                "option": "instance_path",
                "default": "%(home_dir)s/instance",
            },
            {
                "option": "timezone",
                "default": "Europe/Paris",
            },
            {
                "option": "dbname",
                "default": "modoboa",
            },
            {
                "option": "dbuser",
                "default": "modoboa",
            },
            {
                "option": "dbpassword",
                "default": make_password,
                "customizable": True,
                "question": "Please enter Modoboa db password",
            },
            {
                "option": "extensions",
                "default": (
                    "modoboa-amavis modoboa-pdfcredentials "
                    "modoboa-postfix-autoreply modoboa-sievefilters "
                    "modoboa-webmail modoboa-contacts "
                    "modoboa-radicale"
                ),
            },
            {
                "option": "devmode",
                "default": "false",
            },
        ]
    },
    {
        "name": "automx",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "user",
                "default": "automx",
            },
            {
                "option": "config_dir",
                "default": "/etc",
            },
            {
                "option": "home_dir",
                "default": "/srv/automx",
            },
            {
                "option": "venv_path",
                "default": "%(home_dir)s/env",
            },
            {
                "option": "instance_path",
                "default": "%(home_dir)s/instance",
            },
        ]
    },
    {
        "name": "amavis",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "user",
                "default": "amavis",
            },
            {
                "option": "max_servers",
                "default": "2",
            },
            {
                "option": "dbname",
                "default": "amavis",
            },
            {
                "option": "dbuser",
                "default": "amavis",
            },
            {
                "option": "dbpassword",
                "default": make_password,
                "customizable": True,
                "question": "Please enter amavis db password"
            },
        ],
    },
    {
        "name": "clamav",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "user",
                "default": "clamav",
            },
        ]
    },
    {
        "name": "dovecot",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "config_dir",
                "default": "/etc/dovecot",
            },
            {
                "option": "user",
                "default": "dovecot",
            },
            {
                "option": "home_dir",
                "default": "/srv/vmail",
            },
            {
                "option": "mailboxes_owner",
                "default": "vmail",
            },
            {
                "option": "extra_protocols",
                "default": "",
            },
            {
                "option": "postmaster_address",
                "default": "postmaster@%(domain)s",
            },
            {
                "option": "radicale_auth_socket_path",
                "default": "/var/run/dovecot/auth-radicale"
            },
        ]
    },
    {
        "name": "nginx",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "config_dir",
                "default": "/etc/nginx",
            },
        ],
    },
    {
        "name": "razor",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "config_dir",
                "default": "/etc/razor",
            },
        ]
    },
    {
        "name": "postfix",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "config_dir",
                "default": "/etc/postfix",
            },
            {
                "option": "message_size_limit",
                "default": "11534336",
            },
        ]
    },
    {
        "name": "postwhite",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "config_dir",
                "default": "/etc",
            },
        ]
    },
    {
        "name": "spamassassin",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "config_dir",
                "default": "/etc/mail/spamassassin",
            },
            {
                "option": "dbname",
                "default": "spamassassin",
            },
            {
                "option": "dbuser",
                "default": "spamassassin",
            },
            {
                "option": "dbpassword",
                "default": make_password,
                "customizable": True,
                "question": "Please enter spamassassin db password"
            },
        ]
    },
    {
        "name": "uwsgi",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "config_dir",
                "default": "/etc/uwsgi",
            },
            {
                "option": "nb_processes",
                "default": "2",
            },
        ]
    },
    {
        "name": "radicale",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "user",
                "default": "radicale",
            },
            {
                "option": "config_dir",
                "default": "/etc/radicale",
            },
            {
                "option": "home_dir",
                "default": "/srv/radicale",
            },
            {
                "option": "venv_path",
                "default": "%(home_dir)s/env",
            }
        ]
    },
    {
        "name": "opendkim",
        "values": [
            {
                "option": "enabled",
                "default": "true",
            },
            {
                "option": "user",
                "default": "opendkim",
            },
            {
                "option": "config_dir",
                "default": "/etc",
            },
            {
                "option": "port",
                "default": "12345"
            },
            {
                "option": "keys_storage_dir",
                "default": "/var/lib/dkim"
            },
            {
                "option": "dbuser",
                "default": "opendkim",
            },
            {
                "option": "dbpassword",
                "default": make_password,
                "customizable": True,
                "question": "Please enter OpenDKIM db password"
            },

        ]
    },
    {
        "name": "backup",
        "values": [
            {
                "option": "default_path",
                "default": DEFAULT_BACKUP_DIRECTORY
            }
        ]
    }
]