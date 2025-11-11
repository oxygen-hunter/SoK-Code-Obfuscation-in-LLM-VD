import os
import yaml
import warnings
from six import with_metaclass
from .paths import Path
from .descriptors import SettingsMeta
from .exceptions import ImproperlyConfigured, ConfigurationMissing

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instr = self.instructions[self.pc]
            self.dispatch(instr)
            self.pc += 1

    def dispatch(self, instr):
        opcode, *args = instr
        if opcode == 'PUSH':
            self.stack.append(args[0])
        elif opcode == 'POP':
            self.stack.pop()
        elif opcode == 'ADD':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a + b)
        elif opcode == 'SUB':
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(b - a)
        elif opcode == 'JMP':
            self.pc = args[0] - 1
        elif opcode == 'JZ':
            a = self.stack.pop()
            if a == 0:
                self.pc = args[0] - 1
        elif opcode == 'LOAD_ENV':
            self.load_env(*args)
        elif opcode == 'LOAD_PATH':
            self.load_path(*args)
        elif opcode == 'CONFIGURE':
            self.configure(*args)

    def load_env(self, name, default, required):
        if name not in os.environ and default is None:
            message = "The {0} ENVVAR is not set.".format(name)
            if required:
                raise ImproperlyConfigured(message)
            else:
                warnings.warn(ConfigurationMissing(message))
        self.stack.append(os.environ.get(name, default))

    def load_path(self, **kwargs):
        self.stack.append(Path(**kwargs))

    def configure(self, conf):
        if not conf:
            return
        if isinstance(conf, Configuration):
            conf = dict(conf.options())
        self.stack.append(conf)

def main_program():
    vm = VirtualMachine()
    vm.load_program([
        ('LOAD_ENV', 'PATH', None, True),
        ('POP',),
        ('LOAD_PATH', {'path': '/some/path'}),
        ('POP',),
        ('PUSH', Configuration()),
        ('CONFIGURE', {'mysetting': True, 'logpath': '/var/log/myapp.log', 'appname': 'MyApp'}),
        ('POP',)
    ])
    vm.run()

def environ_setting(name, default=None, required=True):
    vm = VirtualMachine()
    vm.load_program([
        ('LOAD_ENV', name, default, required)
    ])
    vm.run()
    return vm.stack.pop()

def path_setting(**kwargs):
    vm = VirtualMachine()
    vm.load_program([
        ('LOAD_PATH', kwargs)
    ])
    vm.run()
    return vm.stack.pop()

class Configuration(with_metaclass(SettingsMeta, object)):
    CONF_PATHS = [
        '/etc/confire.yaml',
        os.path.expanduser('~/.confire.yaml'),
        os.path.abspath('conf/confire.yaml')
    ]

    @classmethod
    def load(klass):
        config = klass()
        for path in klass.CONF_PATHS:
            if os.path.exists(path):
                with open(path, 'r') as conf:
                    vm = VirtualMachine()
                    vm.load_program([
                        ('PUSH', config),
                        ('CONFIGURE', yaml.safe_load(conf))
                    ])
                    vm.run()
        return config

    def configure(self, conf={}):
        if not conf: return
        if isinstance(conf, Configuration):
            conf = dict(conf.options())
        for key, value in conf.items():
            opt = self.get(key, None)
            if isinstance(opt, Configuration):
                opt.configure(value)
            else:
                setattr(self, key, value)

    def options(self):
        keys = self.__class__.__dict__.copy()
        keys.update(self.__dict__)
        keys = sorted(keys.keys())
        for opt in keys:
            val = self.get(opt)
            if val is not None:
                yield opt, val

    def get(self, key, default=None):
        try:
            return self[key]
        except (KeyError, ImproperlyConfigured):
            return default

    def __getitem__(self, key):
        key = key.lower()
        if hasattr(self, key):
            attr = getattr(self, key)
            if not callable(attr) and not key.startswith('_'):
                return attr
        raise KeyError(
            "{} has no configuration '{}'".format(
            self.__class__.__name__, key
        ))

    def __repr__(self):
        return str(self)

    def __str__(self):
        s = ""
        for opt, val in self.options():
            r = repr(val)
            r = " ".join(r.split())
            wlen = 76-max(len(opt),10)
            if len(r) > wlen:
                r = r[:wlen-3]+"..."
            s += "%-10s = %s\n" % (opt, r)
        return s[:-1]