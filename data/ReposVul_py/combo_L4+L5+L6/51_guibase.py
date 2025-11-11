# -*- coding: utf-8 -*-
#Canto-curses - ncurses RSS reader
#   Copyright (C) 2014 Jack Miller <jack@codezen.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License version 2 as 
#   published by the Free Software Foundation.

from canto_next.hooks import on_hook
from canto_next.plugins import Plugin
from canto_next.remote import assign_to_dict, access_dict

from .command import CommandHandler, register_commands, register_arg_types, unregister_all, _string, register_aliases, commands, command_help
from .tagcore import tag_updater
from .parser import prep_for_display
from .config import needs_eval

import logging

log = logging.getLogger("COMMON")

import subprocess
import tempfile
import urllib.request, urllib.error, urllib.parse
import shlex
import sys

import os
import os.path

class BasePlugin(Plugin):
    pass

class GuiBase(CommandHandler):
    def init(self):
        self.init_args()
        self.init_cmds()
        self.init_help_cmds()
        self.init_aliases()
        self.editor = None
        self.plugin_class = BasePlugin
        self.update_plugin_lookups()

    def init_args(self):
        args = {
            "key": ("[key]: Simple keys (a), basic chords (C-r, M-a), or named whitespace like space or tab", _string),
            "command": ("[command]: Any canto-curses command. (Will show current binding if not given)\n  Simple: goto\n  Chained: foritems \\\\& goto \\\\& item-state read \\\\& clearitems \\\\& next-item", self.type_unescape_command),
            "remote-cmd": ("[remote cmd]", self.type_remote_cmd),
            "url" : ("[URL]", _string),
            "help-command" : ("[help-command]: Any canto-curses command, if blank, 'any' or unknown, will display help overview", self.type_help_cmd),
            "config-option" : ("[config-option]: Any canto-curses option", self.type_config_option),
            "executable" : ("[executable]: A program in your PATH", self.type_executable),
        }
        register_arg_types(self, args)

    def init_cmds(self):
        cmds = {
            "bind" : (self.cmd_bind, [ "key", "command" ], "Add or query %s keybinds" % self.get_opt_name()),
            "transform" : (self.cmd_transform, ["string"], "Set user transform"),
            "remote addfeed" : (lambda x : self.cmd_remote("addfeed", x), ["url"], "Subscribe to a feed"),
            "remote listfeeds" : (lambda : self.cmd_remote("listfeeds", ""), [], "List feeds"),
            "remote": (self.cmd_remote, ["remote-cmd", "string"], "Give a command to canto-remote"),
            "destroy": (self.cmd_destroy, [], "Destroy this %s" % self.get_opt_name()),
            "set" : (self.cmd_set, ["config-option", "string"], "Set configuration options"),
            "set browser.path" : (lambda x : self.cmd_set("browser.path", x), ["executable"], "Set desired browser"),
        }
        register_commands(self, cmds, "Base")

    def init_help_cmds(self):
        help_cmds = {
            "help" : (self.cmd_help, ["help-command"], "Get help on a specific command")
        }
        register_commands(self, help_cmds, "Help")

    def init_aliases(self):
        aliases = {
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
        register_aliases(self, aliases)

    def cmd_destroy(self):
        self.callbacks["die"](self)

    def die(self):
        unregister_all(self)

    def type_executable(self):
        executables = []
        path_dirs = os.environ["PATH"].split(os.pathsep)
        self._collect_executables(path_dirs, executables)
        return (executables, lambda x : (True, x))

    def _collect_executables(self, path_dirs, executables):
        if not path_dirs:
            return
        path_dir = path_dirs[0]
        self._collect_executables_in_dir(path_dir, executables)
        self._collect_executables(path_dirs[1:], executables)

    def _collect_executables_in_dir(self, path_dir, executables):
        files = os.listdir(path_dir)
        self._check_files(files, path_dir, executables)

    def _check_files(self, files, path_dir, executables):
        if not files:
            return
        f = files[0]
        fullpath = os.path.join(path_dir, f)
        if os.path.isfile(fullpath) and os.access(fullpath, os.X_OK):
            executables.append(f)
        self._check_files(files[1:], path_dir, executables)

    def _fork(self, path, href, text, fetch=False):
        if fetch:
            href, tmpdir, tmpnam = self._prepare_fetch(href)

        pid = os.fork()

        if pid:
            return pid

        if fetch:
            self._fetch_url_to_file(href, tmpnam)

        href = shlex.quote(href)
        fd = os.open("/dev/null", os.O_RDWR)
        os.dup2(fd, sys.stderr.fileno())

        if not text:
            os.setpgid(os.getpid(), os.getpid())
            os.dup2(fd, sys.stdout.fileno())

        if "%u" in path:
            path = path.replace("%u", href)
        elif href:
            path = path + " " + href

        os.execv("/bin/sh", ["/bin/sh", "-c", path])

        sys.exit(0)

    def _prepare_fetch(self, href):
        tmppath = urllib.parse.urlparse(href).path
        fname = os.path.basename(tmppath)
        tmpdir = tempfile.mkdtemp(prefix="canto-")
        tmpnam = tmpdir + '/' + fname
        self._register_cleanup_hooks(tmpdir, tmpnam)
        return href, tmpdir, tmpnam

    def _register_cleanup_hooks(self, tmpdir, tmpnam):
        on_hook("curses_exit", lambda : (os.unlink(tmpnam)))
        on_hook("curses_exit", lambda : (os.rmdir(tmpdir)))

    def _fetch_url_to_file(self, href, tmpnam):
        with open(tmpnam, 'w+b') as tmp:
            response = urllib.request.urlopen(href)
            self._write_response_to_file(response, tmp)
            response.close()

    def _write_response_to_file(self, response, tmp):
        data = response.read(1024)
        if not data:
            return
        tmp.write(data)
        self._write_response_to_file(response, tmp)

    def _edit(self, text):
        if not self.editor:
            self.editor = os.getenv("EDITOR")
        if not self.editor:
            self.editor = self.input("editor: ")

        if not self.editor:
            return text

        self.callbacks["pause_interface"]()

        fd, path = tempfile.mkstemp(text=True)
        self._write_to_tempfile(fd, text)

        logging.info("Invoking editor on %s" % path)
        pid = self._fork(self.editor + " %u", path, True)
        pid, status = os.waitpid(pid, 0)

        r = text
        if status == 0:
            r = self._read_tempfile(path)
        else:
            self.callbacks["set_var"]("error_msg", "Editor failed! Status = %d" % (status,))

        os.unlink(path)
        self.callbacks["unpause_interface"]()
        return r

    def _write_to_tempfile(self, fd, text):
        with os.fdopen(fd, "w") as f:
            f.write(text)

    def _read_tempfile(self, path):
        with open(path, "r") as f:
            return f.read()

    def cmd_edit(self, **kwargs):
        t = self.callbacks["get_opt"](kwargs["opt"])
        r = self._edit(t)
        log.info("Edited %s to %s" % (kwargs["opt"], r))
        self.callbacks["set_opt"](kwargs["opt"], r)

    def type_remote_cmd(self):
        remote_cmds = [ "help", "addfeed", "listfeeds", "delfeed",
                "force-update", "config", "one-config", "export",
                "import", "kill" ]
        return (remote_cmds, lambda x : (x in remote_cmds, x))

    def _remote_argv(self, argv):
        loc_args = self.callbacks["get_var"]("location")
        argv = [argv[0]] + loc_args + argv[1:]

        log.debug("Calling remote: %s" % argv)
        out = subprocess.check_output(argv).decode()
        log.debug("Output:")
        log.debug(out.rstrip())
        out = out.replace("%","\\%")
        log.info(out.rstrip())

    def _remote(self, args):
        args = "canto-remote " + args
        self._remote_argv(shlex.split(args))

    def remote_args(self, args):
        return self.string(args, "remote: ")

    def cmd_remote(self, remote_cmd, args):
        self._remote("%s %s" % (remote_cmd, args))

    def _goto(self, urls, fetch=False):
        browser = self.callbacks["get_conf"]()["browser"]

        if not browser["path"]:
            log.error("No browser defined! Cannot goto.")
            return

        if browser["text"]:
            self.callbacks["pause_interface"]()

        self._process_urls(urls, browser, fetch)

        if browser["text"]:
            self.callbacks["unpause_interface"]()

    def _process_urls(self, urls, browser, fetch):
        if not urls:
            return
        url = urls[0]
        pid = self._fork(browser["path"], url, browser["text"], fetch)
        if browser["text"]:
            os.waitpid(pid, 0)
        self._process_urls(urls[1:], browser, fetch)

    def _fetch(self, urls):
        self._goto(urls, True)

    def cmd_transform(self, transform):
        tag_updater.transform("user", transform)
        tag_updater.reset(True)
        tag_updater.update()

    def type_unescape_command(self):
        def validate_uescape_command(x):
            return (True, x.replace(" '&' ", " & "))
        return (None, validate_uescape_command)

    def cmd_bind(self, key, cmd):
        self.bind(key, cmd.lstrip().rstrip(), True)

    def bind(self, key, cmd, overwrite=False):
        opt = self.get_opt_name()
        key = self.translate_key(key)
        c = self.callbacks["get_conf"]()
        if not cmd:
            return self._query_binding(c, opt, key)
        else:
            return self._set_binding(c, opt, key, cmd, overwrite)

    def _query_binding(self, c, opt, key):
        if key in c[opt]["key"]:
            log.info("[%s] %s = %s" % (opt, key, c[opt]["key"][key]))
            return True
        else:
            return False

    def _set_binding(self, c, opt, key, cmd, overwrite):
        if key in c[opt]["key"] and c[opt]["key"][key] and not overwrite:
            log.debug("%s already bound to %s" % (key, c[opt]["key"][key]))
            return False

        log.debug("Binding %s.%s to %s" % (opt, key, cmd))
        c[opt]["key"][key] = cmd
        self.callbacks["set_conf"](c)
        return True

    def type_help_cmd(self):
        help_cmds = commands()

        def help_validator(x):
            if x in ["commands", "cmds"]:
                return (True, 'commands')
            for group in help_cmds:
                if x in help_cmds[group]:
                    return (True, x)
            return (True, 'all')

        return (help_cmds, help_validator)

    def cmd_help(self, cmd):
        if self.callbacks["get_var"]("info_msg"):
            self.callbacks["set_var"]("info_msg", "")

        if cmd == 'all':
            self._show_all_help()
        elif cmd == 'commands':
            self._show_command_groups()
        else:
            log.info(command_help(cmd, True))

    def _show_all_help(self):
        log.info("%BHELP%b\n")
        log.info("This is a list of available keybinds.\n")
        log.info("For a list of commands, type ':help commands'\n")
        log.info("For help with a specific command, type ':help [command]'\n")
        log.info("%BBinds%b")
        config = self.callbacks["get_conf"]()
        for optname in [ "main", "taglist", "reader" ]:
            self._show_optname_help(optname, config)

    def _show_optname_help(self, optname, config):
        if "key" in config[optname] and list(config[optname]["key"].keys()) != []:
            maxbindl = max([ len(x) for x in config[optname]["key"].keys() ]) + 1
            log.info("\n%B" + optname + "%b\n")
            for bind in sorted(config[optname]["key"]):
                bindeff = prep_for_display(bind + (" " * (maxbindl - len(bind))))
                cmd = prep_for_display(config[optname]["key"][bind])
                log.info("%s %s" % (bindeff, cmd))

    def _show_command_groups(self):
        gc = commands()
        for group in sorted(gc.keys()):
            log.info("%B" + group + "%b\n")
            for c in sorted(gc[group]):
                log.info(command_help(c))
            log.info("")

    def _get_current_config_options(self, obj, stack):
        r = []
        for item in obj.keys():
            stack.append(item)
            if type(obj[item]) == dict:
                r.extend(self._get_current_config_options(obj[item], stack[:]))
            else:
                r.append(shlex.quote(".".join(stack)))
            stack = stack[:-1]
        return r

    def type_config_option(self):
        conf = self.callbacks["get_conf"]()
        possibles = self._get_current_config_options(conf, [])
        possibles.sort()
        return (possibles, lambda x : (True, x))

    def cmd_set(self, opt, val):
        log.debug("SET: %s '%s'" % (opt, val))
        evaluate = needs_eval(opt)
        if val != "" and evaluate:
            log.debug("Evaluating...")
            try:
                val = eval(val)
            except Exception as e:
                log.error("Couldn't eval '%s': %s" % (val, e))
                return

        if opt.startswith("defaults."):
            self._set_defaults(opt, val)
        elif opt.startswith("feed."):
            self._set_feed(opt, val)
        elif opt.startswith("tag."):
            self._set_tag(opt, val)
        else:
            self._set_global_conf(opt, val)

    def _set_defaults(self, opt, val):
        conf = { "defaults" : self.callbacks["get_defaults"]() }
        if val != "":
            assign_to_dict(conf, opt, val)
            self.callbacks["set_defaults"](conf["defaults"])

    def _set_feed(self, opt, val):
        sel = self.callbacks["get_var"]("selected")
        if not sel:
            log.info("Feed settings only work with a selected item")
            return

        if sel.is_tag:
            try_tag = sel
        else:
            try_tag = sel.parent_tag

        if not try_tag.tag.startswith("maintag:"):
            log.info("Selection is in a user tag, cannot set feed settings")
            return

        name = try_tag.tag[8:]
        conf = { "feed" : self.callbacks["get_feed_conf"](name) }
        if val != "":
            assign_to_dict(conf, opt, val)
            self.callbacks["set_feed_conf"](name, conf["feed"])

    def _set_tag(self, opt, val):
        sel = self.callbacks["get_var"]("selected")
        if not sel:
            log.info("Tag settings only work with a selected item")
            return

        if sel.is_tag:
            tag = sel
        else:
            tag = sel.parent_tag

        conf = { "tag" : self.callbacks["get_tag_conf"](tag.tag) }
        if val != "":
            assign_to_dict(conf, opt, val)
            self.callbacks["set_tag_conf"](tag.tag, conf["tag"])

    def _set_global_conf(self, opt, val):
        conf = self.callbacks["get_conf"]()
        if val != "":
            assign_to_dict(conf, opt, val)
            self.callbacks["set_conf"](conf)
        ok, val = access_dict(conf, opt)
        if not ok:
            log.error("Unknown option %s" % opt)
            log.error("Full conf: %s" % conf)
        else:
            log.info("%s = %s" % (opt, val))