import logging
from ansible_runner.config._base import BaseConfig, BaseExecutionMode
from ansible_runner.exceptions import ConfigurationError
from ansible_runner.utils import get_executable_path

logger = logging.getLogger('ansible-runner')

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def execute(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            getattr(self, f"op_{opcode}")(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        a = self.op_POP()
        b = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        a = self.op_POP()
        b = self.op_POP()
        self.op_PUSH(b - a)

    def op_JZ(self, target):
        if self.op_POP() == 0:
            self.pc = target - 1

    def op_STORE(self, idx):
        value = self.op_POP()
        if idx < len(self.stack):
            self.stack[idx] = value
        else:
            self.stack.extend([None] * (idx - len(self.stack) + 1))
            self.stack[idx] = value

    def op_LOAD(self, idx):
        self.op_PUSH(self.stack[idx])

    def op_CALL(self, func, *args):
        func(*args)

class DocConfig(BaseConfig):
    def __init__(self, runner_mode=None, **kwargs):
        self.vm = VirtualMachine()
        self.vm.execute([
            ('PUSH', 'subprocess'),
            ('LOAD', 0),
            ('JZ', 10),
            ('PUSH', 'pexpect'),
            ('LOAD', 0),
            ('JZ', 10),
            ('CALL', self.invalid_runner_mode_exception),
            ('STORE', 0),
            ('JZ', 13),
            ('CALL', self.init_ansible_doc_exec_path, kwargs),
            ('STORE', 1)
        ])
        self.execution_mode = BaseExecutionMode.ANSIBLE_COMMANDS
        super(DocConfig, self).__init__(**kwargs)

    def init_ansible_doc_exec_path(self, kwargs):
        if kwargs.get("process_isolation"):
            self._ansible_doc_exec_path = "ansible-doc"
        else:
            self._ansible_doc_exec_path = get_executable_path("ansible-doc")

    def invalid_runner_mode_exception(self):
        raise ConfigurationError("Invalid runner mode, valid value is either 'pexpect' or 'subprocess'")

    _supported_response_formats = ('json', 'human')

    def prepare_plugin_docs_command(self, plugin_names, plugin_type=None, response_format=None,
                                    snippet=False, playbook_dir=None, module_path=None):
        self.vm.execute([
            ('PUSH', response_format),
            ('LOAD', 0),
            ('JZ', 2),
            ('CALL', self.check_response_format, response_format),
            ('STORE', 0),
            ('CALL', self.check_plugin_names_type, plugin_names),
            ('STORE', 0),
            ('CALL', self._prepare_env, self.runner_mode),
            ('STORE', 0),
            ('CALL', self.append_cmdline_args, response_format, snippet, plugin_type, playbook_dir, module_path, plugin_names),
            ('STORE', 0),
            ('CALL', self.handle_command, self.execution_mode, self.cmdline_args),
        ])

    def check_response_format(self, response_format):
        if response_format and response_format not in DocConfig._supported_response_formats:
            raise ConfigurationError("Invalid response_format, valid value is one of either {0}".format(", ".join(DocConfig._supported_response_formats)))

    def check_plugin_names_type(self, plugin_names):
        if not isinstance(plugin_names, list):
            raise ConfigurationError("plugin_names should be of type list")

    def append_cmdline_args(self, response_format, snippet, plugin_type, playbook_dir, module_path, plugin_names):
        self.cmdline_args = []
        if response_format == 'json':
            self.cmdline_args.append('-j')
        if snippet:
            self.cmdline_args.append('-s')
        if plugin_type:
            self.cmdline_args.extend(['-t', plugin_type])
        if playbook_dir:
            self.cmdline_args.extend(['--playbook-dir', playbook_dir])
        if module_path:
            self.cmdline_args.extend(['-M', module_path])
        self.cmdline_args.extend(plugin_names)
        self.command = [self._ansible_doc_exec_path] + self.cmdline_args

    def handle_command(self, execution_mode, cmdline_args):
        self._handle_command_wrap(execution_mode, cmdline_args)

    def prepare_plugin_list_command(self, list_files=None, response_format=None, plugin_type=None,
                                    playbook_dir=None, module_path=None):
        self.vm.execute([
            ('PUSH', response_format),
            ('LOAD', 0),
            ('JZ', 2),
            ('CALL', self.check_response_format, response_format),
            ('STORE', 0),
            ('CALL', self._prepare_env, self.runner_mode),
            ('STORE', 0),
            ('CALL', self.append_plugin_list_args, list_files, response_format, plugin_type, playbook_dir, module_path),
            ('STORE', 0),
            ('CALL', self.handle_command, self.execution_mode, self.cmdline_args),
        ])

    def append_plugin_list_args(self, list_files, response_format, plugin_type, playbook_dir, module_path):
        self.cmdline_args = ['-F' if list_files else '-l']
        if response_format == 'json':
            self.cmdline_args.append('-j')
        if plugin_type:
            self.cmdline_args.extend(['-t', plugin_type])
        if playbook_dir:
            self.cmdline_args.extend(['--playbook-dir', playbook_dir])
        if module_path:
            self.cmdline_args.extend(['-M', module_path])
        self.command = [self._ansible_doc_exec_path] + self.cmdline_args