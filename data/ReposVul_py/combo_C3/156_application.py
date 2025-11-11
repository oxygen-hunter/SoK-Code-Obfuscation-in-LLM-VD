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

class NoStart(Exception):
    pass

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.registers = {}

    def execute(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            inst, *args = self.instructions[self.pc]
            getattr(self, f"inst_{inst}")(*args)
            self.pc += 1

    def inst_PUSH(self, value):
        self.stack.append(value)

    def inst_POP(self):
        self.stack.pop()

    def inst_LOAD(self, name):
        self.stack.append(self.registers.get(name, None))

    def inst_STORE(self, name):
        self.registers[name] = self.stack.pop()

    def inst_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def inst_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def inst_JMP(self, addr):
        self.pc = addr - 1

    def inst_JZ(self, addr):
        if self.stack.pop() == 0:
            self.pc = addr - 1

    def inst_CALL(self, func):
        func()

class JupyterApp(Application):
    name = "jupyter"
    description = "A Jupyter Application"
    aliases = {}
    flags = {}

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.vm = VM()
        self.vm.registers.update({
            'log_level': logging.INFO,
            'config_dir': '',
            'data_dir': '',
            'runtime_dir': '',
            'config_file_name': '',
            'config_file': '',
            'answer_yes': False,
            'generate_config': False,
            'subcommand': '',
            'argv': [],
        })

    def _log_level_default(self):
        return logging.INFO

    def _jupyter_path_default(self):
        return jupyter_path()

    def _config_dir_default(self):
        return jupyter_config_dir()

    @property
    def config_file_paths(self):
        path = jupyter_config_path()
        if self.vm.registers['config_dir'] not in path:
            path.insert(0, self.vm.registers['config_dir'])
        return path

    def _data_dir_default(self):
        d = jupyter_data_dir()
        ensure_dir_exists(d, mode=0o700)
        return d

    def _runtime_dir_default(self):
        rd = jupyter_runtime_dir()
        ensure_dir_exists(rd, mode=0o700)
        return rd

    @observe("runtime_dir")
    def _runtime_dir_changed(self, change):
        ensure_dir_exists(change["new"], mode=0o700)

    def _config_file_name_default(self):
        if not self.name:
            return ""
        return self.name.replace("-", "_") + "_config"

    def write_default_config(self):
        self.vm.execute([
            ('LOAD', 'config_file'),
            ('JZ', 5),
            ('STORE', 'config_file'),
            ('JMP', 7),
            ('LOAD', 'config_dir'),
            ('LOAD', 'config_file_name'),
            ('ADD',),
            ('STORE', 'config_file'),
            ('LOAD', 'config_file'),
            ('CALL', self._write_config),
        ])

    def _write_config(self):
        config_file = self.vm.registers['config_file']
        if os.path.exists(config_file) and not self.vm.registers['answer_yes']:
            answer = ""

            def ask():
                prompt = "Overwrite %s with default config? [y/N]" % config_file
                try:
                    return input(prompt).lower() or "n"
                except KeyboardInterrupt:
                    print("")
                    return "n"

            answer = ask()
            while not answer.startswith(("y", "n")):
                print("Please answer 'yes' or 'no'")
                answer = ask()
            if answer.startswith("n"):
                return

        config_text = self.generate_config_file()
        if isinstance(config_text, bytes):
            config_text = config_text.decode("utf8")
        print("Writing default config to: %s" % config_file)
        ensure_dir_exists(os.path.abspath(os.path.dirname(config_file)), 0o700)
        with open(config_file, mode="w", encoding="utf-8") as f:
            f.write(config_text)

    def migrate_config(self):
        if os.path.exists(os.path.join(self.vm.registers['config_dir'], "migrated")):
            return

        from .migrate import get_ipython_dir, migrate

        if not os.path.exists(get_ipython_dir()):
            return

        migrate()

    def load_config_file(self, suppress_errors=True):
        self.vm.execute([
            ('LOAD', 'config_file_paths'),
            ('CALL', self._load_config),
        ])

    def _load_config(self):
        base_config = "jupyter_config"
        try:
            super().load_config_file(base_config, path=self.config_file_paths)
        except ConfigFileNotFound:
            pass

        config_file = self.vm.registers['config_file']
        if config_file:
            path, config_file_name = os.path.split(config_file)
        else:
            path = self.config_file_paths
            config_file_name = self.vm.registers['config_file_name']

            if not config_file_name or (config_file_name == base_config):
                return

        try:
            super().load_config_file(config_file_name, path=path)
        except ConfigFileNotFound:
            pass
        except Exception:
            if (not suppress_errors) or self.raise_config_file_errors:
                raise

    def _find_subcommand(self, name):
        name = f"{self.name}-{name}"
        return which(name)

    @property
    def _dispatching(self):
        return bool(self.vm.registers['generate_config'] or self.subapp or self.vm.registers['subcommand'])

    @catch_config_error
    def initialize(self, argv=None):
        if argv is None:
            argv = sys.argv[1:]
        self.vm.registers['argv'] = argv
        if argv:
            subc = self._find_subcommand(argv[0])
            if subc:
                self.vm.registers['subcommand'] = subc
                return
        self.parse_command_line(argv)
        cl_config = deepcopy(self.config)
        if self._dispatching:
            return
        self.migrate_config()
        self.load_config_file()
        self.update_config(cl_config)
        if allow_insecure_writes:
            issue_insecure_write_warning()

    def start(self):
        self.vm.execute([
            ('LOAD', 'subcommand'),
            ('JZ', 3),
            ('CALL', self._exec_subcommand),
            ('JMP', 10),
            ('LOAD', 'subapp'),
            ('JZ', 7),
            ('CALL', self._exec_subapp),
            ('JMP', 10),
            ('LOAD', 'generate_config'),
            ('JZ', 10),
            ('CALL', self.write_default_config),
            ('JMP', 10),
        ])

    def _exec_subcommand(self):
        subcommand = self.vm.registers['subcommand']
        os.execv(subcommand, [subcommand] + self.vm.registers['argv'][1:])
        raise NoStart()

    def _exec_subapp(self):
        self.subapp.start()
        raise NoStart()

    @classmethod
    def launch_instance(cls, argv=None, **kwargs):
        try:
            return super().launch_instance(argv=argv, **kwargs)
        except NoStart:
            return

if __name__ == "__main__":
    JupyterApp.launch_instance()