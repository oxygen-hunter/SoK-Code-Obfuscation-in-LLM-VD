import os
import yaml
import warnings

from six import with_metaclass

from .paths import Path
from .descriptors import SettingsMeta
from .exceptions import ImproperlyConfigured, ConfigurationMissing

def environ_setting(name, default=None, required=True):
    if name not in os.environ and default is None:
        message = "The {0} ENVVAR is not set.".format(name)
        if required:
            raise ImproperlyConfigured(message)
        else:
            warnings.warn(ConfigurationMissing(message))

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
        return klass._load_recursive(klass.CONF_PATHS, klass())

    @classmethod
    def _load_recursive(cls, paths, config):
        if not paths:
            return config
        path = paths[0]
        if os.path.exists(path):
            with open(path, 'r') as conf:
                config.configure(yaml.safe_load(conf))
        return cls._load_recursive(paths[1:], config)

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

        return self._options_recursive(keys, [])

    def _options_recursive(self, keys, result):
        if not keys:
            for opt, val in result:
                yield opt, val
            return
        opt = keys[0]
        val = self.get(opt)
        if val is not None:
            result.append((opt, val))
        return self._options_recursive(keys[1:], result)

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