# -*- coding: utf-8 -*-
# Canto-curses - ncurses RSS reader
# Copyright (C) 2014 Jack Miller <jack@codezen.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.

from canto_next.hooks import on_hook
from canto_next.plugins import Plugin
from canto_next.remote import assign_to_dict, access_dict

from .command import CommandHandler, register_commands, register_arg_types, unregister_all, _string, register_aliases, commands, command_help
from .tagcore import tag_updater
from .parser import prep_for_display
from .config import needs_eval

import logging

_log = logging.getLogger("COMMON")

import subprocess
import tempfile
import urllib.request, urllib.error, urllib.parse
import shlex
import sys

import os
import os.path

class _BasePlugin(Plugin):
    pass

class _GuiBase(CommandHandler):
    def init(self):
        _args = {
            "key": ("[key]: Simple keys (a), basic chords (C-r, M-a), or named whitespace like space or tab", _string),
            "command": ("[command]: Any canto-curses command. (Will show current binding if not given)\n  Simple: goto\n  Chained: foritems \\\\& goto \\\\& item-state read \\\\& clearitems \\\\& next-item", self.type_unescape_command),
            "remote-cmd": ("[remote cmd]", self.type_remote_cmd),
            "url" : ("[URL]", _string),
            "help-command" : ("[help-command]: Any canto-curses command, if blank, 'any' or unknown, will display help overview", self.type_help_cmd),
            "config-option" : ("[config-option]: Any canto-curses option", self.type_config_option),
            "executable" : ("[executable]: A program in your PATH", self.type_executable),
        }

        _cmds = {
            "bind" : (self.cmd_bind, [ "key", "command" ], "Add or query %s keybinds" % self.get_opt_name()),
            "transform" : (self.cmd_transform, ["string"], "Set user transform"),
            "remote addfeed" : (lambda x : self.cmd_remote("addfeed", x), ["url"], "Subscribe to a feed"),
            "remote listfeeds" : (lambda : self.cmd_remote("listfeeds", ""), [], "List feeds"),
            "remote": (self.cmd_remote, ["remote-cmd", "string"], "Give a command to canto-remote"),
            "destroy": (self.cmd_destroy, [], "Destroy this %s" % self.get_opt_name()),
            "set" : (self.cmd_set, ["config-option", "string"], "Set configuration options"),
            "set browser.path" : (lambda x : self.cmd_set("browser.path", x), ["executable"], "Set desired browser"),
        }

        _help_cmds = {
            "help" : (self.cmd_help, ["help-command"], "Get help on a specific command")
        }

        _aliases = {
            "add" : "remote addfeed",
            "del" : "remote delfeed",
            "list" : "remote listfeeds",

            # Compatibility / evaluation aliases
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

        register_arg_types(self, _args)

        register_commands(self, _cmds, "Base")
        register_commands(self, _help_cmds, "Help")

        register_aliases(self, _aliases)

        self._editor = None

        self.plugin_class = _BasePlugin
        self.update_plugin_lookups()

    def cmd_destroy(self):
        self.callbacks["die"](self)

    def die(self):
        unregister_all(self)

    # Provide completions, but we don't care to verify settings.

    def type_executable(self):
        _executables = []
        for _path_dir in os.environ["PATH"].split(os.pathsep):
            for _f in os.listdir(_path_dir):
                _fullpath = os.path.join(_path_dir, _f)
                if os.path.isfile(_fullpath) and os.access(_fullpath, os.X_OK):
                    _executables.append(_f)

        return (_executables, lambda x : (True, x))

    def _fork(self, _path, _href, _text, _fetch=False):

        # Prepare temporary files, if fetch.

        if _fetch:
            # Get a path (sans query strings, etc.) for the URL
            _tmppath = urllib.parse.urlparse(_href).path

            # Return just the basename of the path (no directories)
            _fname = os.path.basename(_tmppath)

            # Grab a temporary directory. This allows us to create a file with
            # an unperturbed filename so scripts can freely use regex /
            # extension matching in addition to mimetype detection.

            _tmpdir = tempfile.mkdtemp(prefix="canto-")
            _tmpnam = _tmpdir + '/' + _fname

            on_hook("curses_exit", lambda : (os.unlink(_tmpnam)))
            on_hook("curses_exit", lambda : (os.rmdir(_tmpdir)))

        _pid = os.fork()

        # Parents can now bail.
        if _pid:
            return _pid

        if _fetch:
            _tmp = open(_tmpnam, 'w+b')

            # Grab the HTTP info / prepare to read.
            _response = urllib.request.urlopen(_href)

            # Grab in kilobyte chunks to avoid wasting memory on something
            # that's going to be immediately written to disk.

            while True:
                _r = _response.read(1024)
                if not _r:
                    break
                _tmp.write(_r)

            _response.close()
            _tmp.close()

            _href = _tmpnam

        # Make sure that we quote href such that malicious URLs like
        # "http://example.com & rm -rf ~/" won't be interpreted by the shell.

        _href = shlex.quote(_href)

        # A lot of programs don't appreciate
        # having their fds closed, so instead
        # we dup them to /dev/null.

        _fd = os.open("/dev/null", os.O_RDWR)
        os.dup2(_fd, sys.stderr.fileno())

        if not _text:
            os.setpgid(os.getpid(), os.getpid())
            os.dup2(_fd, sys.stdout.fileno())

        if "%u" in _path:
            _path = _path.replace("%u", _href)
        elif _href:
            _path = _path + " " + _href

        os.execv("/bin/sh", ["/bin/sh", "-c", _path])

        # Just in case.
        sys.exit(0)

    def _edit(self, _text):
        if not self._editor:
            self._editor = os.getenv("EDITOR")
        if not self._editor:
            self._editor = self.input("editor: ")

        # No editor, or cancelled dialog, no change.
        if not self._editor:
            return _text

        self.callbacks["pause_interface"]()

        # Setup tempfile to edit.
        _fd, _path = tempfile.mkstemp(text=True)

        _f = os.fdopen(_fd, "w")
        _f.write(_text)
        _f.close()

        # Invoke editor
        logging.info("Invoking editor on %s" % _path)
        _pid = self._fork(self._editor + " %u", _path, True)
        _pid, _status = os.waitpid(_pid, 0)

        if _status == 0:
            _f = open(_path, "r")
            _r = _f.read()
            _f.close()
        else:
            self.callbacks["set_var"]("error_msg",
                    "Editor failed! Status = %d" % (_status,))
            _r = _text

        # Cleanup temp file.
        os.unlink(_path)

        self.callbacks["unpause_interface"]()

        return _r

    def cmd_edit(self, **_kwargs):
        _t = self.callbacks["get_opt"](_kwargs["opt"])
        _r = self._edit(_t)
        _log.info("Edited %s to %s" % (_kwargs["opt"], _r))
        self.callbacks["set_opt"](_kwargs["opt"], _r)

    def type_remote_cmd(self):
        _remote_cmds = [ "help", "addfeed", "listfeeds", "delfeed",
                "force-update", "config", "one-config", "export",
                "import", "kill" ]
        return (_remote_cmds, lambda x : (x in _remote_cmds, x))

    def _remote_argv(self, _argv):
        loc_args = self.callbacks["get_var"]("location")
        _argv = [_argv[0]] + loc_args + _argv[1:]

        _log.debug("Calling remote: %s" % _argv)

        # check_output return bytes, we must decode.
        out = subprocess.check_output(_argv).decode()

        _log.debug("Output:")
        _log.debug(out.rstrip())

        # Strip anything that could be misconstrued as style
        # from remote output.

        out = out.replace("%","\\%")

        _log.info(out.rstrip())

    def _remote(self, _args):
        _args = "canto-remote " + _args

        # Add location args, so the remote is connecting
        # to the correct daemon.

        self._remote_argv(shlex.split(_args))

    def remote_args(self, _args):
        return self.string(_args, "remote: ")

    def cmd_remote(self, _remote_cmd, _args):
        self._remote("%s %s" % (_remote_cmd, _args))

    def _goto(self, _urls, _fetch=False):
        _browser = self.callbacks["get_conf"]()["browser"]

        if not _browser["path"]:
            _log.error("No browser defined! Cannot goto.")
            return

        if _browser["text"]:
            self.callbacks["pause_interface"]()

        for _url in _urls:
            _pid = self._fork(_browser["path"], _url, _browser["text"], _fetch)
            if _browser["text"]:
                os.waitpid(_pid, 0)

        if _browser["text"]:
            self.callbacks["unpause_interface"]()

    # Like goto, except download the file to /tmp before executing browser.

    def _fetch(self, _urls):
        self._goto(_urls, True)

    def cmd_transform(self, _transform):
        tag_updater.transform("user", _transform)
        tag_updater.reset(True)
        tag_updater.update()

    def type_unescape_command(self):
        def validate_uescape_command(_x):
            # Change the escaped '&' from shlex into a raw &
            return (True, _x.replace(" '&' ", " & "))
        return (None, validate_uescape_command)

    def cmd_bind(self, _key, _cmd):
        self.bind(_key, _cmd.lstrip().rstrip(), True)

    def bind(self, _key, _cmd, _overwrite=False):
        _opt = self.get_opt_name()
        _key = self.translate_key(_key)
        _c = self.callbacks["get_conf"]()
        if not _cmd:
            if _key in _c[_opt]["key"]:
                _log.info("[%s] %s = %s" % (_opt, _key, _c[_opt]["key"][_key]))
                return True
            else:
                return False
        else:
            if _key in _c[_opt]["key"] and _c[_opt]["key"][_key] and not _overwrite:
                _log.debug("%s already bound to %s" % (_key, _c[_opt]["key"][_key]))
                return False

            _log.debug("Binding %s.%s to %s" % (_opt, _key, _cmd))

            _c[_opt]["key"][_key] = _cmd
            self.callbacks["set_conf"](_c)
            return True

    def type_help_cmd(self):
        _help_cmds = commands()

        def help_validator(_x):
            if _x in ["commands", "cmds"]:
                return (True, 'commands')
            for _group in _help_cmds:
                if _x in _help_cmds[_group]:
                    return (True, _x)
            return (True, 'all')

        return (_help_cmds, help_validator)

    def cmd_help(self, _cmd):
        if self.callbacks["get_var"]("info_msg"):
            self.callbacks["set_var"]("info_msg", "")

        if _cmd == 'all':
            _log.info("%BHELP%b\n")
            _log.info("This is a list of available keybinds.\n")
            _log.info("For a list of commands, type ':help commands'\n")
            _log.info("For help with a specific command, type ':help [command]'\n")
            _log.info("%BBinds%b")

            _config = self.callbacks["get_conf"]()

            for _optname in [ "main", "taglist", "reader" ]:
                if "key" in _config[_optname] and list(_config[_optname]["key"].keys()) != []:
                    _maxbindl = max([ len(x) for x in _config[_optname]["key"].keys() ]) + 1
                    _log.info("\n%B" + _optname + "%b\n")
                    for _bind in sorted(_config[_optname]["key"]):
                        _bindeff = prep_for_display(_bind + (" " * (_maxbindl - len(_bind))))
                        _cmd = prep_for_display(_config[_optname]["key"][_bind])
                        _log.info("%s %s" % (_bindeff, _cmd))

        elif _cmd == 'commands':
            _gc = commands()
            for _group in sorted(_gc.keys()):
                _log.info("%B" + _group + "%b\n")
                for _c in sorted(_gc[_group]):
                    _log.info(command_help(_c))
                _log.info("")
        else:
            _log.info(command_help(_cmd, True))

    # Validate a single config option
    # Will offer completions for any recognized config option
    # Will *not* reject validly formatted options that don't already exist

    def _get_current_config_options(self, _obj, _stack):
        _r = []

        for _item in _obj.keys():
            _stack.append(_item)

            if type(_obj[_item]) == dict:
                _r.extend(self._get_current_config_options(_obj[_item], _stack[:]))
            else:
                _r.append(shlex.quote(".".join(_stack)))

            _stack = _stack[:-1]

        return _r

    def type_config_option(self):
        _conf = self.callbacks["get_conf"]()

        _possibles = self._get_current_config_options(_conf, [])
        _possibles.sort()

        return (_possibles, lambda x : (True, x))

    def cmd_set(self, _opt, _val):
        _log.debug("SET: %s '%s'" % (_opt, _val))

        _evaluate = needs_eval(_opt)

        if _val != "" and _evaluate:
            _log.debug("Evaluating...")
            try:
                _val = eval(_val)
            except Exception as _e:
                _log.error("Couldn't eval '%s': %s" % (_val, _e))
                return

        if _opt.startswith("defaults."):
            _conf = { "defaults" : self.callbacks["get_defaults"]() }

            if _val != "":
                assign_to_dict(_conf, _opt, _val)
                self.callbacks["set_defaults"](_conf["defaults"])
        elif _opt.startswith("feed."):
            _sel = self.callbacks["get_var"]("selected")
            if not _sel:
                _log.info("Feed settings only work with a selected item")
                return

            if _sel.is_tag:
                _try_tag = _sel
            else:
                _try_tag = _sel.parent_tag

            if not _try_tag.tag.startswith("maintag:"):
                _log.info("Selection is in a user tag, cannot set feed settings")
                return

            _name = _try_tag.tag[8:]

            _conf = { "feed" : self.callbacks["get_feed_conf"](_name) }

            if _val != "":
                assign_to_dict(_conf, _opt, _val)
                self.callbacks["set_feed_conf"](_name, _conf["feed"])
        elif _opt.startswith("tag."):
            _sel = self.callbacks["get_var"]("selected")
            if not _sel:
                _log.info("Tag settings only work with a selected item")
                return

            if _sel.is_tag:
                _tag = _sel
            else:
                _tag = _sel.parent_tag

            _conf = { "tag" : self.callbacks["get_tag_conf"](_tag.tag) }

            if _val != "":
                assign_to_dict(_conf, _opt, _val)
                self.callbacks["set_tag_conf"](_tag.tag, _conf["tag"])
        else:
            _conf = self.callbacks["get_conf"]()

            if _val != "":
                assign_to_dict(_conf, _opt, _val)
                self.callbacks["set_conf"](_conf)

        _ok, _val = access_dict(_conf, _opt)
        if not _ok:
            _log.error("Unknown option %s" % _opt)
            _log.error("Full conf: %s" % _conf)
        else:
            _log.info("%s = %s" % (_opt, _val))