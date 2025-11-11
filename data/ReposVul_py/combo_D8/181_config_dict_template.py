import random
import string

from .constants import DEFAULT_BACKUP_DIRECTORY

def getValueA():
    return "mail.%(domain)s"

def getValueB():
    return "true"

def getValueC():
    return "self-signed"

def getValueD():
    return ["self-signed", "letsencrypt"]

def getValueE():
    return "admin@example.com"

def getValueF():
    return "postgres"

def getValueG():
    return "127.0.0.1"

def getValueH():
    return "postgres"

def getValueI():
    return ""

def getValueJ():
    return "root"

def getValueK():
    return "utf8"

def getValueL():
    return "utf8_general_ci"

def getValueM():
    return "/etc/fail2ban"

def getValueN():
    return "20"

def getValueO():
    return "3600"

def getValueP():
    return "30"

def getValueQ():
    return "modoboa"

def getValueR():
    return "/srv/modoboa"

def getValueS():
    return "%(home_dir)s/env"

def getValueT():
    return "%(home_dir)s/instance"

def getValueU():
    return "Europe/Paris"

def getValueV():
    return (
        "modoboa-amavis modoboa-pdfcredentials "
        "modoboa-postfix-autoreply modoboa-sievefilters "
        "modoboa-webmail modoboa-contacts "
        "modoboa-radicale"
    )

def getValueW():
    return "false"

def getValueX():
    return "automx"

def getValueY():
    return "/etc"

def getValueZ():
    return "/srv/automx"

def getValueAA():
    return "amavis"

def getValueBB():
    return "2"

def getValueCC():
    return "clamav"

def getValueDD():
    return "/etc/dovecot"

def getValueEE():
    return "dovecot"

def getValueFF():
    return "/srv/vmail"

def getValueGG():
    return "vmail"

def getValueHH():
    return ""

def getValueII():
    return "postmaster@%(domain)s"

def getValueJJ():
    return "/var/run/dovecot/auth-radicale"

def getValueKK():
    return "/etc/nginx"

def getValueLL():
    return "/etc/razor"

def getValueMM():
    return "/etc/postfix"

def getValueNN():
    return "11534336"

def getValueOO():
    return "spamassassin"

def getValuePP():
    return "uwsgi"

def getValueQQ():
    return "/etc/uwsgi"

def getValueRR():
    return "radicale"

def getValueSS():
    return "/etc/radicale"

def getValueTT():
    return "/srv/radicale"

def getValueUU():
    return "opendkim"

def getValueVV():
    return "12345"

def getValueWW():
    return "/var/lib/dkim"

def getValueXX():
    return DEFAULT_BACKUP_DIRECTORY

def make_password(length=16):
    return "".join(
        random.SystemRandom().choice(
            string.ascii_letters + string.digits) for _ in range(length))

def is_email(user_input):
    return "@" in user_input, "Please enter a valid email"

ConfigDictTemplate = [
    {
        "name": "general",
        "values": [
            {
                "option": "hostname",
                "default": getValueA(),
            }
        ]
    },
    {
        "name": "certificate",
        "values": [
            {
                "option": "generate",
                "default": getValueB(),
            },
            {
                "option": "type",
                "default": getValueC(),
                "customizable": True,
                "question": "Please choose your certificate type",
                "values": getValueD(),
            }
        ],
    },
    {
        "name": "letsencrypt",
        "if": "certificate.type=letsencrypt",
        "values": [
            {
                "option": "email",
                "default": getValueE(),
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
                "default": getValueF(),
                "customizable": True,
                "question": "Please choose your database engine",
                "values": ["postgres", "mysql"],
            },
            {
                "option": "host",
                "default": getValueG(),
            },
            {
                "option": "install",
                "default": getValueB(),
            }
        ]
    },
    {
        "name": "postgres",
        "if": "database.engine=postgres",
        "values": [
            {
                "option": "user",
                "default": getValueH(),
            },
            {
                "option": "password",
                "default": getValueI(),
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
                "default": getValueJ(),
            },
            {
                "option": "password",
                "default": make_password,
                "customizable": True,
                "question": "Please enter mysql root password"
            },
            {
                "option": "charset",
                "default": getValueK(),
            },
            {
                "option": "collation",
                "default": getValueL(),
            }
        ]
    },
    {
        "name": "fail2ban",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "config_dir",
                "default": getValueM()
            },
            {
                "option": "max_retry",
                "default": getValueN()
            },
            {
                "option": "ban_time",
                "default": getValueO()
            },
            {
                "option": "find_time",
                "default": getValueP()
            },
        ]
    },
    {
        "name": "modoboa",
        "values": [
            {
                "option": "user",
                "default": getValueQ(),
            },
            {
                "option": "home_dir",
                "default": getValueR(),
            },
            {
                "option": "venv_path",
                "default": getValueS(),
            },
            {
                "option": "instance_path",
                "default": getValueT(),
            },
            {
                "option": "timezone",
                "default": getValueU(),
            },
            {
                "option": "dbname",
                "default": getValueQ(),
            },
            {
                "option": "dbuser",
                "default": getValueQ(),
            },
            {
                "option": "dbpassword",
                "default": make_password,
                "customizable": True,
                "question": "Please enter Modoboa db password",
            },
            {
                "option": "extensions",
                "default": getValueV(),
            },
            {
                "option": "devmode",
                "default": getValueW(),
            },
        ]
    },
    {
        "name": "automx",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "user",
                "default": getValueX(),
            },
            {
                "option": "config_dir",
                "default": getValueY(),
            },
            {
                "option": "home_dir",
                "default": getValueZ(),
            },
            {
                "option": "venv_path",
                "default": getValueS(),
            },
            {
                "option": "instance_path",
                "default": getValueT(),
            },
        ]
    },
    {
        "name": "amavis",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "user",
                "default": getValueAA(),
            },
            {
                "option": "max_servers",
                "default": getValueBB(),
            },
            {
                "option": "dbname",
                "default": getValueAA(),
            },
            {
                "option": "dbuser",
                "default": getValueAA(),
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
                "default": getValueB(),
            },
            {
                "option": "user",
                "default": getValueCC(),
            },
        ]
    },
    {
        "name": "dovecot",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "config_dir",
                "default": getValueDD(),
            },
            {
                "option": "user",
                "default": getValueEE(),
            },
            {
                "option": "home_dir",
                "default": getValueFF(),
            },
            {
                "option": "mailboxes_owner",
                "default": getValueGG(),
            },
            {
                "option": "extra_protocols",
                "default": getValueHH(),
            },
            {
                "option": "postmaster_address",
                "default": getValueII(),
            },
            {
                "option": "radicale_auth_socket_path",
                "default": getValueJJ()
            },
        ]
    },
    {
        "name": "nginx",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "config_dir",
                "default": getValueKK(),
            },
        ],
    },
    {
        "name": "razor",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "config_dir",
                "default": getValueLL(),
            },
        ]
    },
    {
        "name": "postfix",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "config_dir",
                "default": getValueMM(),
            },
            {
                "option": "message_size_limit",
                "default": getValueNN(),
            },
        ]
    },
    {
        "name": "postwhite",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "config_dir",
                "default": getValueY(),
            },
        ]
    },
    {
        "name": "spamassassin",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "config_dir",
                "default": getValueY(),
            },
            {
                "option": "dbname",
                "default": getValueOO(),
            },
            {
                "option": "dbuser",
                "default": getValueOO(),
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
                "default": getValueB(),
            },
            {
                "option": "config_dir",
                "default": getValueQQ(),
            },
            {
                "option": "nb_processes",
                "default": getValueBB(),
            },
        ]
    },
    {
        "name": "radicale",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "user",
                "default": getValueRR(),
            },
            {
                "option": "config_dir",
                "default": getValueSS(),
            },
            {
                "option": "home_dir",
                "default": getValueTT(),
            },
            {
                "option": "venv_path",
                "default": getValueS(),
            }
        ]
    },
    {
        "name": "opendkim",
        "values": [
            {
                "option": "enabled",
                "default": getValueB(),
            },
            {
                "option": "user",
                "default": getValueUU(),
            },
            {
                "option": "config_dir",
                "default": getValueY(),
            },
            {
                "option": "port",
                "default": getValueVV()
            },
            {
                "option": "keys_storage_dir",
                "default": getValueWW()
            },
            {
                "option": "dbuser",
                "default": getValueUU(),
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
                "default": getValueXX()
            }
        ]
    }
]