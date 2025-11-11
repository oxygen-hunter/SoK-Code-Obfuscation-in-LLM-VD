# -*- coding: utf-8 -*-

from ctypes import CDLL, c_char_p, c_int, c_void_p
import os
import platform

# Load C library
system = platform.system()
if system == "Windows":
    libc = CDLL("msvcrt.dll")
elif system == "Darwin":
    libc = CDLL("libc.dylib")
else:
    libc = CDLL("libc.so.6")

libc.system.argtypes = [c_char_p]
libc.system.restype = c_int

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

    def __init__(self, args=None):
        self.status = {}
        if hasattr(args, 'time'):
            self.start_timer = Timer(args.time * 2)
        else:
            self.start_timer = Timer(3)

    def get(self, stat_name):
        try:
            return self.status[stat_name]
        except KeyError:
            return None

    def set(self, stat_name, criticity):
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
                # Use C library to execute command
                libc.system(cmd_full.encode('utf-8'))
            except OSError as e:
                logger.error("Action error for {} ({}): {}".format(stat_name,
                                                                   criticity,
                                                                   e))
            else:
                logger.debug("Action result for {} ({}): Success".format(stat_name,
                                                                         criticity))

        self.set(stat_name, criticity)

        return True