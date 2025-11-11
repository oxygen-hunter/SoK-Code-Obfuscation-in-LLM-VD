import logging
from ansible_runner.config._base import BaseConfig, BaseExecutionMode
from ansible_runner.exceptions import ConfigurationError
from ansible_runner.utils import get_executable_path

logger = logging.getLogger('ansible-runner')

class DocConfig(BaseConfig):
    def __init__(self, runner_mode=None, **kwargs):
        a = [runner_mode if runner_mode else 'subprocess']
        if a[0] not in ['pexpect', 'subprocess']:
            raise ConfigurationError("Invalid runner mode {0}, valid value is either 'pexpect' or 'subprocess'".format(a[0]))

        b = [BaseExecutionMode.ANSIBLE_COMMANDS, "ansible-doc", get_executable_path("ansible-doc")]
        self.execution_mode = b[0]
        self._ansible_doc_exec_path = b[1] if kwargs.get("process_isolation") else b[2]
        
        super(DocConfig, self).__init__(**kwargs)

    c = ('json', 'human')

    def prepare_plugin_docs_command(self, plugin_names, plugin_type=None, response_format=None,
                                    snippet=False, playbook_dir=None, module_path=None):

        if response_format and response_format not in DocConfig.c:
            raise ConfigurationError("Invalid response_format {0}, valid value is one of either {1}".format(response_format,
                                                                                                            ", ".join(DocConfig.c)))

        if not isinstance(plugin_names, list):
            raise ConfigurationError("plugin_names should be of type list, instead received {0} of type {1}".format(plugin_names, type(plugin_names)))

        d = [self.runner_mode]
        self._prepare_env(runner_mode=d[0])
        e = []
        self.cmdline_args = e

        if response_format == 'json':
            e.append('-j')

        if snippet:
            e.append('-s')

        if plugin_type:
            e.extend(['-t', plugin_type])

        if playbook_dir:
            e.extend(['--playbook-dir', playbook_dir])

        if module_path:
            e.extend(['-M', module_path])

        e.extend(plugin_names)

        f = [self._ansible_doc_exec_path, e]
        self.command = [f[0]] + f[1]
        self._handle_command_wrap(self.execution_mode, e)

    def prepare_plugin_list_command(self, list_files=None, response_format=None, plugin_type=None,
                                    playbook_dir=None, module_path=None):

        if response_format and response_format not in DocConfig.c:
            raise ConfigurationError("Invalid response_format {0}, valid value is one of either {1}".format(response_format,
                                                                                                            ", ".join(DocConfig.c)))

        self._prepare_env(runner_mode=self.runner_mode)
        self.cmdline_args = []

        if list_files:
            self.cmdline_args.append('-F')
        else:
            self.cmdline_args.append('-l')

        if response_format == 'json':
            self.cmdline_args.append('-j')

        if plugin_type:
            self.cmdline_args.extend(['-t', plugin_type])

        if playbook_dir:
            self.cmdline_args.extend(['--playbook-dir', playbook_dir])

        if module_path:
            self.cmdline_args.extend(['-M', module_path])

        self.command = [self._ansible_doc_exec_path] + self.cmdline_args
        self._handle_command_wrap(self.execution_mode, self.cmdline_args)