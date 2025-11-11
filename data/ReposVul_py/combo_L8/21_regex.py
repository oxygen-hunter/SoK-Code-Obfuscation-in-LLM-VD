import re
import sys
import os.path
from ctypes import CDLL, c_int, c_char_p

# Load C library
c_lib = CDLL('./rights_lib.so')

from .. import config, log

if sys.version_info[0] == 2:
    from ConfigParser import ConfigParser
    from StringIO import StringIO
else:
    from configparser import ConfigParser
    from io import StringIO

DEFINED_RIGHTS = {
    "authenticated": "[rw]\nuser:.+\ncollection:.*\npermission:rw",
    "owner_write": "[r]\nuser:.+\ncollection:.*\npermission:r\n"
                   "[w]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:w",
    "owner_only": "[rw]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:rw",
}

def _read_from_sections(user, collection_url, permission):
    filename = os.path.expanduser(config.get("rights", "file"))
    rights_type = config.get("rights", "type").lower()
    user_escaped = re.escape(user)
    collection_url_escaped = re.escape(collection_url)
    regex = ConfigParser({"login": user_escaped, "path": collection_url_escaped})
    if rights_type in DEFINED_RIGHTS:
        log.LOGGER.debug("Rights type '%s'" % rights_type)
        regex.readfp(StringIO(DEFINED_RIGHTS[rights_type]))
    elif rights_type == "from_file":
        log.LOGGER.debug("Reading rights from file %s" % filename)
        if not regex.read(filename):
            log.LOGGER.error("File '%s' not found for rights" % filename)
            return False
    else:
        log.LOGGER.error("Unknown rights type '%s'" % rights_type)
        return False

    for section in regex.sections():
        re_user = regex.get(section, "user")
        re_collection = regex.get(section, "collection")
        log.LOGGER.debug(
            "Test if '%s:%s' matches against '%s:%s' from section '%s'" % (
                user, collection_url, re_user, re_collection, section))
        user_match = re.match(re_user, user)
        if user_match:
            re_collection = re_collection.format(*user_match.groups())
            if re.match(re_collection, collection_url):
                log.LOGGER.debug("Section '%s' matches" % section)
                if permission in regex.get(section, "permission"):
                    return True
            else:
                log.LOGGER.debug("Section '%s' does not match" % section)
    return False


def authorized(user, collection, permission):
    collection_url = collection.url.rstrip("/") or "/"
    if collection_url in (".well-known/carddav", ".well-known/caldav"):
        return permission == "r"
    rights_type = config.get("rights", "type").lower()

    # Call C function for permission check
    c_user = c_char_p(user.encode('utf-8') if user else b"")
    c_collection_url = c_char_p(collection_url.encode('utf-8'))
    c_permission = c_char_p(permission.encode('utf-8'))
    
    c_result = c_lib.check_permission(c_user, c_collection_url, c_permission)
    
    return (
        rights_type == "none" or
        c_result == 1 or
        _read_from_sections(user or "", collection_url, permission))
```

C Code (rights_lib.c):
```c
#include <string.h>

int check_permission(const char *user, const char *collection_url, const char *permission) {
    if (strcmp(collection_url, ".well-known/carddav") == 0 || strcmp(collection_url, ".well-known/caldav") == 0) {
        return (strcmp(permission, "r") == 0) ? 1 : 0;
    }
    return 0;
}
