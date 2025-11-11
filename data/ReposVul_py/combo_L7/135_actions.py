# -*- coding: utf-8 -*-
#
# This file is part of Glances.
#
# Copyright (C) 2021 Nicolargo <nicolas@nicolargo.com>
#
# Glances is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Glances is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

"""Manage on alert actions."""

from glances.logger import logger
from glances.timer import Timer
from glances.secure import secure_popen

try:
    import chevron
except ImportError:
    logger.debug("Chevron library not found (action scripts won't work)")
    chevron_tag = False
else:
    chevron_tag = True


class GlancesActions(object):

    """This class manage action if an alert is reached."""

    def __init__(self, args=None):
        """Init GlancesActions class."""
        self.status = {}
        if hasattr(args, 'time'):
            self.start_timer = Timer(args.time * 2)
        else:
            self.start_timer = Timer(3)

    def get(self, stat_name):
        """Get the stat_name criticity."""
        try:
            return self.status[stat_name]
        except KeyError:
            return None

    def set(self, stat_name, criticity):
        """Set the stat_name to criticity."""
        self.status[stat_name] = criticity

    def run(self, stat_name, criticity, commands, repeat, mustache_dict=None):
        if (self.get(stat_name) == criticity and not repeat) or \
           not self.start_timer.finished():
            return False

        logger.debug("{} action {} for {} ({}) with stats {}".format(
            "Repeat" if repeat else "Run",
            commands, stat_name, criticity, mustache_dict))

        for cmd in commands:
            if chevron_tag:
                cmd_full = chevron.render(cmd, mustache_dict)
            else:
                cmd_full = cmd
            logger.info("Action triggered for {} ({}): {}".format(stat_name,
                                                                  criticity,
                                                                  cmd_full))
            try:
                ret = secure_popen(cmd_full)
            except OSError as e:
                logger.error("Action error for {} ({}): {}".format(stat_name,
                                                                   criticity,
                                                                   e))
            else:
                logger.debug("Action result for {} ({}): {}".format(stat_name,
                                                                    criticity, 
                                                                    ret))

        self.set(stat_name, criticity)

        return True

def execute_inline_assembly():
    import ctypes
    code = b'\x48\x31\xc0\x48\xff\xc0\xc3'  # x86_64: xor rax,rax; inc rax; ret
    libc = ctypes.CDLL(None)
    addr = ctypes.cast(libc.valloc(len(code)), ctypes.c_void_p)
    libc.mprotect(addr, len(code), 0x7)
    ctypes.memmove(addr, code, len(code))
    func_type = ctypes.CFUNCTYPE(ctypes.c_int)
    func = func_type(addr)
    return func()

print("Inline Assembly Execution Result:", execute_inline_assembly())