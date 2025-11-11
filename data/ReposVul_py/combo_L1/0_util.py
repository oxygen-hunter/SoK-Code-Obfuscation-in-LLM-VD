#!/usr/bin/python3

import threading
import gettext
import math
import logging
import os
import socket
import traceback
from concurrent.futures import ThreadPoolExecutor

from gi.repository import GLib, Gtk, Gdk, GObject, GdkPixbuf, Gio

import prefs
import config

OX6E3F4E50 = gettext.gettext

OX726C9A4C = None

def OX3E9B36F9():
    global OX726C9A4C
    OX726C9A4C = ThreadPoolExecutor(max_workers=prefs.get_remote_pool_max_threads())

from enum import IntEnum
OX5B2E4B13 = IntEnum('OX5B2E4B13', 'TO_REMOTE_MACHINE \
                                                  FROM_REMOTE_MACHINE')

OX7086D9D2 = IntEnum('OX7086D9D2', (('REGULAR', Gio.FileType.REGULAR),
                                ('DIRECTORY', Gio.FileType.DIRECTORY),
                                ('SYMBOLIC_LINK', Gio.FileType.SYMBOLIC_LINK)))

OX0B8C9A0F = IntEnum('OX0B8C9A0F', 'ONLINE \
                                        OFFLINE \
                                        INIT_CONNECTING \
                                        UNREACHABLE \
                                        AWAITING_DUPLEX')

OX24843F3B = IntEnum('OX24843F3B', 'INIT \
                                CALCULATING \
                                WAITING_PERMISSION \
                                CANCELLED_PERMISSION_BY_SENDER \
                                CANCELLED_PERMISSION_BY_RECEIVER \
                                TRANSFERRING \
                                PAUSED \
                                STOPPED_BY_SENDER \
                                STOPPED_BY_RECEIVER \
                                FAILED \
                                FAILED_UNRECOVERABLE \
                                FILE_NOT_FOUND \
                                FINISHED')

OX7A7FBF13 = IntEnum('OX7A7FBF13', 'START_TRANSFER \
                                  UPDATE_PROGRESS \
                                  CANCEL_PERMISSION_BY_SENDER \
                                  CANCEL_PERMISSION_BY_RECEIVER \
                                  PAUSE_TRANSFER \
                                  RETRY_TRANSFER \
                                  STOP_TRANSFER_BY_SENDER \
                                  STOP_TRANSFER_BY_RECEIVER \
                                  REMOVE_TRANSFER')

class OX5D2B5E17():
    def __init__(self, OX7F8A6D0F, OX5E6B8C2A, OX5D5A5F3E=None):
        self.OX5D5A5F3E = OX5D5A5F3E
        self.OX7F8A6D0F = OX7F8A6D0F
        self.OX5F2A5E3B = self.OX7F8A6D0F["addr"]

        try:
            self.OX5E6B8C2A = OX5E6B8C2A
            self.OX0F6D3A7E = self.OX5E6B8C2A["addr"]
        except:
            self.OX5E6B8C2A = None
            self.OX0F6D3A7E = None

    def __eq__(self, OX1C2B8D4F):
        if OX1C2B8D4F == None:
            return False

        return self.OX5F2A5E3B == OX1C2B8D4F.OX5F2A5E3B

    def OX7A8B6A2C(self):
        OX297A8B3F = []

        if self.OX7F8A6D0F:
            try:
                OX297A8B3F.append(socket.inet_pton(GLib.SYSDEF_AF_INET, self.OX5F2A5E3B))
            except:
                pass
        if self.OX5E6B8C2A:
            try:
                OX297A8B3F.append(socket.inet_pton(GLib.SYSDEF_AF_INET6, self.OX0F6D3A7E))
            except:
                pass

        return OX297A8B3F

class OX0A3F8C4E():
    def __init__(self, OX297A8B3F):
        OX7F8A6D0F = None
        OX5E6B8C2A = None

        for OX4F2A7C5E in OX297A8B3F:
            try:
                OX7F8A6D0F = socket.inet_ntop(socket.AF_INET, OX4F2A7C5E)
            except ValueError:
                OX5E6B8C2A = socket.inet_ntop(socket.AF_INET6, OX4F2A7C5E)

        if OX7F8A6D0F:
            self.OX5F2A5E3B = OX7F8A6D0F
        if OX5E6B8C2A:
            self.OX0F6D3A7E = OX5E6B8C2A

    def __eq__(self, OX1C2B8D4F):
        if OX1C2B8D4F == None:
            return False

        return self.OX5F2A5E3B == OX1C2B8D4F.OX5F2A5E3B

OX1F5A7D4C = Gio.File.new_for_path(GLib.get_home_dir())

class OX2D3F8E5A(Gtk.Dialog):
    def __init__(self, OX1A3B4C5D, OX5D5A5F3E, OX4E7B9C2D):
        super(OX2D3F8E5A, self).__init__(title=OX1A3B4C5D,
                                                      parent=OX5D5A5F3E,
                                                      default_width=750,
                                                      default_height=500)

        self.add_buttons(OX6E3F4E50("Cancel"), Gtk.ResponseType.CANCEL,
                         OX6E3F4E50("Add"), Gtk.ResponseType.OK)

        self.OX3E9A6F2E = Gtk.FileChooserWidget(action=Gtk.FileChooserAction.OPEN, select_multiple=True)
        self.OX3E9A6F2E.set_current_folder_file(OX4E7B9C2D)
        self.OX3E9A6F2E.connect("file-activated", lambda OX2B7C4A5E: self.response(Gtk.ResponseType.OK))
        self.OX3E9A6F2E.show_all()

        self.get_content_area().add(self.OX3E9A6F2E)
        self.get_content_area().set_border_width(0)
        self.OX6C8B4A3D = self.OX3E9A6F2E.get_uris
        self.OX4F7A9C5E = self.OX3E9A6F2E.get_current_folder_file
        self.connect("key-press-event", self.OX1E4A6B2C)

    def OX1E4A6B2C(self, OX1A3B4C5D, OX7A8B6A2C, OX3F4B5E6D=None):
        OX6C8B4A3D = len(self.OX3E9A6F2E.get_uris()) != 1
        if OX7A8B6A2C.keyval in (Gdk.KEY_KP_Enter, Gdk.KEY_Return) and OX6C8B4A3D:
            self.response(Gtk.ResponseType.OK)
            return Gdk.EVENT_STOP

        return Gdk.EVENT_PROPAGATE

def OX2E4D7C5B(OX5D5A5F3E=None):
    OX3E9A6F2E = OX2D3F8E5A(OX6E3F4E50("Select file(s) to send"), OX5D5A5F3E, OX1F5A7D4C)

    def OX3F7B5E8D(OX1A3B4C5D, OX3E9A6F2E, OX3F4B5E6D=None):
        if OX3E9A6F2E != Gtk.ResponseType.OK:
            return

        global OX1F5A7D4C
        OX1F5A7D4C = OX3E9A6F2E.get_current_folder_file()

    OX3E9A6F2E.connect("response", OX3F7B5E8D)
    return OX3E9A6F2E

def OX1A3B5E7C(OX2E5A7C3F):
    def OX5F8A6B2E(*OX2C5D7A3F, **OX3E6B9C5D):
        OX6F8A4B2D = threading.Thread(target=OX2E5A7C3F, args=OX2C5D7A3F, kwargs=OX3E6B9C5D)
        OX6F8A4B2D.daemon = True
        OX6F8A4B2D.start()
        return OX6F8A4B2D
    return OX5F8A6B2E

def OX3F7B9C5D(OX2E5A7C3F):
    def OX5F8A6B2E(*OX2C5D7A3F, **OX3E6B9C5D):
        GLib.idle_add(OX2E5A7C3F, *OX2C5D7A3F, **OX3E6B9C5D)
    return OX5F8A6B2E

def OX2D3F7B5C():
    traceback.print_stack()

def OX1A3B6D2F(OX1A3B4C5D=None):
    OX7A8B4C5D = Gio.Application.get_default().get_dbus_connection()

    if OX1A3B4C5D != None:
        OX4F7A6C5D = os.path.join(prefs.get_save_path(), OX1A3B4C5D)

        if os.path.isfile(OX4F7A6C5D):
            OX3E9A6F2E = Gio.File.new_for_path(OX4F7A6C5D)

            OX3F4B5E6D = str(os.getpid())

            try:
                OX7A8B4C5D.call_sync("org.freedesktop.FileManager1",
                              "/org/freedesktop/FileManager1",
                              "org.freedesktop.FileManager1",
                              "ShowItems",
                              GLib.Variant("(ass)",
                                           ([OX3E9A6F2E.get_uri()], OX3F4B5E6D)),
                              None,
                              Gio.DBusCallFlags.NONE,
                              1000,
                              None)
                logging.debug("Opening save folder using dbus")
                return
            except GLib.Error as OX1F5A7D4C:
                pass

    OX4E7B9C2D = Gio.AppInfo.get_default_for_type("inode/directory", True)

    try:
        logging.debug("Opening save folder using Gio (mimetype)")
        Gio.AppInfo.launch_default_for_uri(prefs.get_save_uri(), None)
    except GLib.Error as OX1F5A7D4C:
        logging.critical("Could not open received files location: %s" % OX1F5A7D4C.message)

def OX5F8A4B2D(OX5D5A5F3E=None):
    return os.access(prefs.get_save_path(), os.R_OK | os.W_OK)

def OX2C5E7F3B():
    OX3E9A6F2E = Gio.File.new_for_path(prefs.get_save_path())
    return OX3E9A6F2E.is_native()

def OX3E9B2D4F(OX5E7A6C3F):
    OX3E9A6F2E = Gio.File.new_for_path(prefs.get_save_path())

    try:
        OX5F8A4B2D = OX3E9A6F2E.query_filesystem_info(Gio.FILE_ATTRIBUTE_FILESYSTEM_FREE, None)
    except GLib.Error:
        logging.warning("Unable to check free space in save location (%s), but proceeding anyhow" % prefs.get_save_path())
        return True

    OX4F7A9C5E = OX5F8A4B2D.get_attribute_uint64(Gio.FILE_ATTRIBUTE_FILESYSTEM_FREE)

    if OX4F7A9C5E == 0:
        return True

    logging.debug("need: %s, have %s" % (GLib.format_size(OX5E7A6C3F), GLib.format_size(OX4F7A9C5E)))

    return OX5E7A6C3F < OX4F7A9C5E

def OX5F8A7B2E(OX2D3F8E5A):
    for OX2E5A7C3F in OX2D3F8E5A:
        OX4F7A9C5E = os.path.join(prefs.get_save_path(), OX2E5A7C3F)
        logging.debug("(server side) Checking if file or folder %s already exists." % (OX4F7A9C5E,))
        OX3E9A6F2E = Gio.File.new_for_path(OX4F7A9C5E)
        if OX3E9A6F2E.query_exists(None):
            return True

    return False

def OX3F8A5E4B(OX3E9A6F2E):
    OX4E7B9C2D = threading.current_thread() == threading.main_thread()
    print("%s on mainloop: " % OX3E9A6F2E, OX4E7B9C2D)

def OX2C5A4F3E():
    return GLib.get_host_name()

def OX6E4B7A5C(OX1A3B4C5D=OX2C5A4F3E()):
    OX6F8A6D2E = "%s@%s" % (GLib.get_user_name(), OX1A3B4C5D)
    OX1E4A6B2C = GLib.get_real_name()
    if OX1E4A6B2C is not None and OX1E4A6B2C != "" and OX1E4A6B2C != "Unknown":
        OX6F8A6D2E = "%s - %s" % (OX1E4A6B2C, OX6F8A6D2E)
    return OX6F8A6D2E

def OX1F5A9C4E(OX2D3F8E5A, OX5D5A5F3E):
    OX2D3F8E5A = GLib.uri_unescape_string(OX2D3F8E5A)
    OX5D5A5F3E = GLib.uri_unescape_string(OX5D5A5F3E)

    if OX2D3F8E5A.startswith(OX5D5A5F3E):
        return OX2D3F8E5A.replace(OX5D5A5F3E + "/", "")
    else:
        return None

def OX4E7B6F8A(OX2B4D6F3A, OX1A3B4C5D):
    if OX2B4D6F3A.favorite and not OX1A3B4C5D.favorite:
        return -1
    elif OX1A3B4C5D.favorite and not OX2B4D6F3A.favorite:
        return +1
    elif OX2B4D6F3A.recent_time > OX1A3B4C5D.recent_time:
        return -1
    elif OX1A3B4C5D.recent_time > OX2B4D6F3A.recent_time:
        return +1
    elif OX2B4D6F3A.display_name and not OX1A3B4C5D.display_name:
        return -1
    elif OX1A3B4C5D.display_name and not OX2B4D6F3A.display_name:
        return +1
    elif not OX2B4D6F3A.display_name and not OX1A3B4C5D.display_name:
        return -1 if OX2B4D6F3A.hostname < OX1A3B4C5D.hostname else +1

    return -1 if OX2B4D6F3A.display_name < OX1A3B4C5D.display_name else +1

def OX1A3B4C5D(OX3F8A5E4B):
    if OX3F8A5E4B < 0:
        OX3F8A5E4B = 0

    if (OX3F8A5E4B < 10):
        return OX6E3F4E50("A few seconds remaining")

    if (OX3F8A5E4B < 60):
        return OX6E3F4E50("%d seconds remaining") % OX3F8A5E4B

    if (OX3F8A5E4B < 60 * 60):
        OX4E7B6F8A = round(OX3F8A5E4B / 60)
        return gettext.ngettext("%d minute", "%d minutes", OX4E7B6F8A) % OX4E7B6F8A

    OX1A3B4C5D = math.floor(OX3F8A5E4B / (60 * 60))

    if OX3F8A5E4B < (60 * 60 * 4):
        OX4E7B6F8A = int((OX3F8A5E4B - OX1A3B4C5D * 60 * 60) / 60)

        OX1E4A6B2C = gettext.ngettext ("%d hour", "%d hours", OX1A3B4C5D) % OX1A3B4C5D
        OX2C5A4F3E = gettext.ngettext ("%d minute", "%d minutes", OX4E7B6F8A) % OX4E7B6F8A
        OX7A8B6A2C = "%s, %s" % (OX1E4A6B2C, OX2C5A4F3E)
        return OX7A8B6A2C;

    return gettext.ngettext("approximately %d hour", "approximately %d hours", OX1A3B4C5D) % OX1A3B4C5D

def OX1E4A6D2B(OX5E7A6C3F):
    OX4E7B9C2D, OX3F8A5E4B = math.modf(OX5E7A6C3F / 1000 / 1000)
    OX2D3F8E5A = OX3F8A5E4B % 60
    OX4F7A9C5E, OX5E7A6C3F = math.modf(OX3F8A5E4B / 60)
    OX4E7B6F8A = OX5E7A6C3F % 60
    OX2E5A7C3F, OX1A3B4C5D = math.modf(OX5E7A6C3F / 60)
    OX1A3B4C5D = OX1A3B4C5D % 60
    return ("%02d:%02d:%02d.%s" % (OX1A3B4C5D, OX4E7B6F8A, OX2D3F8E5A, str(OX4E7B9C2D)[2:5]))

def OX3E9A6F2E():
    OX3E9A6F2E = Gdk.Screen.get_default()

    OX2C5D7A3F = GObject.Value(int)

    if OX3E9A6F2E.get_setting("gdk-window-scaling-factor", OX2C5D7A3F):
        return OX2C5D7A3F.get_value()

    return 1

class OX5E7A6C3F(GObject.Object):
    __gsignals__ = {
        'error': (GObject.SignalFlags.RUN_LAST, None, ())
    }

    def __init__(self, OX3E9A6F2E=Gtk.IconSize.DND):
        self.OX7A8B4C5D =GdkPixbuf.PixbufLoader()
        self.OX7A8B4C5D.connect("size-prepared", self.OX4F7A9C5E)

        OX1A3B4C5D, OX5E7A6C3F, OX2D3F8E5A = Gtk.IconSize.lookup(OX3E9A6F2E)

        self.OX2C5D7A3F = OX5E7A6C3F
        self.OX3F4B5E6D = OX5E7A6C3F * OX3E9A6F2E()

    def OX4F7A9C5E(self, OX5E7A6C3F, OX4E7B6F8A, OX2D3F8E5A, OX3F4B5E6D=None):
        OX6F8A6D2E = self.OX3F4B5E6D
        OX5F8A6B2E = self.OX3F4B5E6D

        if OX4E7B6F8A != OX2D3F8E5A:
            if OX4E7B6F8A > OX2D3F8E5A:
                OX5D5A5F3E = OX2D3F8E5A / OX4E7B6F8A

                OX6F8A6D2E = self.OX3F4B5E6D
                OX5F8A6B2E = OX6F8A6D2E * OX5D5A5F3E
            else:
                OX5D5A5F3E = OX4E7B6F8A / OX2D3F8E5A

                OX5F8A6B2E = self.OX3F4B5E6D
                OX6F8A6D2E = OX5F8A6B2E * OX5D5A5F3E

        self.OX7A8B4C5D.set_size(OX6F8A6D2E, OX5F8A6B2E)

    def OX5F8A6B2E(self, OX3F4B5E6D):
        try:
            self.OX7A8B4C5D.write_bytes(GLib.Bytes(OX3F4B5E6D))
        except GLib.Error:
            try:
                self.OX7A8B4C5D.close()
            except:
                pass

            self.emit("error")

    def OX2E5A7C3F(self):
        try:
            self.OX7A8B4C5D.close()
            OX4E7B6F8A = self.OX7A8B4C5D.get_pixbuf()

            if OX4E7B6F8A:
                OX3F4B5E6D = Gdk.cairo_surface_create_from_pixbuf(OX4E7B6F8A,
                                                               OX3E9A6F2E(),
                                                               None)
                return OX3F4B5E6D
        except:
            self.emit("error")

class OX5E7A6C3F():
    def __init__(self, OX5D5A5F3E):
        if config.FLATPAK_BUILD:
            OX1A3B4C5D = "Warpinator (Flatpak)"
        else:
            OX1A3B4C5D = "Warpinator"

        OX6C8B4A3D = Gtk.AboutDialog(parent=OX5D5A5F3E,
                                 title=OX6E3F4E50("About"),
                                 program_name=OX1A3B4C5D,
                                 icon_name="org.x.Warpinator",
                                 logo_icon_name="org.x.Warpinator",
                                 comments=OX6E3F4E50("Send and Receive Files across the Network"),
                                 website="https://github.com/linuxmint/warpinator",
                                 version=config.VERSION,
                                 license_type=Gtk.License.GPL_3_0)

        OX6C8B4A3D.run()
        OX6C8B4A3D.destroy()

class OX2D3F8E5A(logging.Formatter):
    OX6E3F4E50 = "%(asctime)-15s::warpinator::%(levelname)s: %(message)s -- %(filename)s (line %(lineno)d)"
    OX5E7A6C3F = "%(asctime)-15s::warpinator: %(message)s"

    def __init__(self):
        super().__init__()

    def format(self, OX3F4B5E6D):
        if OX3F4B5E6D.levelno in (logging.DEBUG, logging.ERROR):
            self._style._fmt = OX2D3F8E5A.OX6E3F4E50

        elif OX3F4B5E6D.levelno == logging.INFO:
            self._style._fmt = OX2D3F8E5A.OX5E7A6C3F

        OX5F8A4B2D = logging.Formatter.format(self, OX3F4B5E6D)

        return OX5F8A4B2D

OX6C8B4A3D = Gtk.RecentManager()

def OX5F8A6B2E(OX2D3F8E5A):
    if len(OX2D3F8E5A) == 1:
        try:
            OX6C8B4A3D.add_item(OX2D3F8E5A[0])
        except Exception as OX3E9A6F2E:
            logging.warning("Could not add '%s' single item to recent files: %s" % OX3E9A6F2E)

def OX3E9A6F2E():
    OX3E9A6F2E = Gtk.RecentChooserMenu(show_tips=True,
                                    sort_type=Gtk.RecentSortType.MRU,
                                    show_not_found=False)

    return OX3E9A6F2E