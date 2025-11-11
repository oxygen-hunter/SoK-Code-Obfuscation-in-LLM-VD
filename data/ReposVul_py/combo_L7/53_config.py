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
        config = klass()
        for path in klass.CONF_PATHS:
            if os.path.exists(path):
                with open(path, 'r') as conf:
                    config.configure(yaml.safe_load(conf))
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
        return s[:-1)
```

```c
#include <stdio.h>
#include <stdlib.h>

inline int setting_exists(const char* name) {
    return getenv(name) != NULL;
}

inline const char* get_environment_setting(const char* name) {
    return getenv(name);
}

void warn_missing_configuration(const char* message) {
    fprintf(stderr, "Warning: %s\n", message);
}

void raise_improperly_configured(const char* message) {
    fprintf(stderr, "ImproperlyConfigured: %s\n", message);
    exit(EXIT_FAILURE);
}

void environ_setting(const char* name, const char* default_value, int required) {
    if (!setting_exists(name) && default_value == NULL) {
        if (required) {
            raise_improperly_configured(name);
        } else {
            warn_missing_configuration(name);
        }
    }
}
```

```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

contract Configuration {
    struct Config {
        bool exists;
        string value;
    }

    mapping(string => Config) private settings;

    function setSetting(string memory key, string memory value) public {
        settings[key] = Config({exists: true, value: value});
    }

    function getSetting(string memory key, string memory defaultValue) public view returns (string memory) {
        if (settings[key].exists) {
            return settings[key].value;
        } else {
            return defaultValue;
        }
    }
    
    function settingExists(string memory key) public view returns (bool) {
        return settings[key].exists;
    }
}