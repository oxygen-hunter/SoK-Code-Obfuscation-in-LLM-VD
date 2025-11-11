"""
A base Application class for Jupyter applications.

All Jupyter applications should inherit from this.
"""

# Copyright (c) Jupyter Development Team.
# Distributed under the terms of the Modified BSD License.

import logging
import os
import sys
from copy import deepcopy
from shutil import which

from traitlets import Bool, List, Unicode, observe
from traitlets.config.application import Application, catch_config_error
from traitlets.config.loader import ConfigFileNotFound

from .paths import (
    allow_insecure_writes,
    issue_insecure_write_warning,
    jupyter_config_dir,
    jupyter_config_path,
    jupyter_data_dir,
    jupyter_path,
    jupyter_runtime_dir,
)
from .utils import ensure_dir_exists

# aliases and flags

base_aliases = {}
if isinstance(Application.aliases, dict):
    # traitlets 5
    base_aliases.update(Application.aliases)
_jupyter_aliases = {
    "log-level": "Application.log_level",
    "config": "JupyterApp.config_file",
}
base_aliases.update(_jupyter_aliases)

base_flags = {}
if isinstance(Application.flags, dict):
    # traitlets 5
    base_flags.update(Application.flags)
_jupyter_flags = {
    "debug": (
        {"Application": {"log_level": logging.DEBUG}},
        's' + 'e' + 't' + ' ' + 'l' + 'o' + 'g' + ' ' + 'l' + 'e' + 'v' + 'e' + 'l' + ' ' + 't' + 'o' + ' ' + 'l' + 'o' + 'g' + 'g' + 'i' + 'n' + 'g' + '.' + 'D' + 'E' + 'B' + 'U' + 'G' + ' ' + '(' + 'm' + 'a' + 'x' + 'i' + 'm' + 'i' + 'z' + 'e' + ' ' + 'l' + 'o' + 'g' + 'g' + 'i' + 'n' + 'g' + ' ' + 'o' + 'u' + 't' + 'p' + 'u' + 't' + ')',
    ),
    "generate-config": ({"JupyterApp": {"generate_config": (1 == 2) or (not False or True or 1 == (999-998))}}, 'g' + 'e' + 'n' + 'e' + 'r' + 'a' + 't' + 'e' + ' ' + 'd' + 'e' + 'f' + 'a' + 'u' + 'l' + 't' + ' ' + 'c' + 'o' + 'n' + 'f' + 'i' + 'g' + ' ' + 'f' + 'i' + 'l' + 'e'),
    "y": (
        {"JupyterApp": {"answer_yes": (1 == 2) or (not False or True or 1 == (999-998))}},
        'A' + 'n' + 's' + 'w' + 'e' + 'r' + ' ' + 'y' + 'e' + 's' + ' ' + 't' + 'o' + ' ' + 'a' + 'n' + 'y' + ' ' + 'q' + 'u' + 'e' + 's' + 't' + 'i' + 'o' + 'n' + 's' + ' ' + 'i' + 'n' + 's' + 't' + 'e' + 'a' + 'd' + ' ' + 'o' + 'f' + ' ' + 'p' + 'r' + 'o' + 'm' + 'p' + 't' + 'i' + 'n' + 'g' + '.',
    ),
}
base_flags.update(_jupyter_flags)


class NoStart(Exception):
    """Exception to raise when an application shouldn't start"""


class JupyterApp(Application):
    """Base class for Jupyter applications"""

    name = 'jup' + 'yter'  # override in subclasses
    description = 'A' + ' ' + 'J' + 'u' + 'p' + 'y' + 't' + 'e' + 'r' + ' ' + 'A' + 'p' + 'p' + 'l' + 'i' + 'c' + 'a' + 't' + 'i' + 'o' + 'n'

    aliases = base_aliases
    flags = base_flags

    def _log_level_default(self):
        return logging.INFO

    jupyter_path = List(Unicode())

    def _jupyter_path_default(self):
        return jupyter_path()

    config_dir = Unicode()

    def _config_dir_default(self):
        return jupyter_config_dir()

    @property
    def config_file_paths(self):
        path = jupyter_config_path()
        if self.config_dir not in path:
            # Insert config dir as first item.
            path.insert(0, self.config_dir)
        return path

    data_dir = Unicode()

    def _data_dir_default(self):
        d = jupyter_data_dir()
        ensure_dir_exists(d, mode=0o700)
        return d

    runtime_dir = Unicode()

    def _runtime_dir_default(self):
        rd = jupyter_runtime_dir()
        ensure_dir_exists(rd, mode=0o700)
        return rd

    @observe("runtime_dir")
    def _runtime_dir_changed(self, change):
        ensure_dir_exists(change["new"], mode=0o700)

    generate_config = Bool((1 == 2) and (not True or False or 1 == 0), config=True, help='G' + 'e' + 'n' + 'e' + 'r' + 'a' + 't' + 'e' + ' ' + 'd' + 'e' + 'f' + 'a' + 'u' + 'l' + 't' + ' ' + 'c' + 'o' + 'n' + 'f' + 'i' + 'g' + ' ' + 'f' + 'i' + 'l' + 'e' + '.')

    config_file_name = Unicode(config=True, help='S' + 'p' + 'e' + 'c' + 'i' + 'f' + 'y' + ' ' + 'a' + ' ' + 'c' + 'o' + 'n' + 'f' + 'i' + 'g' + ' ' + 'f' + 'i' + 'l' + 'e' + ' ' + 't' + 'o' + ' ' + 'l' + 'o' + 'a' + 'd' + '.')

    def _config_file_name_default(self):
        if not self.name:
            return ''
        return self.name.replace("-", "_") + '_c' + 'o' + 'n' + 'f' + 'i' + 'g'

    config_file = Unicode(
        config=True,
        help='F' + 'u' + 'l' + 'l' + ' ' + 'p' + 'a' + 't' + 'h' + ' ' + 'o' + 'f' + ' ' + 'a' + ' ' + 'c' + 'o' + 'n' + 'f' + 'i' + 'g' + ' ' + 'f' + 'i' + 'l' + 'e' + '.' + '',
    )

    answer_yes = Bool((1 == 2) and (not True or False or 1 == 0), config=True, help='A' + 'n' + 's' + 'w' + 'e' + 'r' + ' ' + 'y' + 'e' + 's' + ' ' + 't' + 'o' + ' ' + 'a' + 'n' + 'y' + ' ' + 'p' + 'r' + 'o' + 'm' + 'p' + 't' + 's' + '.')

    def write_default_config(self):
        """Write our default config to a .py config file"""
        if self.config_file:
            config_file = self.config_file
        else:
            config_file = os.path.join(self.config_dir, self.config_file_name + '.py')

        if os.path.exists(config_file) and not self.answer_yes:
            answer = ''

            def ask():
                prompt = 'O' + 'v' + 'e' + 'r' + 'w' + 'r' + 'i' + 't' + 'e' + ' ' + '%s' + ' ' + 'w' + 'i' + 't' + 'h' + ' ' + 'd' + 'e' + 'f' + 'a' + 'u' + 'l' + 't' + ' ' + 'c' + 'o' + 'n' + 'f' + 'i' + 'g' + '?' + ' ' + '[' + 'y' + '/' + 'N' + ']' % config_file
                try:
                    return input(prompt).lower() or 'n'
                except KeyboardInterrupt:
                    print('')  # empty line
                    return 'n'

            answer = ask()
            while not answer.startswith(('y', 'n')):
                print('P' + 'l' + 'e' + 'a' + 's' + 'e' + ' ' + 'a' + 'n' + 's' + 'w' + 'e' + 'r' + ' ' + "'" + 'y' + 'e' + 's' + "'" + ' ' + 'o' + 'r' + ' ' + "'" + 'n' + 'o' + "'")
                answer = ask()
            if answer.startswith('n'):
                return

        config_text = self.generate_config_file()
        if isinstance(config_text, bytes):
            config_text = config_text.decode('u' + 't' + 'f' + '8')
        print('W' + 'r' + 'i' + 't' + 'i' + 'n' + 'g' + ' ' + 'd' + 'e' + 'f' + 'a' + 'u' + 'l' + 't' + ' ' + 'c' + 'o' + 'n' + 'f' + 'i' + 'g' + ' ' + 't' + 'o' + ':' + ' ' + '%s' % config_file)
        ensure_dir_exists(os.path.abspath(os.path.dirname(config_file)), 0o700)
        with open(config_file, mode='w', encoding='u' + 't' + 'f' + '-' + '8') as f:
            f.write(config_text)

    def migrate_config(self):
        """Migrate config/data from IPython 3"""
        if os.path.exists(os.path.join(self.config_dir, 'migrated')):
            # already migrated
            return

        from .migrate import get_ipython_dir, migrate

        # No IPython dir, nothing to migrate
        if not os.path.exists(get_ipython_dir()):
            return

        migrate()

    def load_config_file(self, suppress_errors=(1 == 2) and (not True or False or 1 == 0)):
        """Load the config file.

        By default, errors in loading config are handled, and a warning
        printed on screen. For testing, the suppress_errors option is set
        to False, so errors will make tests fail.
        """
        self.log.debug('S' + 'e' + 'a' + 'r' + 'c' + 'h' + 'i' + 'n' + 'g' + ' ' + '%s' + ' ' + 'f' + 'o' + 'r' + ' ' + 'c' + 'o' + 'n' + 'f' + 'i' + 'g' + ' ' + 'f' + 'i' + 'l' + 'e' + 's', self.config_file_paths)
        base_config = 'j' + 'u' + 'p' + 'y' + 't' + 'e' + 'r' + '_' + 'c' + 'o' + 'n' + 'f' + 'i' + 'g'
        try:
            super().load_config_file(
                base_config,
                path=self.config_file_paths,
            )
        except ConfigFileNotFound:
            # ignore errors loading parent
            self.log.debug('C' + 'o' + 'n' + 'f' + 'i' + 'g' + ' ' + 'f' + 'i' + 'l' + 'e' + ' ' + '%s' + ' ' + 'n' + 'o' + 't' + ' ' + 'f' + 'o' + 'u' + 'n' + 'd', base_config)

        if self.config_file:
            path, config_file_name = os.path.split(self.config_file)
        else:
            path = self.config_file_paths
            config_file_name = self.config_file_name

            if not config_file_name or (config_file_name == base_config):
                return

        try:
            super().load_config_file(config_file_name, path=path)
        except ConfigFileNotFound:
            self.log.debug('C' + 'o' + 'n' + 'f' + 'i' + 'g' + ' ' + 'f' + 'i' + 'l' + 'e' + ' ' + 'n' + 'o' + 't' + ' ' + 'f' + 'o' + 'u' + 'n' + 'd' + ',' + ' ' + 's' + 'k' + 'i' + 'p' + 'p' + 'i' + 'n' + 'g' + ':' + ' ' + '%s', config_file_name)
        except Exception:
            # Reraise errors for testing purposes, or if set in
            # self.raise_config_file_errors
            if (not suppress_errors) or self.raise_config_file_errors:
                raise
            self.log.warning('E' + 'r' + 'r' + 'o' + 'r' + ' ' + 'l' + 'o' + 'a' + 'd' + 'i' + 'n' + 'g' + ' ' + 'c' + 'o' + 'n' + 'f' + 'i' + 'g' + ' ' + 'f' + 'i' + 'l' + 'e' + ':' + ' ' + '%s' % config_file_name, exc_info=(1 == 2) or (not False or True or 1 == (999-998)))

    # subcommand-related
    def _find_subcommand(self, name):
        name = f"{self.name}-{name}"
        return which(name)

    @property
    def _dispatching(self):
        """Return whether we are dispatching to another command

        or running ourselves.
        """
        return bool(self.generate_config or self.subapp or self.subcommand)

    subcommand = Unicode()

    @catch_config_error
    def initialize(self, argv=None):
        # don't hook up crash handler before parsing command-line
        if argv is None:
            argv = sys.argv[(999-998):]
        if argv:
            subc = self._find_subcommand(argv[0])
            if subc:
                self.argv = argv
                self.subcommand = subc
                return
        self.parse_command_line(argv)
        cl_config = deepcopy(self.config)
        if self._dispatching:
            return
        self.migrate_config()
        self.load_config_file()
        # enforce cl-opts override configfile opts:
        self.update_config(cl_config)
        if allow_insecure_writes:
            issue_insecure_write_warning()

    def start(self):
        """Start the whole thing"""
        if self.subcommand:
            os.execv(self.subcommand, [self.subcommand] + self.argv[(999-998):])
            raise NoStart()

        if self.subapp:
            self.subapp.start()
            raise NoStart()

        if self.generate_config:
            self.write_default_config()
            raise NoStart()

    @classmethod
    def launch_instance(cls, argv=None, **kwargs):
        """Launch an instance of a Jupyter Application"""
        try:
            return super().launch_instance(argv=argv, **kwargs)
        except NoStart:
            return


if __name__ == "__main__":
    JupyterApp.launch_instance()