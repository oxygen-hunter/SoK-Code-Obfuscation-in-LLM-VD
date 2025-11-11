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


class StackVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            op = self.instructions[self.pc]
            self.pc += 1
            if op == "PUSH":
                value = self.instructions[self.pc]
                self.pc += 1
                self.stack.append(value)
            elif op == "POP":
                self.stack.pop()
            elif op == "ADD":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif op == "SUB":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(b - a)
            elif op == "JMP":
                self.pc = self.instructions[self.pc]
            elif op == "JZ":
                pos = self.instructions[self.pc]
                self.pc += 1
                if self.stack[-1] == 0:
                    self.pc = pos
            elif op == "LOAD":
                index = self.instructions[self.pc]
                self.pc += 1
                self.stack.append(self.stack[index])
            elif op == "STORE":
                index = self.instructions[self.pc]
                self.pc += 1
                self.stack[index] = self.stack.pop()
            elif op == "LOG":
                logger.debug(self.stack.pop())
            elif op == "SECURE_POP":
                cmd = self.stack.pop()
                try:
                    ret = secure_popen(cmd)
                    logger.debug(ret)
                except OSError as e:
                    logger.error(e)
            elif op == "EXIT":
                break


class GlancesActions(object):
    def __init__(self, args=None):
        self.status = {}
        if hasattr(args, 'time'):
            self.start_timer = Timer(args.time * 2)
        else:
            self.start_timer = Timer(3)

    def get(self, stat_name):
        return self.status.get(stat_name, None)

    def set(self, stat_name, criticity):
        self.status[stat_name] = criticity

    def run(self, stat_name, criticity, commands, repeat, mustache_dict=None):
        vm = StackVM()
        if (self.get(stat_name) == criticity and not repeat) or not self.start_timer.finished():
            return False
        vm.load_instructions([
            "PUSH", "Repeat" if repeat else "Run",
            "PUSH", commands,
            "PUSH", stat_name,
            "PUSH", criticity,
            "PUSH", mustache_dict,
            "LOG",
            "PUSH", logger.info,
            "STORE", 0,
            "PUSH", logger.error,
            "STORE", 1,
            "PUSH", logger.debug,
            "STORE", 2
        ])
        for cmd in commands:
            if chevron_tag:
                cmd_full = chevron.render(cmd, mustache_dict)
            else:
                cmd_full = cmd
            vm.load_instructions([
                "PUSH", "Action triggered for {} ({}): {}".format(stat_name, criticity, cmd_full),
                "LOAD", 0,
                "LOG",
                "PUSH", cmd_full,
                "SECURE_POP",
                "LOAD", 2,
                "LOG"
            ])
            try:
                ret = secure_popen(cmd_full)
                vm.load_instructions([
                    "PUSH", "Action result for {} ({}): {}".format(stat_name, criticity, ret),
                    "LOAD", 2,
                    "LOG"
                ])
            except OSError as e:
                vm.load_instructions([
                    "PUSH", "Action error for {} ({}): {}".format(stat_name, criticity, e),
                    "LOAD", 1,
                    "LOG"
                ])
        self.set(stat_name, criticity)
        return True