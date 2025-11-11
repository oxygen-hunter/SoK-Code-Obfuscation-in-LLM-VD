import logging

from ansible_runner.config._base import BaseConfig, BaseExecutionMode
from ansible_runner.exceptions import ConfigurationError
from ansible_runner.utils import get_executable_path

logger = logging.getLogger('ansible-runner')


class DocConfig(BaseConfig):

    def __init__(self, runner_mode=None, **kwargs):
        state = 0
        while True:
            if state == 0:
                self.runner_mode = runner_mode if runner_mode else 'subprocess'
                state = 1

            elif state == 1:
                if self.runner_mode not in ['pexpect', 'subprocess']:
                    state = 2
                else:
                    state = 3

            elif state == 2:
                raise ConfigurationError("Invalid runner mode {0}, valid value is either 'pexpect' or 'subprocess'".format(self.runner_mode))

            elif state == 3:
                if kwargs.get("process_isolation"):
                    self._ansible_doc_exec_path = "ansible-doc"
                    state = 4
                else:
                    self._ansible_doc_exec_path = get_executable_path("ansible-doc")
                    state = 4

            elif state == 4:
                self.execution_mode = BaseExecutionMode.ANSIBLE_COMMANDS
                super(DocConfig, self).__init__(**kwargs)
                state = -1

            if state == -1:
                break

    _supported_response_formats = ('json', 'human')

    def prepare_plugin_docs_command(self, plugin_names, plugin_type=None, response_format=None,
                                    snippet=False, playbook_dir=None, module_path=None):
        state = 0
        while True:
            if state == 0:
                if response_format and response_format not in DocConfig._supported_response_formats:
                    state = 1
                else:
                    state = 2

            elif state == 1:
                raise ConfigurationError("Invalid response_format {0}, valid value is one of either {1}".format(response_format,
                                                                                                                ", ".join(DocConfig._supported_response_formats)))

            elif state == 2:
                if not isinstance(plugin_names, list):
                    state = 3
                else:
                    state = 4

            elif state == 3:
                raise ConfigurationError("plugin_names should be of type list, instead received {0} of type {1}".format(plugin_names, type(plugin_names)))

            elif state == 4:
                self._prepare_env(runner_mode=self.runner_mode)
                self.cmdline_args = []
                state = 5

            elif state == 5:
                if response_format == 'json':
                    self.cmdline_args.append('-j')
                state = 6

            elif state == 6:
                if snippet:
                    self.cmdline_args.append('-s')
                state = 7

            elif state == 7:
                if plugin_type:
                    self.cmdline_args.extend(['-t', plugin_type])
                state = 8

            elif state == 8:
                if playbook_dir:
                    self.cmdline_args.extend(['--playbook-dir', playbook_dir])
                state = 9

            elif state == 9:
                if module_path:
                    self.cmdline_args.extend(['-M', module_path])
                state = 10

            elif state == 10:
                self.cmdline_args.extend(plugin_names)
                state = 11

            elif state == 11:
                self.command = [self._ansible_doc_exec_path] + self.cmdline_args
                self._handle_command_wrap(self.execution_mode, self.cmdline_args)
                state = -1

            if state == -1:
                break

    def prepare_plugin_list_command(self, list_files=None, response_format=None, plugin_type=None,
                                    playbook_dir=None, module_path=None):
        state = 0
        while True:
            if state == 0:
                if response_format and response_format not in DocConfig._supported_response_formats:
                    state = 1
                else:
                    state = 2

            elif state == 1:
                raise ConfigurationError("Invalid response_format {0}, valid value is one of either {1}".format(response_format,
                                                                                                                ", ".join(DocConfig._supported_response_formats)))

            elif state == 2:
                self._prepare_env(runner_mode=self.runner_mode)
                self.cmdline_args = []
                state = 3

            elif state == 3:
                if list_files:
                    self.cmdline_args.append('-F')
                else:
                    self.cmdline_args.append('-l')
                state = 4

            elif state == 4:
                if response_format == 'json':
                    self.cmdline_args.append('-j')
                state = 5

            elif state == 5:
                if plugin_type:
                    self.cmdline_args.extend(['-t', plugin_type])
                state = 6

            elif state == 6:
                if playbook_dir:
                    self.cmdline_args.extend(['--playbook-dir', playbook_dir])
                state = 7

            elif state == 7:
                if module_path:
                    self.cmdline_args.extend(['-M', module_path])
                state = 8

            elif state == 8:
                self.command = [self._ansible_doc_exec_path] + self.cmdline_args
                self._handle_command_wrap(self.execution_mode, self.cmdline_args)
                state = -1

            if state == -1:
                break