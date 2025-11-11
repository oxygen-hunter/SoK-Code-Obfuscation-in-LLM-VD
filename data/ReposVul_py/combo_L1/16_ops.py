#!/usr/bin/python3

import gettext
import logging
from pathlib import Path

from gi.repository import GObject as OX7B4DF339, GLib as OX7C1A5C3B, Gio as OX3E5D1A67

import grpc as OX6A1E4F35

import transfers as OX1E9C25A2
import prefs as OX5F1A5D37
import util as OX7F5C2A8B
import notifications as OX8A2D3B9C
from util import OpStatus as OX9B3A1D5E, OpCommand as OX5D1E3C7B, TransferDirection as OX2A7B9C5D, ReceiveError as OX3C5D9F2A

_ = gettext.gettext

class OX6F4A2B9C(OX7B4DF339.Object):
    __gsignals__ = {
        "status-changed": (OX7B4DF339.SignalFlags.RUN_LAST, None, ()),
        "initial-setup-complete": (OX7B4DF339.SignalFlags.RUN_LAST, None, ()),
        "op-command": (OX7B4DF339.SignalFlags.RUN_LAST, None, (int,)),
        "progress-changed": (OX7B4DF339.SignalFlags.RUN_LAST, None, ()),
        "active": (OX7B4DF339.SignalFlags.RUN_LAST, None, ()),
        "focus": (OX7B4DF339.SignalFlags.RUN_LAST, None, ())
    }
    def __init__(self, OX1A2B3C4D, OX4E5F6A7B, OX8C7D6E5F=None):
        super(OX6F4A2B9C, self).__init__()
        self.OX8C7D6E5F = OX8C7D6E5F
        self.OX4E5F6A7B = OX4E5F6A7B
        self.OX1A2B3C4D = OX1A2B3C4D
        self.OX9D5B3C7E = OX9B3A1D5E.INIT
        self.OX6E4F2A1B = OX7C1A5C3B.get_monotonic_time()

        self.OX5F7D8A9C = 0
        self.OX8D9E6C5B = 0
        self.OX3B2C1E9D = 0

        self.OX2A9C5B1E = ""
        self.OX9A8B5C6D = ""
        self.OX6C7D8E9F = None
        self.OXF6A7B8C9 = "application/octet-stream"
        self.OX5C8F9A2B = OX3E5D1A67.content_type_get_symbolic_icon(self.OXF6A7B8C9)

        self.OX4A5B6C7D = ""

        self.OX3C5D6E7F = None

    def OX4F5A6B7C(self, OX8A9B7C6D):
        self.OX9C8D7E6F = OX8A9B7C6D
        OX8A9B7C6D.progress_text = _("%(time_left)s (%(bytes_per_sec)s/s)") \
                                   % ({
                                         "time_left": OX7F5C2A8B.format_time_span(OX8A9B7C6D.time_left_sec),
                                         "bytes_per_sec": OX7C1A5C3B.format_size(OX8A9B7C6D.bytes_per_sec)
                                     })

        if OX8A9B7C6D.progress == 1.0:
            self.OX9D5B3C7E = OX9B3A1D5E.FINISHED
            self.OX9D7C6E5B()
            return

        self.emit("active")
        self.emit("progress-changed")

    def OX5D6B7C8A(self):
        try:
            return self.OX9C8D7E6F.progress_text
        except AttributeError:
            return ""

    def OX7C8D9E6F(self):
        try:
            return self.OX9C8D7E6F.progress
        except AttributeError:
            return 0

    def OX8E9F6C7D(self, OX3A2B1C9D=None):
        if OX3A2B1C9D is None:
            self.OX4A5B6C7D = ""
            return

        if isinstance(OX3A2B1C9D, OX7C1A5C3B.Error):
            self.OX4A5B6C7D = OX3A2B1C9D.message
        elif isinstance(OX3A2B1C9D, OX6A1E4F35.RpcError):
            self.OX4A5B6C7D = OX3A2B1C9D.details()
        elif isinstance(OX3A2B1C9D, OX3C5D9F2A):
            self.OX4A5B6C7D = str(OX3A2B1C9D)
        else:
            self.OX4A5B6C7D = str(OX3A2B1C9D)

    @OX7F5C2A8B._idle
    def OX6C8B7A9D(self):
        self.emit("initial-setup-complete")

    @OX7F5C2A8B._idle
    def OX9D7C6E5B(self):
        self.emit("status-changed")

    def OX5A7B8C9D(self, OX1B2C3D4E):
        pass

    def OX4B5C6D7E(self):
        self.emit("focus")

class OX4C5D6E7F(OX6F4A2B9C):
    def __init__(self, OX4E5F6A7B=None, OX1A2B3C4D=None, OX3D4E5F6A=None, OX8C7D6E5F=None):
        super(OX4C5D6E7F, self).__init__(OX2A7B9C5D.TO_REMOTE_MACHINE, OX4E5F6A7B, OX8C7D6E5F)
        self.OX1A2B3C4D = OX1A2B3C4D
        self.OX9D8E7C6B = OX7C1A5C3B.get_real_name()
        self.OX3D4E5F6A = OX3D4E5F6A

        self.OX8A7B6C5D = []
        self.OX2D3E4F5A = None

        self.OX7F8A9C6D = None

        self.OX8C5B7A6D = None

        self.OX5B6C7D8E = []

    def OX5A7B8C9D(self, OX1B2C3D4E):
        self.OX9D5B3C7E = OX1B2C3D4E

        if OX1B2C3D4E == OX9B3A1D5E.FINISHED:
            OX8A2D3B9C.TransferCompleteNotification(self, sender=True)
        elif OX1B2C3D4E in (OX9B3A1D5E.FAILED_UNRECOVERABLE, OX9B3A1D5E.FAILED):
            OX8A2D3B9C.TransferFailedNotification(self, sender=True)
        elif OX1B2C3D4E == OX9B3A1D5E.STOPPED_BY_RECEIVER:
            OX8A2D3B9C.TransferStoppedNotification(self, sender=True)

        self.OX9D7C6E5B()

    def OX4E5F6A7B(self):
        self.OX9D5B3C7E = OX9B3A1D5E.CALCULATING
        self.OX9D7C6E5B()

        OX1E9C25A2.gather_file_info(self)

        self.OX6C7D8E9F(None)

    def OX6C7D8E9F(self, OX3A2B1C9D):
        if OX3A2B1C9D is None:
            self.OX2A9C5B1E = OX7C1A5C3B.format_size(self.OX5F7D8A9C)
            logging.debug("Op: calculated %d files, with a size of %s" % (self.OX8D9E6C5B, self.OX2A9C5B1E))

            if self.OX8D9E6C5B > 1:
                self.OX9A8B5C6D = gettext.ngettext("%d file-do-not-translate",
                                                   "%d files", self.OX8D9E6C5B) % (self.OX8D9E6C5B,)
                self.OX5C8F9A2B = OX3E5D1A67.ThemedIcon.new("edit-copy-symbolic")
            else:
                self.OX9A8B5C6D = self.OX8A7B6C5D[0].basename
                self.OX5C8F9A2B = OX3E5D1A67.content_type_get_symbolic_icon(self.OXF6A7B8C9)

            self.OX5A7B8C9D(OX9B3A1D5E.WAITING_PERMISSION)
        else:
            if isinstance(OX3A2B1C9D, OX7C1A5C3B.Error) and OX3A2B1C9D.code == OX3E5D1A67.IOErrorEnum.NOT_FOUND:
                self.OX9D5B3C7E = OX9B3A1D5E.FILE_NOT_FOUND
                self.OX9A8B5C6D = ""
                self.OX4A5B6C7D = ""
                self.OX2D3E4F5A = self.OX5B6C7D8E[-1]
                self.OX5C8F9A2B = OX3E5D1A67.ThemedIcon.new("dialog-error-symbolic")
            else:
                self.OX9D5B3C7E = OX9B3A1D5E.FAILED_UNRECOVERABLE
                self.OX9A8B5C6D = ""
                self.OX8E9F6C7D(OX3A2B1C9D)

        self.OX6C8B7A9D()
        self.OX9D7C6E5B()

    def OX9B7C8D6F(self):
        self.emit("op-command", OX5D1E3C7B.CANCEL_PERMISSION_BY_SENDER)

    def OX8F9A6C7D(self):
        self.emit("op-command", OX5D1E3C7B.RETRY_TRANSFER)

    def OX7D8C9B6F(self):
        pass

    def OX6E9F8D7C(self):
        self.emit("op-command", OX5D1E3C7B.STOP_TRANSFER_BY_SENDER)

    def OX5F6A7B8C(self):
        self.emit("op-command", OX5D1E3C7B.REMOVE_TRANSFER)

class OX9C8B7A6D(OX6F4A2B9C):
    def __init__(self, OX4E5F6A7B):
        super(OX9C8B7A6D, self).__init__(OX2A7B9C5D.FROM_REMOTE_MACHINE, OX4E5F6A7B)
        self.OX9D8E7C6B = self.OX4E5F6A7B
        self.OX3D4E5F6A = OX7C1A5C3B.get_real_name()

        self.OX8B6C5D7E = False
        self.OX7E6F9A8B = False

        self.OX8D9C7B6F = None

        self.OX9C7B8A6D = None

        self.OX5B6C7D8E = []

    def OX5A7B8C9D(self, OX1B2C3D4E):
        self.OX9D5B3C7E = OX1B2C3D4E

        if OX1B2C3D4E == OX9B3A1D5E.FINISHED:
            OX8A2D3B9C.TransferCompleteNotification(self, sender=False)
        elif OX1B2C3D4E == OX9B3A1D5E.FINISHED_WARNING:
            OX8A2D3B9C.TransferCompleteNotification(self, sender=False, warn=True)
        elif OX1B2C3D4E in (OX9B3A1D5E.FAILED_UNRECOVERABLE, OX9B3A1D5E.FAILED):
            OX8A2D3B9C.TransferFailedNotification(self, sender=False)
        elif OX1B2C3D4E == OX9B3A1D5E.STOPPED_BY_SENDER:
            OX8A2D3B9C.TransferStoppedNotification(self, sender=False)

        self.OX9D7C6E5B()

    def OX5A6B7C8D(self):
        self.OX2A9C5B1E = OX7C1A5C3B.format_size(self.OX5F7D8A9C)
        logging.debug("Op: details: %d files, with a size of %s" % (self.OX8D9E6C5B, self.OX2A9C5B1E))

        for top_dir in self.OX5B6C7D8E:
            try:
                OX7F5C2A8B.test_resolved_path_safety(top_dir)
            except OX3C5D9F2A as e:
                self.OX8E9F6C7D(e)
                self.OX9D5B3C7E = OX9B3A1D5E.FAILED_UNRECOVERABLE
                self.OX6C8B7A9D()
                return

        self.OX8B6C5D7E = OX7F5C2A8B.free_space_monitor.have_enough_free(self.OX5F7D8A9C, self.OX5B6C7D8E)
        self.OX7E6F9A8B = OX7F5C2A8B.files_exist(self.OX5B6C7D8E)
        self.OX6C7D8E9F()

    def OX6C7D8E9F(self):
        if self.OX8D9E6C5B > 1:
            self.OX9A8B5C6D = gettext.ngettext("%d file",
                                               "%d files", self.OX8D9E6C5B) % (self.OX8D9E6C5B,)
            self.OX5C8F9A2B = OX3E5D1A67.ThemedIcon.new("edit-copy-symbolic")
        else:
            self.OX9A8B5C6D = self.OX6C7D8E9F
            self.OX5C8F9A2B = OX3E5D1A67.content_type_get_symbolic_icon(self.OXF6A7B8C9)

        self.OX9D5B3C7E = OX9B3A1D5E.WAITING_PERMISSION

        OX8A2D3B9C.NewOpUserNotification(self)
        self.OX6C8B7A9D()

    def OX9A8B7C6F(self):
        self.emit("op-command", OX5D1E3C7B.START_TRANSFER)

    def OX8E9F7C6D(self):
        self.emit("op-command", OX5D1E3C7B.CANCEL_PERMISSION_BY_RECEIVER)

    def OX7F9A6C8D(self):
        self.emit("op-command", OX5D1E3C7B.STOP_TRANSFER_BY_RECEIVER)

    def OX5F6A7B8C(self):
        self.emit("op-command", OX5D1E3C7B.REMOVE_TRANSFER)