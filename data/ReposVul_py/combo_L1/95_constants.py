import os
import pwd
import sys
import ConfigParser
from string import ascii_letters, digits

def OX1B2A3D4E(value):
    if value is None:
        return False
    val = str(value)
    if val.lower() in [ "true", "t", "y", "1", "yes" ]:
        return True
    else:
        return False

def OX2C3D4E5F(OX3E4F5G6H, OX4F5G6H7I, OX5G6H7I8J, OX6H7I8J9K, OX7I8J9K0A, OX8J9K0A1B=False, OX9K0A1B2C=False, OX0A1B2C3D=False, OXA1B2C3D4=False):
    OXB2C3D4E5 = OX3D4E5F6G(OX3E4F5G6H, OX4F5G6H7I, OX5G6H7I8J, OX6H7I8J9K, OX7I8J9K0A)
    if OX8J9K0A1B:
        return OX1B2A3D4E(OXB2C3D4E5)
    if OXB2C3D4E5 and OX9K0A1B2C:
        return int(OXB2C3D4E5)
    if OXB2C3D4E5 and OX0A1B2C3D:
        return float(OXB2C3D4E5)
    if OXB2C3D4E5 and OXA1B2C3D4:
        return [x.strip() for x in OXB2C3D4E5.split(',')]
    return OXB2C3D4E5

def OX3D4E5F6G(OX3E4F5G6H, OX4F5G6H7I, OX5G6H7I8J, OX6H7I8J9K, OX7I8J9K0A):
    if OX6H7I8J9K is not None:
        OX4F5G6H7I = os.environ.get(OX6H7I8J9K, None)
        if OX4F5G6H7I is not None:
            return OX4F5G6H7I
    if OX3E4F5G6H is not None:
        try:
            return OX3E4F5G6H.get(OX4F5G6H7I, OX5G6H7I8J, raw=True)
        except:
            return OX7I8J9K0A
    return OX7I8J9K0A

def OX4E5F6G7H():
    OX5F6G7H8I = ConfigParser.ConfigParser()

    OX6G7H8I9J = os.getenv("ANSIBLE_CONFIG", None)
    if OX6G7H8I9J is not None:
        OX6G7H8I9J = os.path.expanduser(OX6G7H8I9J)
    OX7H8I9J0A = os.getcwd() + "/ansible.cfg"
    OX8I9J0A1B = os.path.expanduser("~/.ansible.cfg")
    OX9J0A1B2C = "/etc/ansible/ansible.cfg"

    for OX0A1B2C3D in [OX6G7H8I9J, OX7H8I9J0A, OX8I9J0A1B, OX9J0A1B2C]:
        if OX0A1B2C3D is not None and os.path.exists(OX0A1B2C3D):
            OX5F6G7H8I.read(OX0A1B2C3D)
            return OX5F6G7H8I
    return None

def OX5F6G7H8I(OX6G7H8I9J):
    if OX6G7H8I9J:
        OX6G7H8I9J = os.path.expanduser(OX6G7H8I9J)
    return OX6G7H8I9J

OX1E2D3C4B = OX4E5F6G7H()

OX2D3C4B5A = pwd.getpwuid(os.geteuid())[0]

if getattr(sys, "real_prefix", None):
    OX3C4B5A6A = os.path.join(sys.prefix, 'share/ansible/')
else:
    OX3C4B5A6A = '/usr/share/ansible/'

OX4B5A6A7A = [ "", ".yml", ".yaml", ".json" ]

OX5A6A7A8A='defaults'

OX6A7A8A9A = OX5F6G7H8I(OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'hostfile', 'ANSIBLE_HOSTS', '/etc/ansible/hosts'))
OX7A8A9A0A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'library', 'ANSIBLE_LIBRARY', OX3C4B5A6A)
OX8A9A0A1A = OX5F6G7H8I(OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'roles_path', 'ANSIBLE_ROLES_PATH', '/etc/ansible/roles'))
OX9A0A1A2A = OX5F6G7H8I(OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'remote_tmp', 'ANSIBLE_REMOTE_TEMP', '$HOME/.ansible/tmp'))
OX0A1A2A3A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'module_name', None, 'command')
OXA1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'pattern', None, '*')
OXB1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'forks', 'ANSIBLE_FORKS', 5, integer=True)
OXC1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'module_args', 'ANSIBLE_MODULE_ARGS', '')
OXD1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'module_lang', 'ANSIBLE_MODULE_LANG', 'en_US.UTF-8')
OXE1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'timeout', 'ANSIBLE_TIMEOUT', 10, integer=True)
OXF1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'poll_interval', 'ANSIBLE_POLL_INTERVAL', 15, integer=True)
OXG1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'remote_user', 'ANSIBLE_REMOTE_USER', OX2D3C4B5A)
OXH1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'ask_pass', 'ANSIBLE_ASK_PASS', False, boolean=True)
OXI1A2A3A4 = OX5F6G7H8I(OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'private_key_file', 'ANSIBLE_PRIVATE_KEY_FILE', None))
OXJ1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'sudo_user', 'ANSIBLE_SUDO_USER', 'root')
OXK1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'ask_sudo_pass', 'ANSIBLE_ASK_SUDO_PASS', False, boolean=True)
OXL1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'remote_port', 'ANSIBLE_REMOTE_PORT', None, integer=True)
OXM1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'ask_vault_pass', 'ANSIBLE_ASK_VAULT_PASS', False, boolean=True)
OXN1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'transport', 'ANSIBLE_TRANSPORT', 'smart')
OXO1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, 'ssh_connection', 'scp_if_ssh', 'ANSIBLE_SCP_IF_SSH', False, boolean=True)
OXP1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'ansible_managed', None, 'Ansible managed: {file} modified on %Y-%m-%d %H:%M:%S by {uid} on {host}')
OXQ1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'syslog_facility', 'ANSIBLE_SYSLOG_FACILITY', 'LOG_USER')
OXR1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'keep_remote_files', 'ANSIBLE_KEEP_REMOTE_FILES', False, boolean=True)
OXS1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'sudo', 'ANSIBLE_SUDO', False, boolean=True)
OXT1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'sudo_exe', 'ANSIBLE_SUDO_EXE', 'sudo')
OXU1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'sudo_flags', 'ANSIBLE_SUDO_FLAGS', '-H')
OXV1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'hash_behaviour', 'ANSIBLE_HASH_BEHAVIOUR', 'replace')
OXW1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'jinja2_extensions', 'ANSIBLE_JINJA2_EXTENSIONS', None)
OXX1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'executable', 'ANSIBLE_EXECUTABLE', '/bin/sh')
OXY1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'su_exe', 'ANSIBLE_SU_EXE', 'su')
OXZ1A2A3A4 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'su', 'ANSIBLE_SU', False, boolean=True)
OX1A3A4B5A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'su_flags', 'ANSIBLE_SU_FLAGS', '')
OX2A3A4B5A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'su_user', 'ANSIBLE_SU_USER', 'root')
OX3A4B5A6A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'ask_su_pass', 'ANSIBLE_ASK_SU_PASS', False, boolean=True)
OX4A5B6C7A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'gathering', 'ANSIBLE_GATHERING', 'implicit').lower()

OX5A6B7C8A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'action_plugins', 'ANSIBLE_ACTION_PLUGINS', '/usr/share/ansible_plugins/action_plugins')
OX6A7B8C9A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'callback_plugins', 'ANSIBLE_CALLBACK_PLUGINS', '/usr/share/ansible_plugins/callback_plugins')
OX7A8B9C0A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'connection_plugins', 'ANSIBLE_CONNECTION_PLUGINS', '/usr/share/ansible_plugins/connection_plugins')
OX8A9B0C1A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'lookup_plugins', 'ANSIBLE_LOOKUP_PLUGINS', '/usr/share/ansible_plugins/lookup_plugins')
OX9A0B1C2A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'vars_plugins', 'ANSIBLE_VARS_PLUGINS', '/usr/share/ansible_plugins/vars_plugins')
OX0B1C2A3A = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'filter_plugins', 'ANSIBLE_FILTER_PLUGINS', '/usr/share/ansible_plugins/filter_plugins')
OXA0B1C2A3 = OX5F6G7H8I(OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'log_path', 'ANSIBLE_LOG_PATH', ''))

OXB0C1D2E3 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'force_color', 'ANSIBLE_FORCE_COLOR', None, boolean=True)
OXC0D1E2F3 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'nocolor', 'ANSIBLE_NOCOLOR', None, boolean=True)
OXD0E1F2G3 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'nocows', 'ANSIBLE_NOCOWS', None, boolean=True)
OXE0F1G2H3 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'display_skipped_hosts', 'DISPLAY_SKIPPED_HOSTS', True, boolean=True)
OXF0G1H2I3 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'error_on_undefined_vars', 'ANSIBLE_ERROR_ON_UNDEFINED_VARS', True, boolean=True)
OXG0H1I2J3 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'host_key_checking', 'ANSIBLE_HOST_KEY_CHECKING', True, boolean=True)
OXH0I1J2K3 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'system_warnings', 'ANSIBLE_SYSTEM_WARNINGS', True, boolean=True)
OXI0J1K2L3 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'deprecation_warnings', 'ANSIBLE_DEPRECATION_WARNINGS', True, boolean=True)
OXJ0K1L2M3 = OX2C3D4E5F(OX1E2D3C4B, OX5A6A7A8A, 'callable_whitelist', 'ANSIBLE_CALLABLE_WHITELIST', [], islist=True)

OXK0L1M2N3 = OX2C3D4E5F(OX1E2D3C4B, 'ssh_connection', 'ssh_args', 'ANSIBLE_SSH_ARGS', None)
OXL0M1N2O3 = OX2C3D4E5F(OX1E2D3C4B, 'ssh_connection', 'control_path', 'ANSIBLE_SSH_CONTROL_PATH', "%(directory)s/ansible-ssh-%%h-%%p-%%r")
OXM0N1O2P3 = OX2C3D4E5F(OX1E2D3C4B, 'ssh_connection', 'pipelining', 'ANSIBLE_SSH_PIPELINING', False, boolean=True)
OXN0O1P2Q3 = OX2C3D4E5F(OX1E2D3C4B, 'paramiko_connection', 'record_host_keys', 'ANSIBLE_PARAMIKO_RECORD_HOST_KEYS', True, boolean=True)
OXO0P1Q2R3 = OX2C3D4E5F(OX1E2D3C4B, 'fireball_connection', 'zeromq_port', 'ANSIBLE_ZEROMQ_PORT', 5099, integer=True)
OXP0Q1R2S3 = OX2C3D4E5F(OX1E2D3C4B, 'accelerate', 'accelerate_port', 'ACCELERATE_PORT', 5099, integer=True)
OXQ0R1S2T3 = OX2C3D4E5F(OX1E2D3C4B, 'accelerate', 'accelerate_timeout', 'ACCELERATE_TIMEOUT', 30, integer=True)
OXR0S1T2U3 = OX2C3D4E5F(OX1E2D3C4B, 'accelerate', 'accelerate_connect_timeout', 'ACCELERATE_CONNECT_TIMEOUT', 1.0, floating=True)
OXS0T1U2V3 = OX2C3D4E5F(OX1E2D3C4B, 'accelerate', 'accelerate_daemon_timeout', 'ACCELERATE_DAEMON_TIMEOUT', 30, integer=True)
OXT0U1V2W3 = OX2C3D4E5F(OX1E2D3C4B, 'accelerate', 'accelerate_keys_dir', 'ACCELERATE_KEYS_DIR', '~/.fireball.keys')
OXU0V1W2X3 = OX2C3D4E5F(OX1E2D3C4B, 'accelerate', 'accelerate_keys_dir_perms', 'ACCELERATE_KEYS_DIR_PERMS', '700')
OXV0W1X2Y3 = OX2C3D4E5F(OX1E2D3C4B, 'accelerate', 'accelerate_keys_file_perms', 'ACCELERATE_KEYS_FILE_PERMS', '600')
OXW0X1Y2Z3 = OX2C3D4E5F(OX1E2D3C4B, 'accelerate', 'accelerate_multi_key', 'ACCELERATE_MULTI_KEY', False, boolean=True)
OXX0Y1Z2A3 = OX2C3D4E5F(OX1E2D3C4B, 'paramiko_connection', 'pty', 'ANSIBLE_PARAMIKO_PTY', True, boolean=True)

OXY0Z1A2B3 = ascii_letters + digits + ".,:-_"

OXZ0A1B2C3 = None
OX1B2C3D4E = None
OX2B3D4E5F = None
OX3B4E5F6G = None
OX4B5F6G7H = 1.0
OX5B6G7H8I = 1.0