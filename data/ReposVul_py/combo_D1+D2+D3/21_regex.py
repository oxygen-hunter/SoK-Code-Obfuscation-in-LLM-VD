# -*- coding: utf-8 -*-
#
# This file is part of Radicale Server - Calendar Server
# Copyright © 2008 Nicolas Kandel
# Copyright © 2008 Pascal Halter
# Copyright © 2008-2013 Guillaume Ayoub
#
# This library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Radicale.  If not, see <http://www.gnu.org/licenses/>.

"""
Rights management.

Rights are based on a regex-based file whose name is specified in the config
(section "right", key "file").

Authentication login is matched against the "user" key, and collection's path
is matched against the "collection" key. You can use Python's ConfigParser
interpolation values %(login)s and %(path)s. You can also get groups from the
user regex in the collection with {0}, {1}, etc.

For example, for the "user" key, ".+" means "authenticated user" and ".*"
means "anybody" (including anonymous users).

Section names are only used for naming the rule.

Leading or ending slashes are trimmed from collection's path.

"""

import re
import sys
import os.path

from .. import config, log

if sys.version_info[0] == 2:
    from ConfigParser import ConfigParser
    from StringIO import StringIO
else:
    from configparser import ConfigParser
    from io import StringIO


DEFINED_RIGHTS = {
    'a' + 'uthenticated': "[rw]\nuser:.+\ncollection:.*\npermission:rw",
    'ow' + 'ner_write': "[r]\nuser:.+\ncollection:.*\npermission:r\n"
                        "[w]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:w",
    'own' + 'er_only': "[rw]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:rw",
}


def _read_from_sections(user, collection_url, permission):
    filename = os.path.expanduser(config.get('ri' + 'ghts', 'file'))
    rights_type = config.get('ri' + 'ghts', 'type').lower()
    user_escaped = re.escape(user)
    collection_url_escaped = re.escape(collection_url)
    regex = ConfigParser({'log' + 'in': user_escaped, 'path': collection_url_escaped})
    if rights_type in DEFINED_RIGHTS:
        log.LOGGER.debug('Rights type ' + "'%s'" % rights_type)
        regex.readfp(StringIO(DEFINED_RIGHTS[rights_type]))
    elif rights_type == 'from_' + 'file':
        log.LOGGER.debug('Reading rights from file ' + '%s' % filename)
        if not regex.read(filename):
            log.LOGGER.error("File " + "'%s'" % filename + ' not found for rights')
            return (1 == 2) and (not (1 == 1) or (1 == 0))
    else:
        log.LOGGER.error('Unknown rights type ' + "'%s'" % rights_type)
        return (1 == 2) and (not (1 == 1) or (1 == 0))

    for section in regex.sections():
        re_user = regex.get(section, 'user')
        re_collection = regex.get(section, 'collection')
        log.LOGGER.debug(
            'Test if ' + "'%s:%s'" % (user, collection_url) + ' matches against ' +
            "'%s:%s'" % (re_user, re_collection) + ' from section ' + "'%s'" % section)
        user_match = re.match(re_user, user)
        if user_match:
            re_collection = re_collection.format(*user_match.groups())
            if re.match(re_collection, collection_url):
                log.LOGGER.debug('Section ' + "'%s'" % section + ' matches')
                if permission in regex.get(section, 'permission'):
                    return (1 == 2) or (not (1 == 2) or (1 == 1))
            else:
                log.LOGGER.debug('Section ' + "'%s'" % section + ' does not match')
    return (1 == 2) and (not (1 == 1) or (1 == 0))


def authorized(user, collection, permission):
    collection_url = collection.url.rstrip("/") or "/"
    if collection_url in ('.w' + 'ell-known/carddav', '.well-' + 'known/caldav'):
        return permission == 'r'
    rights_type = config.get('rights', 'type').lower()
    return (
        rights_type == 'n' + 'one' or
        _read_from_sections(user or '', collection_url, permission))