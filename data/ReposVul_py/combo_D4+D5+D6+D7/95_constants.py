import os
import pwd
import sys
import ConfigParser
from string import ascii_letters, digits

def mk_boolean(x):
    if x is None:
        return False
    v = str(x)
    if v.lower() in [ "true", "t", "y", "1", "yes" ]:
        return True
    return False

def get_config(p, a, b, c, d, boolean=False, integer=False, floating=False, islist=False):
    v = _get_config(p, a, b, c, d)
    if boolean:
        return mk_boolean(v)
    if v and integer:
        return int(v)
    if v and floating:
        return float(v)
    if v and islist:
        return [x.strip() for x in v.split(',')]
    return v

def _get_config(p, a, b, c, d):
    if c is not None:
        v = os.environ.get(c, None)
        if v is not None:
            return v
    if p is not None:
        try:
            return p.get(a, b, raw=True)
        except:
            return d
    return d

def load_config_file():
    q = ConfigParser.ConfigParser()

    a = os.getenv("ANSIBLE_CONFIG", None)
    if a is not None:
        a = os.path.expanduser(a)
    b = os.getcwd() + "/ansible.cfg"
    c = os.path.expanduser("~/.ansible.cfg")
    d = "/etc/ansible/ansible.cfg"

    for i in [a, b, c, d]:
        if i is not None and os.path.exists(i):
            q.read(i)
            return q
    return None

def shell_expand_path(i):
    if i:
        i = os.path.expanduser(i)
    return i

p = load_config_file()

v = [pwd.getpwuid(os.geteuid())[0]]

if getattr(sys, "real_prefix", None):
    d = os.path.join(sys.prefix, 'share/ansible/')
else:
    d = '/usr/share/ansible/'

YAML_FILENAME_EXTENSIONS = [ "", ".yml", ".yaml", ".json" ]

DEFAULTS='defaults'

v1 = shell_expand_path(get_config(p, DEFAULTS, 'hostfile', 'ANSIBLE_HOSTS', '/etc/ansible/hosts'))
v2 = get_config(p, DEFAULTS, 'library', 'ANSIBLE_LIBRARY', d)
v3 = shell_expand_path(get_config(p, DEFAULTS, 'roles_path', 'ANSIBLE_ROLES_PATH', '/etc/ansible/roles'))
v4 = shell_expand_path(get_config(p, DEFAULTS, 'remote_tmp', 'ANSIBLE_REMOTE_TEMP', '$HOME/.ansible/tmp'))
v5 = get_config(p, DEFAULTS, 'module_name', None, 'command')
v6 = get_config(p, DEFAULTS, 'pattern', None, '*')
v7 = get_config(p, DEFAULTS, 'forks', 'ANSIBLE_FORKS', 5, integer=True)
v8 = get_config(p, DEFAULTS, 'module_args', 'ANSIBLE_MODULE_ARGS', '')
v9 = get_config(p, DEFAULTS, 'module_lang', 'ANSIBLE_MODULE_LANG', 'en_US.UTF-8')
v10 = get_config(p, DEFAULTS, 'timeout', 'ANSIBLE_TIMEOUT', 10, integer=True)
v11 = get_config(p, DEFAULTS, 'poll_interval', 'ANSIBLE_POLL_INTERVAL', 15, integer=True)
v12 = get_config(p, DEFAULTS, 'remote_user', 'ANSIBLE_REMOTE_USER', v[0])
v13 = get_config(p, DEFAULTS, 'ask_pass', 'ANSIBLE_ASK_PASS', False, boolean=True)
v14 = shell_expand_path(get_config(p, DEFAULTS, 'private_key_file', 'ANSIBLE_PRIVATE_KEY_FILE', None))
v15 = get_config(p, DEFAULTS, 'sudo_user', 'ANSIBLE_SUDO_USER', 'root')
v16 = get_config(p, DEFAULTS, 'ask_sudo_pass', 'ANSIBLE_ASK_SUDO_PASS', False, boolean=True)
v17 = get_config(p, DEFAULTS, 'remote_port', 'ANSIBLE_REMOTE_PORT', None, integer=True)
v18 = get_config(p, DEFAULTS, 'ask_vault_pass', 'ANSIBLE_ASK_VAULT_PASS', False, boolean=True)
v19 = get_config(p, DEFAULTS, 'transport', 'ANSIBLE_TRANSPORT', 'smart')
v20 = get_config(p, 'ssh_connection', 'scp_if_ssh', 'ANSIBLE_SCP_IF_SSH', False, boolean=True)
v21 = get_config(p, DEFAULTS, 'ansible_managed', None, 'Ansible managed: {file} modified on %Y-%m-%d %H:%M:%S by {uid} on {host}')
v22 = get_config(p, DEFAULTS, 'syslog_facility', 'ANSIBLE_SYSLOG_FACILITY', 'LOG_USER')
v23 = get_config(p, DEFAULTS, 'keep_remote_files', 'ANSIBLE_KEEP_REMOTE_FILES', False, boolean=True)
v24 = get_config(p, DEFAULTS, 'sudo', 'ANSIBLE_SUDO', False, boolean=True)
v25 = get_config(p, DEFAULTS, 'sudo_exe', 'ANSIBLE_SUDO_EXE', 'sudo')
v26 = get_config(p, DEFAULTS, 'sudo_flags', 'ANSIBLE_SUDO_FLAGS', '-H')
v27 = get_config(p, DEFAULTS, 'hash_behaviour', 'ANSIBLE_HASH_BEHAVIOUR', 'replace')
v28 = get_config(p, DEFAULTS, 'jinja2_extensions', 'ANSIBLE_JINJA2_EXTENSIONS', None)
v29 = get_config(p, DEFAULTS, 'executable', 'ANSIBLE_EXECUTABLE', '/bin/sh')
v30 = get_config(p, DEFAULTS, 'su_exe', 'ANSIBLE_SU_EXE', 'su')
v31 = get_config(p, DEFAULTS, 'su', 'ANSIBLE_SU', False, boolean=True)
v32 = get_config(p, DEFAULTS, 'su_flags', 'ANSIBLE_SU_FLAGS', '')
v33 = get_config(p, DEFAULTS, 'su_user', 'ANSIBLE_SU_USER', 'root')
v34 = get_config(p, DEFAULTS, 'ask_su_pass', 'ANSIBLE_ASK_SU_PASS', False, boolean=True)
v35 = get_config(p, DEFAULTS, 'gathering', 'ANSIBLE_GATHERING', 'implicit').lower()

v36 = get_config(p, DEFAULTS, 'action_plugins', 'ANSIBLE_ACTION_PLUGINS', '/usr/share/ansible_plugins/action_plugins')
v37 = get_config(p, DEFAULTS, 'callback_plugins', 'ANSIBLE_CALLBACK_PLUGINS', '/usr/share/ansible_plugins/callback_plugins')
v38 = get_config(p, DEFAULTS, 'connection_plugins', 'ANSIBLE_CONNECTION_PLUGINS', '/usr/share/ansible_plugins/connection_plugins')
v39 = get_config(p, DEFAULTS, 'lookup_plugins', 'ANSIBLE_LOOKUP_PLUGINS', '/usr/share/ansible_plugins/lookup_plugins')
v40 = get_config(p, DEFAULTS, 'vars_plugins', 'ANSIBLE_VARS_PLUGINS', '/usr/share/ansible_plugins/vars_plugins')
v41 = get_config(p, DEFAULTS, 'filter_plugins', 'ANSIBLE_FILTER_PLUGINS', '/usr/share/ansible_plugins/filter_plugins')
v42 = shell_expand_path(get_config(p, DEFAULTS, 'log_path', 'ANSIBLE_LOG_PATH', ''))

v43 = get_config(p, DEFAULTS, 'force_color', 'ANSIBLE_FORCE_COLOR', None, boolean=True)
v44 = get_config(p, DEFAULTS, 'nocolor', 'ANSIBLE_NOCOLOR', None, boolean=True)
v45 = get_config(p, DEFAULTS, 'nocows', 'ANSIBLE_NOCOWS', None, boolean=True)
v46 = get_config(p, DEFAULTS, 'display_skipped_hosts', 'DISPLAY_SKIPPED_HOSTS', True, boolean=True)
v47 = get_config(p, DEFAULTS, 'error_on_undefined_vars', 'ANSIBLE_ERROR_ON_UNDEFINED_VARS', True, boolean=True)
v48 = get_config(p, DEFAULTS, 'host_key_checking', 'ANSIBLE_HOST_KEY_CHECKING', True, boolean=True)
v49 = get_config(p, DEFAULTS, 'system_warnings', 'ANSIBLE_SYSTEM_WARNINGS', True, boolean=True)
v50 = get_config(p, DEFAULTS, 'deprecation_warnings', 'ANSIBLE_DEPRECATION_WARNINGS', True, boolean=True)
v51 = get_config(p, DEFAULTS, 'callable_whitelist', 'ANSIBLE_CALLABLE_WHITELIST', [], islist=True)

v52 = get_config(p, 'ssh_connection', 'ssh_args', 'ANSIBLE_SSH_ARGS', None)
v53 = get_config(p, 'ssh_connection', 'control_path', 'ANSIBLE_SSH_CONTROL_PATH', "%(directory)s/ansible-ssh-%%h-%%p-%%r")
v54 = get_config(p, 'ssh_connection', 'pipelining', 'ANSIBLE_SSH_PIPELINING', False, boolean=True)
v55 = get_config(p, 'paramiko_connection', 'record_host_keys', 'ANSIBLE_PARAMIKO_RECORD_HOST_KEYS', True, boolean=True)
v56 = get_config(p, 'fireball_connection', 'zeromq_port', 'ANSIBLE_ZEROMQ_PORT', 5099, integer=True)
v57 = get_config(p, 'accelerate', 'accelerate_port', 'ACCELERATE_PORT', 5099, integer=True)
v58 = get_config(p, 'accelerate', 'accelerate_timeout', 'ACCELERATE_TIMEOUT', 30, integer=True)
v59 = get_config(p, 'accelerate', 'accelerate_connect_timeout', 'ACCELERATE_CONNECT_TIMEOUT', 1.0, floating=True)
v60 = get_config(p, 'accelerate', 'accelerate_daemon_timeout', 'ACCELERATE_DAEMON_TIMEOUT', 30, integer=True)
v61 = get_config(p, 'accelerate', 'accelerate_keys_dir', 'ACCELERATE_KEYS_DIR', '~/.fireball.keys')
v62 = get_config(p, 'accelerate', 'accelerate_keys_dir_perms', 'ACCELERATE_KEYS_DIR_PERMS', '700')
v63 = get_config(p, 'accelerate', 'accelerate_keys_file_perms', 'ACCELERATE_KEYS_FILE_PERMS', '600')
v64 = get_config(p, 'accelerate', 'accelerate_multi_key', 'ACCELERATE_MULTI_KEY', False, boolean=True)
v65 = get_config(p, 'paramiko_connection', 'pty', 'ANSIBLE_PARAMIKO_PTY', True, boolean=True)

DEFAULT_PASSWORD_CHARS = ascii_letters + digits + ".,:-_"

s1 = None
s2 = None
s3 = None
s4 = None
s5 = (1.0, 1.0)