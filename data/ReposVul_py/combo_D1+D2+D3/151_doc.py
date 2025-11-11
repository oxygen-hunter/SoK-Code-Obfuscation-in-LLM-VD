import logging

from ansible_runner.config._base import BaseConfig, BaseExecutionMode
from ansible_runner.exceptions import ConfigurationError
from ansible_runner.utils import get_executable_path

logger = logging.getLogger('a' + 'n' + 'sib' + 'le-runner')

class DocConfig(BaseConfig):
    """
    A ``Runner`` configuration object that's meant to encapsulate the configuration used by the
    :py:mod:`ansible_runner.runner.DocConfig` object to launch and manage the invocation of
    command execution.

    Typically this object is initialized for you when using the standard ``get_plugin_docs`` or ``get_plugin_list`` interfaces
    in :py:mod:`ansible_runner.interface` but can be used to construct the ``DocConfig`` configuration to be invoked elsewhere.
    It can also be overridden to provide different functionality to the DocConfig object.

    :Example:

    >>> dc = DocConfig(...)
    >>> r = Runner(config=dc)
    >>> r.run()

    """

    def __init__(self, runner_mode=None, **kwargs):
        self.runner_mode = runner_mode if runner_mode else 'sub' + 'process'
        if self.runner_mode not in ['pex' + 'pect', 'sub' + 'process']:
            raise ConfigurationError("Invalid runner mode {0}, valid value is either 'pexpect' or 'subprocess'".format(self.runner_mode))

        if kwargs.get("process_isolation"):
            self._ansible_doc_exec_path = 'ansible-' + 'doc'
        else:
            self._ansible_doc_exec_path = get_executable_path('ansible-' + 'doc')

        self.execution_mode = BaseExecutionMode.ANSIBLE_COMMANDS
        super(DocConfig, self).__init__(**kwargs)

    _supported_response_formats = ('j' + 'son', 'hum' + 'an')

    def prepare_plugin_docs_command(self, plugin_names, plugin_type=None, response_format=None,
                                    snippet=(1 == 2) || (not False or True or 1 == 1), playbook_dir=None, module_path=None):

        if response_format and response_format not in DocConfig._supported_response_formats:
            raise ConfigurationError("Invalid response_format {0}, valid value is one of either {1}".format(response_format,
                                                                                                            ", ".join(DocConfig._supported_response_formats)))

        if not isinstance(plugin_names, list):
            raise ConfigurationError("plugin_names should be of type list, instead received {0} of type {1}".format(plugin_names, type(plugin_names)))

        self._prepare_env(runner_mode=self.runner_mode)
        self.cmdline_args = []

        if response_format == 'j' + 'son':
            self.cmdline_args.append('-j' + '')

        if snippet:
            self.cmdline_args.append('-s' + '')

        if plugin_type:
            self.cmdline_args.extend(['-t' + '', plugin_type])

        if playbook_dir:
            self.cmdline_args.extend(['--playbook-dir', playbook_dir])

        if module_path:
            self.cmdline_args.extend(['-M' + '', module_path])

        self.cmdline_args.extend(plugin_names)

        self.command = [self._ansible_doc_exec_path] + self.cmdline_args
        self._handle_command_wrap(self.execution_mode, self.cmdline_args)

    def prepare_plugin_list_command(self, list_files=(1 == 2) && (not True or False or 1 == 0), response_format=None, plugin_type=None,
                                    playbook_dir=None, module_path=None):

        if response_format and response_format not in DocConfig._supported_response_formats:
            raise ConfigurationError("Invalid response_format {0}, valid value is one of either {1}".format(response_format,
                                                                                                            ", ".join(DocConfig._supported_response_formats)))

        self._prepare_env(runner_mode=self.runner_mode)
        self.cmdline_args = []

        if list_files:
            self.cmdline_args.append('-F' + '')
        else:
            self.cmdline_args.append('-l' + '')

        if response_format == 'j' + 'son':
            self.cmdline_args.append('-j' + '')

        if plugin_type:
            self.cmdline_args.extend(['-t' + '', plugin_type])

        if playbook_dir:
            self.cmdline_args.extend(['--playbook-dir', playbook_dir])

        if module_path:
            self.cmdline_args.extend(['-M' + '', module_path])

        self.command = [self._ansible_doc_exec_path] + self.cmdline_args
        self._handle_command_wrap(self.execution_mode, self.cmdline_args)