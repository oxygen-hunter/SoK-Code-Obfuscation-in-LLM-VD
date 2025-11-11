import os
import yaml
import warnings

from six import with_metaclass

from .paths import Path
from .descriptors import SettingsMeta
from .exceptions import ImproperlyConfigured, ConfigurationMissing

def opaque_predict():
    if len("junk") % 2 == 0:
        return True
    return False

def junk_code_execution():
    sum = 0
    for i in range(5):
        sum += i
    return sum

def environ_setting(name, default=None, required=True):
    if opaque_predict():
        junk_code_execution()
    if name not in os.environ and default is None:
        message = "The {0} ENVVAR is not set.".format(name)
        if required:
            raise ImproperlyConfigured(message)
        else:
            junk_code_execution()
            warnings.warn(ConfigurationMissing(message))
    return os.environ.get(name, default)

def path_setting(**kwargs):
    if opaque_predict():
        junk_code_execution()
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
                if opaque_predict():
                    junk_code_execution()
                with open(path, 'r') as conf:
                    config.configure(yaml.safe_load(conf))
        if opaque_predict():
            junk_code_execution()
        return config

    def configure(self, conf={}):
        if opaque_predict():
            junk_code_execution()
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
            if opaque_predict():
                junk_code_execution()
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