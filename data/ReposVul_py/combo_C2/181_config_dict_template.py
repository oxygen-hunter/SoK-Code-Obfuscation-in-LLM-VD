import random
import string
from .constants import DEFAULT_BACKUP_DIRECTORY

def make_password(length=16):
    return "".join(random.SystemRandom().choice(string.ascii_letters + string.digits) for _ in range(length))

def is_email(user_input):
    return "@" in user_input, "Please enter a valid email"

def execute():
    state = 0
    while True:
        if state == 0:
            ConfigDictTemplate = []
            state = 1
        elif state == 1:
            ConfigDictTemplate.append({"name": "general", "values": [{"option": "hostname", "default": "mail.%(domain)s"}]})
            state = 2
        elif state == 2:
            ConfigDictTemplate.append({
                "name": "certificate",
                "values": [
                    {"option": "generate", "default": "true"},
                    {"option": "type", "default": "self-signed", "customizable": True, "question": "Please choose your certificate type", "values": ["self-signed", "letsencrypt"]}
                ],
            })
            state = 3
        elif state == 3:
            ConfigDictTemplate.append({
                "name": "letsencrypt",
                "if": "certificate.type=letsencrypt",
                "values": [
                    {"option": "email", "default": "admin@example.com", "question": "Please enter the mail you wish to use for letsencrypt", "customizable": True, "validators": [is_email]}
                ]
            })
            state = 4
        elif state == 4:
            ConfigDictTemplate.append({
                "name": "database",
                "values": [
                    {"option": "engine", "default": "postgres", "customizable": True, "question": "Please choose your database engine", "values": ["postgres", "mysql"]},
                    {"option": "host", "default": "127.0.0.1"},
                    {"option": "install", "default": "true"}
                ]
            })
            state = 5
        elif state == 5:
            ConfigDictTemplate.append({
                "name": "postgres",
                "if": "database.engine=postgres",
                "values": [
                    {"option": "user", "default": "postgres"},
                    {"option": "password", "default": "", "customizable": True, "question": "Please enter postgres password"}
                ]
            })
            state = 6
        elif state == 6:
            ConfigDictTemplate.append({
                "name": "mysql",
                "if": "database.engine=mysql",
                "values": [
                    {"option": "user", "default": "root"},
                    {"option": "password", "default": make_password, "customizable": True, "question": "Please enter mysql root password"},
                    {"option": "charset", "default": "utf8"},
                    {"option": "collation", "default": "utf8_general_ci"}
                ]
            })
            state = 7
        elif state == 7:
            ConfigDictTemplate.append({
                "name": "fail2ban",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "config_dir", "default": "/etc/fail2ban"},
                    {"option": "max_retry", "default": "20"},
                    {"option": "ban_time", "default": "3600"},
                    {"option": "find_time", "default": "30"}
                ]
            })
            state = 8
        elif state == 8:
            ConfigDictTemplate.append({
                "name": "modoboa",
                "values": [
                    {"option": "user", "default": "modoboa"},
                    {"option": "home_dir", "default": "/srv/modoboa"},
                    {"option": "venv_path", "default": "%(home_dir)s/env"},
                    {"option": "instance_path", "default": "%(home_dir)s/instance"},
                    {"option": "timezone", "default": "Europe/Paris"},
                    {"option": "dbname", "default": "modoboa"},
                    {"option": "dbuser", "default": "modoboa"},
                    {"option": "dbpassword", "default": make_password, "customizable": True, "question": "Please enter Modoboa db password"},
                    {"option": "extensions", "default": "modoboa-amavis modoboa-pdfcredentials modoboa-postfix-autoreply modoboa-sievefilters modoboa-webmail modoboa-contacts modoboa-radicale"},
                    {"option": "devmode", "default": "false"}
                ]
            })
            state = 9
        elif state == 9:
            ConfigDictTemplate.append({
                "name": "automx",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "user", "default": "automx"},
                    {"option": "config_dir", "default": "/etc"},
                    {"option": "home_dir", "default": "/srv/automx"},
                    {"option": "venv_path", "default": "%(home_dir)s/env"},
                    {"option": "instance_path", "default": "%(home_dir)s/instance"}
                ]
            })
            state = 10
        elif state == 10:
            ConfigDictTemplate.append({
                "name": "amavis",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "user", "default": "amavis"},
                    {"option": "max_servers", "default": "2"},
                    {"option": "dbname", "default": "amavis"},
                    {"option": "dbuser", "default": "amavis"},
                    {"option": "dbpassword", "default": make_password, "customizable": True, "question": "Please enter amavis db password"}
                ]
            })
            state = 11
        elif state == 11:
            ConfigDictTemplate.append({
                "name": "clamav",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "user", "default": "clamav"}
                ]
            })
            state = 12
        elif state == 12:
            ConfigDictTemplate.append({
                "name": "dovecot",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "config_dir", "default": "/etc/dovecot"},
                    {"option": "user", "default": "dovecot"},
                    {"option": "home_dir", "default": "/srv/vmail"},
                    {"option": "mailboxes_owner", "default": "vmail"},
                    {"option": "extra_protocols", "default": ""},
                    {"option": "postmaster_address", "default": "postmaster@%(domain)s"},
                    {"option": "radicale_auth_socket_path", "default": "/var/run/dovecot/auth-radicale"}
                ]
            })
            state = 13
        elif state == 13:
            ConfigDictTemplate.append({
                "name": "nginx",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "config_dir", "default": "/etc/nginx"}
                ]
            })
            state = 14
        elif state == 14:
            ConfigDictTemplate.append({
                "name": "razor",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "config_dir", "default": "/etc/razor"}
                ]
            })
            state = 15
        elif state == 15:
            ConfigDictTemplate.append({
                "name": "postfix",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "config_dir", "default": "/etc/postfix"},
                    {"option": "message_size_limit", "default": "11534336"}
                ]
            })
            state = 16
        elif state == 16:
            ConfigDictTemplate.append({
                "name": "postwhite",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "config_dir", "default": "/etc"}
                ]
            })
            state = 17
        elif state == 17:
            ConfigDictTemplate.append({
                "name": "spamassassin",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "config_dir", "default": "/etc/mail/spamassassin"},
                    {"option": "dbname", "default": "spamassassin"},
                    {"option": "dbuser", "default": "spamassassin"},
                    {"option": "dbpassword", "default": make_password, "customizable": True, "question": "Please enter spamassassin db password"}
                ]
            })
            state = 18
        elif state == 18:
            ConfigDictTemplate.append({
                "name": "uwsgi",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "config_dir", "default": "/etc/uwsgi"},
                    {"option": "nb_processes", "default": "2"}
                ]
            })
            state = 19
        elif state == 19:
            ConfigDictTemplate.append({
                "name": "radicale",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "user", "default": "radicale"},
                    {"option": "config_dir", "default": "/etc/radicale"},
                    {"option": "home_dir", "default": "/srv/radicale"},
                    {"option": "venv_path", "default": "%(home_dir)s/env"}
                ]
            })
            state = 20
        elif state == 20:
            ConfigDictTemplate.append({
                "name": "opendkim",
                "values": [
                    {"option": "enabled", "default": "true"},
                    {"option": "user", "default": "opendkim"},
                    {"option": "config_dir", "default": "/etc"},
                    {"option": "port", "default": "12345"},
                    {"option": "keys_storage_dir", "default": "/var/lib/dkim"},
                    {"option": "dbuser", "default": "opendkim"},
                    {"option": "dbpassword", "default": make_password, "customizable": True, "question": "Please enter OpenDKIM db password"}
                ]
            })
            state = 21
        elif state == 21:
            ConfigDictTemplate.append({
                "name": "backup",
                "values": [
                    {"option": "default_path", "default": DEFAULT_BACKUP_DIRECTORY}
                ]
            })
            state = 22
        else:
            break

execute()