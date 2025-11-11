import gettext
import os
import sys
from oslo.config import cfg
from keystone.common import logging

gettext.install('keystone', unicode=1)

_DEFAULT_LOG_FORMAT = "%(asctime)s %(levelname)8s [%(name)s] %(message)s"
_DEFAULT_LOG_DATE_FORMAT = "%Y-%m-%d %H:%M:%S"
_DEFAULT_AUTH_METHODS = ['password', 'token']

COMMON_CLI_OPTS = [
    cfg.BoolOpt('debug',
                short='d',
                default=False,
                help='Print debugging output (set logging level to '
                     'DEBUG instead of default WARNING level).'),
    cfg.BoolOpt('verbose',
                short='v',
                default=False,
                help='Print more verbose output (set logging level to '
                     'INFO instead of default WARNING level).'),
]

LOGGING_CLI_OPTS = [
    cfg.StrOpt('log-config',
               metavar='PATH',
               help='If this option is specified, the logging configuration '
                    'file specified is used and overrides any other logging '
                    'options specified. Please see the Python logging module '
                    'documentation for details on logging configuration '
                    'files.'),
    cfg.StrOpt('log-format',
               default=_DEFAULT_LOG_FORMAT,
               metavar='FORMAT',
               help='A logging.Formatter log message format string which may '
                    'use any of the available logging.LogRecord attributes.'),
    cfg.StrOpt('log-date-format',
               default=_DEFAULT_LOG_DATE_FORMAT,
               metavar='DATE_FORMAT',
               help='Format string for %%(asctime)s in log records.'),
    cfg.StrOpt('log-file',
               metavar='PATH',
               help='Name of log file to output. '
                    'If not set, logging will go to stdout.'),
    cfg.StrOpt('log-dir',
               help='The directory in which to store log files. '
                    '(will be prepended to --log-file)'),
    cfg.BoolOpt('use-syslog',
                default=False,
                help='Use syslog for logging.'),
    cfg.StrOpt('syslog-log-facility',
               default='LOG_USER',
               help='syslog facility to receive log lines.')
]

CONF = cfg.CONF

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr, *args = self.instructions[self.pc]
            getattr(self, f'op_{instr}')(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        b = self.op_POP()
        a = self.op_POP()
        self.op_PUSH(a - b)

    def op_JMP(self, target):
        self.pc = target - 1

    def op_JZ(self, target):
        if self.op_POP() == 0:
            self.pc = target - 1

    def op_LOAD(self, var_name):
        self.op_PUSH(globals()[var_name])

    def op_STORE(self, var_name):
        globals()[var_name] = self.op_POP()

def setup_logging_vm():
    vm = VM()
    vm.load_instructions([
        ('LOAD', 'CONF'),
        ('LOAD', 'CONF.log_config'),
        ('PUSH', None),
        ('SUB',),
        ('JZ', 8),
        ('LOAD', 'os'),
        ('LOAD', 'os.path'),
        ('LOAD', 'os.path.exists'),
        ('LOAD', 'CONF.log_config'),
        ('CALL', 1),
        ('JZ', 16),
        ('LOAD', 'logging'),
        ('LOAD', 'logging.config'),
        ('LOAD', 'logging.config.fileConfig'),
        ('LOAD', 'CONF.log_config'),
        ('CALL', 1),
        ('RETURN',),
        ('PUSH', RuntimeError),
        ('LOAD', '_'),
        ('LOAD', 'CONF.log_config'),
        ('FORMAT', 'Unable to locate specified logging config file: %s'),
        ('CALL', 1),
        ('RAISE',),
        ('LOAD', 'logging'),
        ('LOAD', 'logging.root'),
        ('STORE', 'root_logger'),
        ('LOAD', 'CONF'),
        ('LOAD', 'CONF.debug'),
        ('JZ', 30),
        ('LOAD', 'root_logger'),
        ('LOAD', 'logging.DEBUG'),
        ('CALL', 1),
        ('JMP', 36),
        ('LOAD', 'CONF.verbose'),
        ('JZ', 35),
        ('LOAD', 'root_logger'),
        ('LOAD', 'logging.INFO'),
        ('CALL', 1),
        ('JMP', 36),
        ('LOAD', 'root_logger'),
        ('LOAD', 'logging.WARNING'),
        ('CALL', 1),
        ('LOAD', 'logging'),
        ('LOAD', 'logging.Formatter'),
        ('LOAD', 'CONF.log_format'),
        ('LOAD', 'CONF.log_date_format'),
        ('CALL', 2),
        ('STORE', 'formatter'),
        ('LOAD', 'CONF'),
        ('LOAD', 'CONF.use_syslog'),
        ('JZ', 57),
        ('TRY',),
        ('LOAD', 'logging'),
        ('LOAD', 'logging.SysLogHandler'),
        ('LOAD', 'CONF.syslog_log_facility'),
        ('GETATTR',),
        ('STORE', 'facility'),
        ('EXCEPT', AttributeError),
        ('PUSH', ValueError),
        ('LOAD', '_'),
        ('FORMAT', 'Invalid syslog facility'),
        ('CALL', 1),
        ('RAISE',),
        ('LOAD', 'logging'),
        ('LOAD', 'logging.SysLogHandler'),
        ('PUSH', '/dev/log'),
        ('LOAD', 'facility'),
        ('CALL', 2),
        ('STORE', 'handler'),
        ('JMP', 72),
        ('LOAD', 'CONF.log_file'),
        ('JZ', 71),
        ('LOAD', 'CONF.log_file'),
        ('STORE', 'logfile'),
        ('LOAD', 'CONF.log_dir'),
        ('JZ', 70),
        ('LOAD', 'os'),
        ('LOAD', 'os.path'),
        ('LOAD', 'os.path.join'),
        ('LOAD', 'CONF.log_dir'),
        ('LOAD', 'logfile'),
        ('CALL', 2),
        ('STORE', 'logfile'),
        ('LOAD', 'logging'),
        ('LOAD', 'logging.WatchedFileHandler'),
        ('LOAD', 'logfile'),
        ('CALL', 1),
        ('STORE', 'handler'),
        ('JMP', 72),
        ('LOAD', 'logging'),
        ('LOAD', 'logging.StreamHandler'),
        ('LOAD', 'sys.stdout'),
        ('CALL', 1),
        ('STORE', 'handler'),
        ('LOAD', 'handler'),
        ('LOAD', 'formatter'),
        ('CALL', 1),
        ('LOAD', 'root_logger'),
        ('LOAD', 'handler'),
        ('CALL', 1),
    ])
    vm.run()

def setup_authentication_vm():
    vm = VM()
    vm.load_instructions([
        ('LOAD', 'CONF.auth.methods'),
        ('STORE', 'methods'),
        ('PUSH', 0),
        ('STORE', 'i'),
        ('LABEL', 1),
        ('LOAD', 'i'),
        ('LOAD', 'methods'),
        ('CALL', 0),
        ('JGE', 'end'),
        ('LOAD', 'methods'),
        ('LOAD', 'i'),
        ('GETITEM',),
        ('STORE', 'method_name'),
        ('LOAD', 'method_name'),
        ('LOAD', '_DEFAULT_AUTH_METHODS'),
        ('CALL', 0),
        ('JNZ', 2),
        ('LOAD', 'register_str'),
        ('LOAD', 'method_name'),
        ('PUSH', 'auth'),
        ('CALL', 2),
        ('LABEL', 2),
        ('LOAD', 'i'),
        ('PUSH', 1),
        ('ADD',),
        ('STORE', 'i'),
        ('JMP', 1),
        ('LABEL', 'end'),
    ])
    vm.run()

def configure_vm():
    vm = VM()
    vm.load_instructions([
        ('LOAD', 'CONF'),
        ('LOAD', 'CONF.register_cli_opts'),
        ('LOAD', 'COMMON_CLI_OPTS'),
        ('CALL', 1),
        ('LOAD', 'CONF.register_cli_opts'),
        ('LOAD', 'LOGGING_CLI_OPTS'),
        ('CALL', 1),
        ('LOAD', 'register_cli_bool'),
        ('PUSH', 'standard-threads'),
        ('PUSH', False),
        ('CALL', 2),
        ('LOAD', 'register_cli_str'),
        ('PUSH', 'pydev-debug-host'),
        ('PUSH', None),
        ('CALL', 2),
        ('LOAD', 'register_cli_int'),
        ('PUSH', 'pydev-debug-port'),
        ('PUSH', None),
        ('CALL', 2),
        ('LOAD', 'register_str'),
        ('PUSH', 'admin_token'),
        ('PUSH', True),
        ('PUSH', 'ADMIN'),
        ('CALL', 3),
        # Further configuration instructions would be added here
    ])
    vm.run()

configure_vm()
setup_logging_vm()
setup_authentication_vm()