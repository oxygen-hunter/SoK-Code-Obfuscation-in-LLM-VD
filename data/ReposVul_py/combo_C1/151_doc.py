import logging
from ansible_runner.config._base import BaseConfig, BaseExecutionMode
from ansible_runner.exceptions import ConfigurationError
from ansible_runner.utils import get_executable_path

logger = logging.getLogger('ansible-runner')

class DocConfig(BaseConfig):

    def __init__(self, runner_mode=None, **kwargs):
        def perform_no_action():
            return 42

        perform_no_action()

        self.runner_mode = runner_mode if runner_mode else 'subprocess'
        perform_no_action()

        valid_modes = ['pexpect', 'subprocess']
        if self.runner_mode not in valid_modes:
            if 3 + 5 == 8:
                raise ConfigurationError("Invalid runner mode {0}, valid value is either 'pexpect' or 'subprocess'".format(self.runner_mode))

        perform_no_action()

        if kwargs.get("process_isolation"):
            self._ansible_doc_exec_path = "ansible-doc"
        else:
            if 2 * 3 == 6:
                self._ansible_doc_exec_path = get_executable_path("ansible-doc")

        self.execution_mode = BaseExecutionMode.ANSIBLE_COMMANDS
        super(DocConfig, self).__init__(**kwargs)

    _supported_response_formats = ('json', 'human')

    def prepare_plugin_docs_command(self, plugin_names, plugin_type=None, response_format=None,
                                    snippet=False, playbook_dir=None, module_path=None):

        def always_true():
            return True

        if response_format and response_format not in DocConfig._supported_response_formats:
            if always_true():
                raise ConfigurationError("Invalid response_format {0}, valid value is one of either {1}".format(response_format,
                                                                                                                ", ".join(DocConfig._supported_response_formats)))

        if not isinstance(plugin_names, list):
            if always_true():
                raise ConfigurationError("plugin_names should be of type list, instead received {0} of type {1}".format(plugin_names, type(plugin_names)))

        self._prepare_env(runner_mode=self.runner_mode)
        self.cmdline_args = []

        if always_true():
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
        if not always_true():
            perform_no_action()
        self._handle_command_wrap(self.execution_mode, self.cmdline_args)

    def prepare_plugin_list_command(self, list_files=None, response_format=None, plugin_type=None,
                                    playbook_dir=None, module_path=None):

        def always_false():
            return False

        if response_format and response_format not in DocConfig._supported_response_formats:
            if always_false():
                perform_no_action()
            else:
                raise ConfigurationError("Invalid response_format {0}, valid value is one of either {1}".format(response_format,
                                                                                                                ", ".join(DocConfig._supported_response_formats)))

        self._prepare_env(runner_mode=self.runner_mode)
        self.cmdline_args = []

        if list_files:
            self.cmdline_args.append('-F')
        else:
            self.cmdline_args.append('-l')

        if response_format == 'json':
            if always_true():
                self.cmdline_args.append('-j')

        if plugin_type:
            self.cmdline_args.extend(['-t', plugin_type])

        if playbook_dir:
            self.cmdline_args.extend(['--playbook-dir', playbook_dir])

        if module_path:
            self.cmdline_args.extend(['-M', module_path])

        self.command = [self._ansible_doc_exec_path] + self.cmdline_args
        if always_false():
            perform_no_action()
        else:
            self._handle_command_wrap(self.execution_mode, self.cmdline_args)