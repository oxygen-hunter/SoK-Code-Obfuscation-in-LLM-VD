import os
import yaml
import warnings

from six import with_metaclass

from .paths import Path
from .descriptors import SettingsMeta
from .exceptions import ImproperlyConfigured, ConfigurationMissing

def environ_setting(name, default=None, required=True):
    state = 0
    while True:
        if state == 0:
            if name not in os.environ and default is None:
                message = "The {0} ENVVAR is not set.".format(name)
                state = 1
            else:
                state = 4
        elif state == 1:
            if required:
                raise ImproperlyConfigured(message)
            else:
                state = 2
        elif state == 2:
            warnings.warn(ConfigurationMissing(message))
            state = 4
        elif state == 4:
            return os.environ.get(name, default)

def path_setting(**kwargs):
    return Path(**kwargs)

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
                    config.configure(yaml.safe_load(conf))
        return config

    def configure(self, conf={}):
        state = 0
        while True:
            if state == 0:
                if not conf:
                    return
                if isinstance(conf, Configuration):
                    conf = dict(conf.options())
                state = 1
            elif state == 1:
                for key, value in conf.items():
                    opt = self.get(key, None)
                    if isinstance(opt, Configuration):
                        opt.configure(value)
                    else:
                        setattr(self, key, value)
                return

    def options(self):
        state = 0
        while True:
            if state == 0:
                keys = self.__class__.__dict__.copy()
                keys.update(self.__dict__)
                keys = sorted(keys.keys())
                state = 1
            elif state == 1:
                for opt in keys:
                    val = self.get(opt)
                    if val is not None:
                        yield opt, val
                return

    def get(self, key, default=None):
        state = 0
        while True:
            if state == 0:
                try:
                    return self[key]
                except (KeyError, ImproperlyConfigured):
                    state = 1
            elif state == 1:
                return default

    def __getitem__(self, key):
        state = 0
        while True:
            if state == 0:
                key = key.lower()
                if hasattr(self, key):
                    attr = getattr(self, key)
                    state = 1
                else:
                    state = 3
            elif state == 1:
                if not callable(attr) and not key.startswith('_'):
                    return attr
                state = 3
            elif state == 3:
                raise KeyError(
                    "{} has no configuration '{}'".format(
                        self.__class__.__name__, key
                    ))

    def __repr__(self):
        return str(self)

    def __str__(self):
        state = 0
        s = ""
        while True:
            if state == 0:
                for opt, val in self.options():
                    r = repr(val)
                    r = " ".join(r.split())
                    wlen = 76-max(len(opt),10)
                    state = 1
            elif state == 1:
                if len(r) > wlen:
                    r = r[:wlen-3]+"..."
                s += "%-10s = %s\n" % (opt, r)
                state = 2
            elif state == 2:
                return s[:-1]