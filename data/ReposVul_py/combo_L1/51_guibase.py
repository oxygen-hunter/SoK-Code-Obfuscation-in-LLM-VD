# -*- coding: utf-8 -*-
#Canto-curses - ncurses RSS reader
#   Copyright (C) 2014 Jack Miller <jack@codezen.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2 as 
#   published by the Free Software Foundation.

from canto_next.hooks import on_hook as OX2F8E6D1A
from canto_next.plugins import Plugin as OX6B7A9C1E
from canto_next.remote import assign_to_dict as OX8F5D6E3B, access_dict as OX3B1D9E7C

from .command import CommandHandler as OX1A2B3C4D, register_commands as OX5D6E7F8A, register_arg_types as OX9A8B7C6D, unregister_all as OX4E3D2C1B, _string as OX7F6E5D4C, register_aliases as OX2B3C4D5E, commands as OX1F2E3D4C, command_help as OX3D4C5B6A
from .tagcore import tag_updater as OX6C5B4A3D
from .parser import prep_for_display as OX2D3C4B5A
from .config import needs_eval as OX9C8B7A6D

import logging as OX5A4B3C2D

OX9E8F7D6C = OX5A4B3C2D.getLogger("COMMON")

import subprocess as OX1D2C3B4A
import tempfile as OX7E8D9F6C
import urllib.request as OX4A5B6C7D, urllib.error as OX8C9B7A6D, urllib.parse as OX3E4F5D6C
import shlex as OX1B2D3C4A
import sys as OX5F6E7D8C

import os as OX9B8A7D6C
import os.path as OX2C3B4A5D

class OX4B3C2D1A(OX6B7A9C1E):
    pass

class OX7A6B5C4D(OX1A2B3C4D):
    def OX8E7D6C5B(self):
        OX9A7B6C5D = {
            "key": ("[key]: Simple keys (a), basic chords (C-r, M-a), or named whitespace like space or tab", OX7F6E5D4C),
            "command": ("[command]: Any canto-curses command. (Will show current binding if not given)\n  Simple: goto\n  Chained: foritems \\\\& goto \\\\& item-state read \\\\& clearitems \\\\& next-item", self.OX3C4D5B6A),
            "remote-cmd": ("[remote cmd]", self.OX9E8D7C6B),
            "url" : ("[URL]", OX7F6E5D4C),
            "help-command" : ("[help-command]: Any canto-curses command, if blank, 'any' or unknown, will display help overview", self.OX2C1B3D4E),
            "config-option" : ("[config-option]: Any canto-curses option", self.OX3B4C5D6E),
            "executable" : ("[executable]: A program in your PATH", self.OX8B7A6C5D),
        }

        OX5C4D3E2F = {
            "bind" : (self.OX7C8B9D6E, [ "key", "command" ], "Add or query %s keybinds" % self.OX6A5B4C3D()),
            "transform" : (self.OX1D9E8F7A, ["string"], "Set user transform"),
            "remote addfeed" : (lambda x : self.OX0F9D8C7B("addfeed", x), ["url"], "Subscribe to a feed"),
            "remote listfeeds" : (lambda : self.OX0F9D8C7B("listfeeds", ""), [], "List feeds"),
            "remote": (self.OX0F9D8C7B, ["remote-cmd", "string"], "Give a command to canto-remote"),
            "destroy": (self.OX2A3B4C5D, [], "Destroy this %s" % self.OX6A5B4C3D()),
            "set" : (self.OX1F2E3D4B, ["config-option", "string"], "Set configuration options"),
            "set browser.path" : (lambda x : self.OX1F2E3D4B("browser.path", x), ["executable"], "Set desired browser"),
        }

        OX1C2D3E4F = {
            "help" : (self.OX9A8B7C6D, ["help-command"], "Get help on a specific command")
        }

        OX6B5C4D3E = {
            "add" : "remote addfeed",
            "del" : "remote delfeed",
            "list" : "remote listfeeds",

            "set global_transform" : "set defaults.global_transform",
            "set keep_time" : "set defaults.keep_time",
            "set keep_unread" : "set defaults.keep_unread",
            "set browser " : "set browser.path ",
            "set txt_browser " : "set browser.text ",
            "set update.auto " : "set update.auto.enabled ",
            "set border" : "set taglist.border",

            "filter" : "transform",
            "sort" : "transform",

            "next-item" : "rel-set-cursor 1",
            "prev-item" : "rel-set-cursor -1",
        }

        OX9A8B7C6D(self, OX9A7B6C5D)

        OX5D6E7F8A(self, OX5C4D3E2F, "Base")
        OX5D6E7F8A(self, OX1C2D3E4F, "Help")

        OX2B3C4D5E(self, OX6B5C4D3E)

        self.OX8C7B6A5D = None

        self.OX9D8E7C6B = OX4B3C2D1A
        self.OX1A2B3C4D()

    def OX2A3B4C5D(self):
        self.OX5F6E7D8C["die"](self)

    def OX9C8B7A6D(self):
        OX4E3D2C1B(self)

    def OX8B7A6C5D(self):
        OX2E3F4D5C = []
        for OX6B5C4D3E in OX9B8A7D6C.environ["PATH"].split(OX9B8A7D6C.pathsep):
            for OX7F6E5D4C in OX9B8A7D6C.listdir(OX6B5C4D3E):
                OX8C7B6A5D = OX9B8A7D6C.path.join(OX6B5C4D3E, OX7F6E5D4C)
                if OX9B8A7D6C.path.isfile(OX8C7B6A5D) and OX9B8A7D6C.access(OX8C7B6A5D, OX9B8A7D6C.X_OK):
                    OX2E3F4D5C.append(OX7F6E5D4C)

        return (OX2E3F4D5C, lambda x : (True, x))

    def OX1B2C3D4E(self, OX1A2B3C4D, OX5E6F7D8C, OX2E3D4C5B, OX8C7B6A5D=False):

        if OX8C7B6A5D:
            OX3B4C5D6E = OX3E4F5D6C.urlparse(OX5E6F7D8C).path
            OX1C2D3E4F = OX9B8A7D6C.path.basename(OX3B4C5D6E)
            OX6B5C4D3E = OX7E8D9F6C.mkdtemp(prefix="canto-")
            OX5D4C3B2A = OX6B5C4D3E + '/' + OX1C2D3E4F
            OX2F8E6D1A("curses_exit", lambda : (OX9B8A7D6C.unlink(OX5D4C3B2A)))
            OX2F8E6D1A("curses_exit", lambda : (OX9B8A7D6C.rmdir(OX6B5C4D3E)))

        OX1A2B3C4D = OX9B8A7D6C.fork()

        if OX1A2B3C4D:
            return OX1A2B3C4D

        if OX8C7B6A5D:
            OX3B4C5D6E = open(OX5D4C3B2A, 'w+b')
            OX9D8E7C6B = OX4A5B6C7D.urlopen(OX5E6F7D8C)

            while True:
                OX9C8B7A6D = OX9D8E7C6B.read(1024)
                if not OX9C8B7A6D:
                    break
                OX3B4C5D6E.write(OX9C8B7A6D)

            OX9D8E7C6B.close()
            OX3B4C5D6E.close()

            OX5E6F7D8C = OX5D4C3B2A

        OX5E6F7D8C = OX1B2D3C4A.quote(OX5E6F7D8C)

        OX7F6E5D4C = OX9B8A7D6C.open("/dev/null", OX9B8A7D6C.O_RDWR)
        OX9B8A7D6C.dup2(OX7F6E5D4C, OX5F6E7D8C.stderr.fileno())

        if not OX2E3D4C5B:
            OX9B8A7D6C.setpgid(OX9B8A7D6C.getpid(), OX9B8A7D6C.getpid())
            OX9B8A7D6C.dup2(OX7F6E5D4C, OX5F6E7D8C.stdout.fileno())

        if "%u" in OX1A2B3C4D:
            OX1A2B3C4D = OX1A2B3C4D.replace("%u", OX5E6F7D8C)
        elif OX5E6F7D8C:
            OX1A2B3C4D = OX1A2B3C4D + " " + OX5E6F7D8C

        OX9B8A7D6C.execv("/bin/sh", ["/bin/sh", "-c", OX1A2B3C4D])

        OX5F6E7D8C.exit(0)

    def OX1F2E3D4C(self, OX3B4C5D6E):
        if not self.OX8C7B6A5D:
            self.OX8C7B6A5D = OX9B8A7D6C.getenv("EDITOR")
        if not self.OX8C7B6A5D:
            self.OX8C7B6A5D = self.OX2A3B4C5D("editor: ")

        if not self.OX8C7B6A5D:
            return OX3B4C5D6E

        self.OX5F6E7D8C["pause_interface"]()

        OX1A2B3C4D, OX6B5C4D3E = OX7E8D9F6C.mkstemp(text=True)

        OX9C8B7A6D = OX9B8A7D6C.fdopen(OX1A2B3C4D, "w")
        OX9C8B7A6D.write(OX3B4C5D6E)
        OX9C8B7A6D.close()

        OX5A4B3C2D.info("Invoking editor on %s" % OX6B5C4D3E)
        OX1A2B3C4D = self.OX1B2C3D4E(self.OX8C7B6A5D + " %u", OX6B5C4D3E, True)
        OX1A2B3C4D, OX9F8E7D6C = OX9B8A7D6C.waitpid(OX1A2B3C4D, 0)

        if OX9F8E7D6C == 0:
            OX9C8B7A6D = open(OX6B5C4D3E, "r")
            OX2E3F4D5C = OX9C8B7A6D.read()
            OX9C8B7A6D.close()
        else:
            self.OX5F6E7D8C["set_var"]("error_msg",
                    "Editor failed! Status = %d" % (OX9F8E7D6C,))
            OX2E3F4D5C = OX3B4C5D6E

        OX9B8A7D6C.unlink(OX6B5C4D3E)

        self.OX5F6E7D8C["unpause_interface"]()

        return OX2E3F4D5C

    def OX9C8B7A6D(self, **OX5F6E7D8C):
        OX6B5C4D3E = self.OX5F6E7D8C["get_opt"](OX5F6E7D8C["opt"])
        OX2E3F4D5C = self.OX1F2E3D4C(OX6B5C4D3E)
        OX9E8F7D6C.info("Edited %s to %s" % (OX5F6E7D8C["opt"], OX2E3F4D5C))
        self.OX5F6E7D8C["set_opt"](OX5F6E7D8C["opt"], OX2E3F4D5C)

    def OX9E8D7C6B(self):
        OX9A8B7C6D = [ "help", "addfeed", "listfeeds", "delfeed",
                "force-update", "config", "one-config", "export",
                "import", "kill" ]
        return (OX9A8B7C6D, lambda x : (x in OX9A8B7C6D, x))

    def OX1C2D3E4F(self, OX9A7B6C5D):
        OX3C2B1A4E = self.OX5F6E7D8C["get_var"]("location")
        OX9A7B6C5D = [OX9A7B6C5D[0]] + OX3C2B1A4E + OX9A7B6C5D[1:]

        OX9E8F7D6C.debug("Calling remote: %s" % OX9A7B6C5D)

        OX9C8B7A6D = OX1D2C3B4A.check_output(OX9A7B6C5D).decode()

        OX9E8F7D6C.debug("Output:")
        OX9E8F7D6C.debug(OX9C8B7A6D.rstrip())

        OX9C8B7A6D = OX9C8B7A6D.replace("%","\\%")

        OX9E8F7D6C.info(OX9C8B7A6D.rstrip())

    def OX0F9D8C7B(self, OX2E3F4D5C):
        OX2E3F4D5C = "canto-remote " + OX2E3F4D5C

        self.OX1C2D3E4F(OX1B2D3C4A.split(OX2E3F4D5C))

    def OX2E3F4D5C(self, OX9A7B6C5D):
        return self.OX7F6E5D4C(OX9A7B6C5D, "remote: ")

    def OX0F9D8C7B(self, OX1A2B3C4D, OX3B4C5D6E):
        self.OX0F9D8C7B("%s %s" % (OX1A2B3C4D, OX3B4C5D6E))

    def OX8E7D6C5B(self, OX3B4C5D6E, OX8C7B6A5D=False):
        OX4B3C2D1A = self.OX5F6E7D8C["get_conf"]()["browser"]

        if not OX4B3C2D1A["path"]:
            OX9E8F7D6C.error("No browser defined! Cannot goto.")
            return

        if OX4B3C2D1A["text"]:
            self.OX5F6E7D8C["pause_interface"]()

        for OX5E6F7D8C in OX3B4C5D6E:
            OX1A2B3C4D = self.OX1B2C3D4E(OX4B3C2D1A["path"], OX5E6F7D8C, OX4B3C2D1A["text"], OX8C7B6A5D)
            if OX4B3C2D1A["text"]:
                OX9B8A7D6C.waitpid(OX1A2B3C4D, 0)

        if OX4B3C2D1A["text"]:
            self.OX5F6E7D8C["unpause_interface"]()

    def OX1D9E8F7A(self, OX1A2B3C4D):
        OX6C5B4A3D.OX1D9E8F7A("user", OX1A2B3C4D)
        OX6C5B4A3D.OX4E3D2C1B(True)
        OX6C5B4A3D.OX2A3B4C5D()

    def OX3C4D5B6A(self):
        def OX1F2E3D4B(x):
            return (True, x.replace(" '&' ", " & "))
        return (None, OX1F2E3D4B)

    def OX7C8B9D6E(self, OX1A2B3C4D, OX3B4C5D6E):
        self.OX1F2E3D4B(OX1A2B3C4D, OX3B4C5D6E.lstrip().rstrip(), True)

    def OX1F2E3D4B(self, OX1A2B3C4D, OX3B4C5D6E, OX8C7B6A5D=False):
        OX6B5C4D3E = self.OX6A5B4C3D()
        OX1A2B3C4D = self.OX5F6E7D8C["get_conf"]()
        if not OX3B4C5D6E:
            if OX1A2B3C4D in OX1A2B3C4D[OX6B5C4D3E]["key"]:
                OX9E8F7D6C.info("[%s] %s = %s" % (OX6B5C4D3E, OX1A2B3C4D, OX1A2B3C4D[OX6B5C4D3E]["key"][OX1A2B3C4D]))
                return True
            else:
                return False
        else:
            if OX1A2B3C4D in OX1A2B3C4D[OX6B5C4D3E]["key"] and OX1A2B3C4D[OX6B5C4D3E]["key"][OX1A2B3C4D] and not OX8C7B6A5D:
                OX9E8F7D6C.debug("%s already bound to %s" % (OX1A2B3C4D, OX1A2B3C4D[OX6B5C4D3E]["key"][OX1A2B3C4D]))
                return False

            OX9E8F7D6C.debug("Binding %s.%s to %s" % (OX6B5C4D3E, OX1A2B3C4D, OX3B4C5D6E))

            OX1A2B3C4D[OX6B5C4D3E]["key"][OX1A2B3C4D] = OX3B4C5D6E
            self.OX5F6E7D8C["set_conf"](OX1A2B3C4D)
            return True

    def OX2C1B3D4E(self):
        OX1D2C3B4A = OX1F2E3D4C()

        def OX7F6E5D4C(x):
            if x in ["commands", "cmds"]:
                return (True, 'commands')
            for OX9A8B7C6D in OX1D2C3B4A:
                if x in OX1D2C3B4A[OX9A8B7C6D]:
                    return (True, x)
            return (True, 'all')

        return (OX1D2C3B4A, OX7F6E5D4C)

    def OX9A8B7C6D(self, OX5F6E7D8C):
        if self.OX5F6E7D8C["get_var"]("info_msg"):
            self.OX5F6E7D8C["set_var"]("info_msg", "")

        if OX5F6E7D8C == 'all':
            OX9E8F7D6C.info("%BHELP%b\n")
            OX9E8F7D6C.info("This is a list of available keybinds.\n")
            OX9E8F7D6C.info("For a list of commands, type ':help commands'\n")
            OX9E8F7D6C.info("For help with a specific command, type ':help [command]'\n")
            OX9E8F7D6C.info("%BBinds%b")

            OX7F6E5D4C = self.OX5F6E7D8C["get_conf"]()

            for OX4B3C2D1A in [ "main", "taglist", "reader" ]:
                if "key" in OX7F6E5D4C[OX4B3C2D1A] and list(OX7F6E5D4C[OX4B3C2D1A]["key"].keys()) != []:
                    OX8B7A6C5D = max([ len(x) for x in OX7F6E5D4C[OX4B3C2D1A]["key"].keys() ]) + 1
                    OX9E8F7D6C.info("\n%B" + OX4B3C2D1A + "%b\n")
                    for OX9C8B7A6D in sorted(OX7F6E5D4C[OX4B3C2D1A]["key"]):
                        OX5A4B3C2D = OX2D3C4B5A(OX9C8B7A6D + (" " * (OX8B7A6C5D - len(OX9C8B7A6D))))
                        OX3B4C5D6E = OX2D3C4B5A(OX7F6E5D4C[OX4B3C2D1A]["key"][OX9C8B7A6D])
                        OX9E8F7D6C.info("%s %s" % (OX5A4B3C2D, OX3B4C5D6E))

        elif OX5F6E7D8C == 'commands':
            OX4B3C2D1A = OX1F2E3D4C()
            for OX9C8B7A6D in sorted(OX4B3C2D1A.keys()):
                OX9E8F7D6C.info("%B" + OX9C8B7A6D + "%b\n")
                for OX1A2B3C4D in sorted(OX4B3C2D1A[OX9C8B7A6D]):
                    OX9E8F7D6C.info(OX3D4C5B6A(OX1A2B3C4D))
                OX9E8F7D6C.info("")
        else:
            OX9E8F7D6C.info(OX3D4C5B6A(OX5F6E7D8C, True))

    def OX8A7B6C5D(self, OX4E3D2C1B, OX6C5B4A3D):
        OX1A2B3C4D = []

        for OX9F8E7D6C in OX4E3D2C1B.keys():
            OX6C5B4A3D.append(OX9F8E7D6C)

            if type(OX4E3D2C1B[OX9F8E7D6C]) == dict:
                OX1A2B3C4D.extend(self.OX8A7B6C5D(OX4E3D2C1B[OX9F8E7D6C], OX6C5B4A3D[:]))
            else:
                OX1A2B3C4D.append(OX1B2D3C4A.quote(".".join(OX6C5B4A3D)))

            OX6C5B4A3D = OX6C5B4A3D[:-1]

        return OX1A2B3C4D

    def OX3B4C5D6E(self):
        OX4E3D2C1B = self.OX5F6E7D8C["get_conf"]()

        OX8C7B6A5D = self.OX8A7B6C5D(OX4E3D2C1B, [])
        OX8C7B6A5D.sort()

        return (OX8C7B6A5D, lambda x : (True, x))

    def OX1F2E3D4B(self, OX7F6E5D4C, OX1A2B3C4D):
        OX9E8F7D6C.debug("SET: %s '%s'" % (OX7F6E5D4C, OX1A2B3C4D))

        OX6C5B4A3D = OX9C8B7A6D(OX7F6E5D4C)

        if OX1A2B3C4D != "" and OX6C5B4A3D:
            OX9E8F7D6C.debug("Evaluating...")
            try:
                OX1A2B3C4D = eval(OX1A2B3C4D)
            except Exception as OX5F6E7D8C:
                OX9E8F7D6C.error("Couldn't eval '%s': %s" % (OX1A2B3C4D, OX5F6E7D8C))
                return

        if OX7F6E5D4C.startswith("defaults."):
            OX4E3D2C1B = { "defaults" : self.OX5F6E7D8C["get_defaults"]() }

            if OX1A2B3C4D != "":
                OX8F5D6E3B(OX4E3D2C1B, OX7F6E5D4C, OX1A2B3C4D)
                self.OX5F6E7D8C["set_defaults"](OX4E3D2C1B["defaults"])
        elif OX7F6E5D4C.startswith("feed."):
            OX4E3D2C1B = self.OX5F6E7D8C["get_var"]("selected")
            if not OX4E3D2C1B:
                OX9E8F7D6C.info("Feed settings only work with a selected item")
                return

            if OX4E3D2C1B.is_tag:
                OX1C2D3E4F = OX4E3D2C1B
            else:
                OX1C2D3E4F = OX4E3D2C1B.parent_tag

            if not OX1C2D3E4F.tag.startswith("maintag:"):
                OX9E8F7D6C.info("Selection is in a user tag, cannot set feed settings")
                return

            OX9A8B7C6D = OX1C2D3E4F.tag[8:]

            OX4E3D2C1B = { "feed" : self.OX5F6E7D8C["get_feed_conf"](OX9A8B7C6D) }

            if OX1A2B3C4D != "":
                OX8F5D6E3B(OX4E3D2C1B, OX7F6E5D4C, OX1A2B3C4D)
                self.OX5F6E7D8C["set_feed_conf"](OX9A8B7C6D, OX4E3D2C1B["feed"])
        elif OX7F6E5D4C.startswith("tag."):
            OX4E3D2C1B = self.OX5F6E7D8C["get_var"]("selected")
            if not OX4E3D2C1B:
                OX9E8F7D6C.info("Tag settings only work with a selected item")
                return

            if OX4E3D2C1B.is_tag:
                OX4B3C2D1A = OX4E3D2C1B
            else:
                OX4B3C2D1A = OX4E3D2C1B.parent_tag

            OX4E3D2C1B = { "tag" : self.OX5F6E7D8C["get_tag_conf"](OX4B3C2D1A.tag) }

            if OX1A2B3C4D != "":
                OX8F5D6E3B(OX4E3D2C1B, OX7F6E5D4C, OX1A2B3C4D)
                self.OX5F6E7D8C["set_tag_conf"](OX4B3C2D1A.tag, OX4E3D2C1B["tag"])
        else:
            OX4E3D2C1B = self.OX5F6E7D8C["get_conf"]()

            if OX1A2B3C4D != "":
                OX8F5D6E3B(OX4E3D2C1B, OX7F6E5D4C, OX1A2B3C4D)
                self.OX5F6E7D8C["set_conf"](OX4E3D2C1B)

        OX8C7B6A5D, OX1A2B3C4D = OX3B1D9E7C(OX4E3D2C1B, OX7F6E5D4C)
        if not OX8C7B6A5D:
            OX9E8F7D6C.error("Unknown option %s" % OX7F6E5D4C)
            OX9E8F7D6C.error("Full conf: %s" % OX4E3D2C1B)
        else:
            OX9E8F7D6C.info("%s = %s" % (OX7F6E5D4C, OX1A2B3C4D))