import os
import pwd
import sys
import ConfigParser
from string import ascii_letters, digits

def mk_boolean(v):
    return v is not None and str(v).lower() in ["true", "t", "y", "1", "yes"]

def get_config(z, x, y, v, w, b=False, i=False, f=False, l=False):
    q = _get_config(z, x, y, v, w)
    if b:
        return mk_boolean(q)
    if q and i:
        return int(q)
    if q and f:
        return float(q)
    if q and l:
        return [o.strip() for o in q.split(',')]
    return q

def _get_config(z, x, y, v, w):
    if v:
        u = os.environ.get(v, None)
        if u:
            return u
    if z:
        try:
            return z.get(x, y, raw=True)
        except:
            return w
    return w

def load_config_file():
    c = ConfigParser.ConfigParser()
    a = os.getenv("ANSIBLE_CONFIG", None)
    if a:
        a = os.path.expanduser(a)
    b = os.getcwd() + "/ansible.cfg"
    d = os.path.expanduser("~/.ansible.cfg")
    e = "/etc/ansible/ansible.cfg"
    for f in [a, b, d, e]:
        if f and os.path.exists(f):
            c.read(f)
            return c
    return None

def shell_expand_path(p):
    if p:
        p = os.path.expanduser(p)
    return p

def getActiveUser():
    return pwd.getpwuid(os.geteuid())[0]

def getDistModulePath():
    if getattr(sys, "real_prefix", None):
        return os.path.join(sys.prefix, 'share/ansible/')
    return '/usr/share/ansible/'

def getYamlExtensions():
    return ["", ".yml", ".yaml", ".json"]

p = load_config_file()
active_user = getActiveUser()
DIST_MODULE_PATH = getDistModulePath()
YAML_FILENAME_EXTENSIONS = getYamlExtensions()
DEFAULTS = 'defaults'

def getDefaultHostList():
    return shell_expand_path(get_config(p, DEFAULTS, 'hostfile', 'ANSIBLE_HOSTS', '/etc/ansible/hosts'))

def getDefaultModulePath():
    return get_config(p, DEFAULTS, 'library', 'ANSIBLE_LIBRARY', DIST_MODULE_PATH)

def getDefaultRolesPath():
    return shell_expand_path(get_config(p, DEFAULTS, 'roles_path', 'ANSIBLE_ROLES_PATH', '/etc/ansible/roles'))

def getDefaultRemoteTmp():
    return shell_expand_path(get_config(p, DEFAULTS, 'remote_tmp', 'ANSIBLE_REMOTE_TEMP', '$HOME/.ansible/tmp'))

def getDefaultModuleName():
    return get_config(p, DEFAULTS, 'module_name', None, 'command')

def getDefaultPattern():
    return get_config(p, DEFAULTS, 'pattern', None, '*')

def getDefaultForks():
    return get_config(p, DEFAULTS, 'forks', 'ANSIBLE_FORKS', 5, integer=True)

def getDefaultModuleArgs():
    return get_config(p, DEFAULTS, 'module_args', 'ANSIBLE_MODULE_ARGS', '')

def getDefaultModuleLang():
    return get_config(p, DEFAULTS, 'module_lang', 'ANSIBLE_MODULE_LANG', 'en_US.UTF-8')

def getDefaultTimeout():
    return get_config(p, DEFAULTS, 'timeout', 'ANSIBLE_TIMEOUT', 10, integer=True)

def getDefaultPollInterval():
    return get_config(p, DEFAULTS, 'poll_interval', 'ANSIBLE_POLL_INTERVAL', 15, integer=True)

def getDefaultRemoteUser():
    return get_config(p, DEFAULTS, 'remote_user', 'ANSIBLE_REMOTE_USER', active_user)

def getDefaultAskPass():
    return get_config(p, DEFAULTS, 'ask_pass', 'ANSIBLE_ASK_PASS', False, boolean=True)

def getDefaultPrivateKeyFile():
    return shell_expand_path(get_config(p, DEFAULTS, 'private_key_file', 'ANSIBLE_PRIVATE_KEY_FILE', None))

def getDefaultSudoUser():
    return get_config(p, DEFAULTS, 'sudo_user', 'ANSIBLE_SUDO_USER', 'root')

def getDefaultAskSudoPass():
    return get_config(p, DEFAULTS, 'ask_sudo_pass', 'ANSIBLE_ASK_SUDO_PASS', False, boolean=True)

def getDefaultRemotePort():
    return get_config(p, DEFAULTS, 'remote_port', 'ANSIBLE_REMOTE_PORT', None, integer=True)

def getDefaultAskVaultPass():
    return get_config(p, DEFAULTS, 'ask_vault_pass', 'ANSIBLE_ASK_VAULT_PASS', False, boolean=True)

def getDefaultTransport():
    return get_config(p, DEFAULTS, 'transport', 'ANSIBLE_TRANSPORT', 'smart')

def getDefaultScpIfSsh():
    return get_config(p, 'ssh_connection', 'scp_if_ssh', 'ANSIBLE_SCP_IF_SSH', False, boolean=True)

def getDefaultManagedStr():
    return get_config(p, DEFAULTS, 'ansible_managed', None, 'Ansible managed: {file} modified on %Y-%m-%d %H:%M:%S by {uid} on {host}')

def getDefaultSyslogFacility():
    return get_config(p, DEFAULTS, 'syslog_facility', 'ANSIBLE_SYSLOG_FACILITY', 'LOG_USER')

def getDefaultKeepRemoteFiles():
    return get_config(p, DEFAULTS, 'keep_remote_files', 'ANSIBLE_KEEP_REMOTE_FILES', False, boolean=True)

def getDefaultSudo():
    return get_config(p, DEFAULTS, 'sudo', 'ANSIBLE_SUDO', False, boolean=True)

def getDefaultSudoExe():
    return get_config(p, DEFAULTS, 'sudo_exe', 'ANSIBLE_SUDO_EXE', 'sudo')

def getDefaultSudoFlags():
    return get_config(p, DEFAULTS, 'sudo_flags', 'ANSIBLE_SUDO_FLAGS', '-H')

def getDefaultHashBehaviour():
    return get_config(p, DEFAULTS, 'hash_behaviour', 'ANSIBLE_HASH_BEHAVIOUR', 'replace')

def getDefaultJinja2Extensions():
    return get_config(p, DEFAULTS, 'jinja2_extensions', 'ANSIBLE_JINJA2_EXTENSIONS', None)

def getDefaultExecutable():
    return get_config(p, DEFAULTS, 'executable', 'ANSIBLE_EXECUTABLE', '/bin/sh')

def getDefaultSuExe():
    return get_config(p, DEFAULTS, 'su_exe', 'ANSIBLE_SU_EXE', 'su')

def getDefaultSu():
    return get_config(p, DEFAULTS, 'su', 'ANSIBLE_SU', False, boolean=True)

def getDefaultSuFlags():
    return get_config(p, DEFAULTS, 'su_flags', 'ANSIBLE_SU_FLAGS', '')

def getDefaultSuUser():
    return get_config(p, DEFAULTS, 'su_user', 'ANSIBLE_SU_USER', 'root')

def getDefaultAskSuPass():
    return get_config(p, DEFAULTS, 'ask_su_pass', 'ANSIBLE_ASK_SU_PASS', False, boolean=True)

def getDefaultGathering():
    return get_config(p, DEFAULTS, 'gathering', 'ANSIBLE_GATHERING', 'implicit').lower()

def getDefaultActionPluginPath():
    return get_config(p, DEFAULTS, 'action_plugins', 'ANSIBLE_ACTION_PLUGINS', '/usr/share/ansible_plugins/action_plugins')

def getDefaultCallbackPluginPath():
    return get_config(p, DEFAULTS, 'callback_plugins', 'ANSIBLE_CALLBACK_PLUGINS', '/usr/share/ansible_plugins/callback_plugins')

def getDefaultConnectionPluginPath():
    return get_config(p, DEFAULTS, 'connection_plugins', 'ANSIBLE_CONNECTION_PLUGINS', '/usr/share/ansible_plugins/connection_plugins')

def getDefaultLookupPluginPath():
    return get_config(p, DEFAULTS, 'lookup_plugins', 'ANSIBLE_LOOKUP_PLUGINS', '/usr/share/ansible_plugins/lookup_plugins')

def getDefaultVarsPluginPath():
    return get_config(p, DEFAULTS, 'vars_plugins', 'ANSIBLE_VARS_PLUGINS', '/usr/share/ansible_plugins/vars_plugins')

def getDefaultFilterPluginPath():
    return get_config(p, DEFAULTS, 'filter_plugins', 'ANSIBLE_FILTER_PLUGINS', '/usr/share/ansible_plugins/filter_plugins')

def getDefaultLogPath():
    return shell_expand_path(get_config(p, DEFAULTS, 'log_path', 'ANSIBLE_LOG_PATH', ''))

def getAnsibleForceColor():
    return get_config(p, DEFAULTS, 'force_color', 'ANSIBLE_FORCE_COLOR', None, boolean=True)

def getAnsibleNoColor():
    return get_config(p, DEFAULTS, 'nocolor', 'ANSIBLE_NOCOLOR', None, boolean=True)

def getAnsibleNoCows():
    return get_config(p, DEFAULTS, 'nocows', 'ANSIBLE_NOCOWS', None, boolean=True)

def getDisplaySkippedHosts():
    return get_config(p, DEFAULTS, 'display_skipped_hosts', 'DISPLAY_SKIPPED_HOSTS', True, boolean=True)

def getDefaultUndefinedVarBehavior():
    return get_config(p, DEFAULTS, 'error_on_undefined_vars', 'ANSIBLE_ERROR_ON_UNDEFINED_VARS', True, boolean=True)

def getHostKeyChecking():
    return get_config(p, DEFAULTS, 'host_key_checking', 'ANSIBLE_HOST_KEY_CHECKING', True, boolean=True)

def getSystemWarnings():
    return get_config(p, DEFAULTS, 'system_warnings', 'ANSIBLE_SYSTEM_WARNINGS', True, boolean=True)

def getDeprecationWarnings():
    return get_config(p, DEFAULTS, 'deprecation_warnings', 'ANSIBLE_DEPRECATION_WARNINGS', True, boolean=True)

def getDefaultCallableWhitelist():
    return get_config(p, DEFAULTS, 'callable_whitelist', 'ANSIBLE_CALLABLE_WHITELIST', [], islist=True)

def getAnsibleSshArgs():
    return get_config(p, 'ssh_connection', 'ssh_args', 'ANSIBLE_SSH_ARGS', None)

def getAnsibleSshControlPath():
    return get_config(p, 'ssh_connection', 'control_path', 'ANSIBLE_SSH_CONTROL_PATH', "%(directory)s/ansible-ssh-%%h-%%p-%%r")

def getAnsibleSshPipelining():
    return get_config(p, 'ssh_connection', 'pipelining', 'ANSIBLE_SSH_PIPELINING', False, boolean=True)

def getParamikoRecordHostKeys():
    return get_config(p, 'paramiko_connection', 'record_host_keys', 'ANSIBLE_PARAMIKO_RECORD_HOST_KEYS', True, boolean=True)

def getZeroMqPort():
    return get_config(p, 'fireball_connection', 'zeromq_port', 'ANSIBLE_ZEROMQ_PORT', 5099, integer=True)

def getAcceleratePort():
    return get_config(p, 'accelerate', 'accelerate_port', 'ACCELERATE_PORT', 5099, integer=True)

def getAccelerateTimeout():
    return get_config(p, 'accelerate', 'accelerate_timeout', 'ACCELERATE_TIMEOUT', 30, integer=True)

def getAccelerateConnectTimeout():
    return get_config(p, 'accelerate', 'accelerate_connect_timeout', 'ACCELERATE_CONNECT_TIMEOUT', 1.0, floating=True)

def getAccelerateDaemonTimeout():
    return get_config(p, 'accelerate', 'accelerate_daemon_timeout', 'ACCELERATE_DAEMON_TIMEOUT', 30, integer=True)

def getAccelerateKeysDir():
    return get_config(p, 'accelerate', 'accelerate_keys_dir', 'ACCELERATE_KEYS_DIR', '~/.fireball.keys')

def getAccelerateKeysDirPerms():
    return get_config(p, 'accelerate', 'accelerate_keys_dir_perms', 'ACCELERATE_KEYS_DIR_PERMS', '700')

def getAccelerateKeysFilePerms():
    return get_config(p, 'accelerate', 'accelerate_keys_file_perms', 'ACCELERATE_KEYS_FILE_PERMS', '600')

def getAccelerateMultiKey():
    return get_config(p, 'accelerate', 'accelerate_multi_key', 'ACCELERATE_MULTI_KEY', False, boolean=True)

def getParamikoPty():
    return get_config(p, 'paramiko_connection', 'pty', 'ANSIBLE_PARAMIKO_PTY', True, boolean=True)

def getDefaultPasswordChars():
    return ascii_letters + digits + ".,:-_"

DEFAULT_HOST_LIST = getDefaultHostList()
DEFAULT_MODULE_PATH = getDefaultModulePath()
DEFAULT_ROLES_PATH = getDefaultRolesPath()
DEFAULT_REMOTE_TMP = getDefaultRemoteTmp()
DEFAULT_MODULE_NAME = getDefaultModuleName()
DEFAULT_PATTERN = getDefaultPattern()
DEFAULT_FORKS = getDefaultForks()
DEFAULT_MODULE_ARGS = getDefaultModuleArgs()
DEFAULT_MODULE_LANG = getDefaultModuleLang()
DEFAULT_TIMEOUT = getDefaultTimeout()
DEFAULT_POLL_INTERVAL = getDefaultPollInterval()
DEFAULT_REMOTE_USER = getDefaultRemoteUser()
DEFAULT_ASK_PASS = getDefaultAskPass()
DEFAULT_PRIVATE_KEY_FILE = getDefaultPrivateKeyFile()
DEFAULT_SUDO_USER = getDefaultSudoUser()
DEFAULT_ASK_SUDO_PASS = getDefaultAskSudoPass()
DEFAULT_REMOTE_PORT = getDefaultRemotePort()
DEFAULT_ASK_VAULT_PASS = getDefaultAskVaultPass()
DEFAULT_TRANSPORT = getDefaultTransport()
DEFAULT_SCP_IF_SSH = getDefaultScpIfSsh()
DEFAULT_MANAGED_STR = getDefaultManagedStr()
DEFAULT_SYSLOG_FACILITY = getDefaultSyslogFacility()
DEFAULT_KEEP_REMOTE_FILES = getDefaultKeepRemoteFiles()
DEFAULT_SUDO = getDefaultSudo()
DEFAULT_SUDO_EXE = getDefaultSudoExe()
DEFAULT_SUDO_FLAGS = getDefaultSudoFlags()
DEFAULT_HASH_BEHAVIOUR = getDefaultHashBehaviour()
DEFAULT_JINJA2_EXTENSIONS = getDefaultJinja2Extensions()
DEFAULT_EXECUTABLE = getDefaultExecutable()
DEFAULT_SU_EXE = getDefaultSuExe()
DEFAULT_SU = getDefaultSu()
DEFAULT_SU_FLAGS = getDefaultSuFlags()
DEFAULT_SU_USER = getDefaultSuUser()
DEFAULT_ASK_SU_PASS = getDefaultAskSuPass()
DEFAULT_GATHERING = getDefaultGathering()
DEFAULT_ACTION_PLUGIN_PATH = getDefaultActionPluginPath()
DEFAULT_CALLBACK_PLUGIN_PATH = getDefaultCallbackPluginPath()
DEFAULT_CONNECTION_PLUGIN_PATH = getDefaultConnectionPluginPath()
DEFAULT_LOOKUP_PLUGIN_PATH = getDefaultLookupPluginPath()
DEFAULT_VARS_PLUGIN_PATH = getDefaultVarsPluginPath()
DEFAULT_FILTER_PLUGIN_PATH = getDefaultFilterPluginPath()
DEFAULT_LOG_PATH = getDefaultLogPath()
ANSIBLE_FORCE_COLOR = getAnsibleForceColor()
ANSIBLE_NOCOLOR = getAnsibleNoColor()
ANSIBLE_NOCOWS = getAnsibleNoCows()
DISPLAY_SKIPPED_HOSTS = getDisplaySkippedHosts()
DEFAULT_UNDEFINED_VAR_BEHAVIOR = getDefaultUndefinedVarBehavior()
HOST_KEY_CHECKING = getHostKeyChecking()
SYSTEM_WARNINGS = getSystemWarnings()
DEPRECATION_WARNINGS = getDeprecationWarnings()
DEFAULT_CALLABLE_WHITELIST = getDefaultCallableWhitelist()
ANSIBLE_SSH_ARGS = getAnsibleSshArgs()
ANSIBLE_SSH_CONTROL_PATH = getAnsibleSshControlPath()
ANSIBLE_SSH_PIPELINING = getAnsibleSshPipelining()
PARAMIKO_RECORD_HOST_KEYS = getParamikoRecordHostKeys()
ZEROMQ_PORT = getZeroMqPort()
ACCELERATE_PORT = getAcceleratePort()
ACCELERATE_TIMEOUT = getAccelerateTimeout()
ACCELERATE_CONNECT_TIMEOUT = getAccelerateConnectTimeout()
ACCELERATE_DAEMON_TIMEOUT = getAccelerateDaemonTimeout()
ACCELERATE_KEYS_DIR = getAccelerateKeysDir()
ACCELERATE_KEYS_DIR_PERMS = getAccelerateKeysDirPerms()
ACCELERATE_KEYS_FILE_PERMS = getAccelerateKeysFilePerms()
ACCELERATE_MULTI_KEY = getAccelerateMultiKey()
PARAMIKO_PTY = getParamikoPty()
DEFAULT_PASSWORD_CHARS = getDefaultPasswordChars()

DEFAULT_SUDO_PASS = None
DEFAULT_REMOTE_PASS = None
DEFAULT_SUBSET = None
DEFAULT_SU_PASS = None
VAULT_VERSION_MIN = 1.0
VAULT_VERSION_MAX = 1.0