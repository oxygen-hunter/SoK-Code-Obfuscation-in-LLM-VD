import os
import re
from subprocess import Popen, PIPE
try:
    import pwd
except ImportError:
    pwd = None

import dbus
import dbus.service
import dbus.mainloop.pyqt5
from PyQt5.QtCore import QCoreApplication

UDEV_RULES_PATH = '/etc/udev/rules.d/99-backintime-%s.rules'

class InvalidChar(dbus.DBusException):
    _dbus_error_name = 'net.launchpad.backintime.InvalidChar'

class InvalidCmd(dbus.DBusException):
    _dbus_error_name = 'net.launchpad.backintime.InvalidCmd'

class LimitExceeded(dbus.DBusException):
    _dbus_error_name = 'net.launchpad.backintime.LimitExceeded'

class PermissionDeniedByPolicy(dbus.DBusException):
    _dbus_error_name = 'com.ubuntu.DeviceDriver.PermissionDeniedByPolicy'

class UdevRules(dbus.service.Object):
    def __init__(self, conn=None, object_path=None, bus_name=None):
        super(UdevRules, self).__init__(conn, object_path, bus_name)

        self.polkit = None
        self.enforce_polkit = True

        self.tmpDict = {}

        self.su = self._which('su', '/bin/su')
        self.backintime = self._which('backintime', '/usr/bin/backintime')
        self.nice = self._which('nice', '/usr/bin/nice')
        self.ionice = self._which('ionice', '/usr/bin/ionice')
        self.max_rules = 100
        self.max_users = 20
        self.max_cmd_len = 100

    def _which(self, exe, fallback):
        proc = Popen(['which', exe], stdout = PIPE)
        ret = proc.communicate()[0].strip().decode()
        if proc.returncode or not ret:
            return fallback

        return ret

    def _validateCmd(self, cmd):
        state = 0
        while state is not None:
            if state == 0:
                if cmd.find("&&") != -1:
                    state = 1
                elif not cmd.startswith(os.path.sep):
                    state = 2
                else:
                    state = 3
            elif state == 1:
                raise InvalidCmd("Parameter 'cmd' contains '&&' concatenation")
            elif state == 2:
                raise InvalidCmd("Parameter 'cmd' does not start with '/'")
            elif state == 3:
                parts = cmd.split()
                whitelist = (
                    (self.nice, ("-n")),
                    (self.ionice, ("-c", "-n")),
                )
                state = 4
            elif state == 4:
                for c, switches in whitelist:
                    if parts and parts[0] == c:
                        parts.pop(0)
                        for sw in switches:
                            while parts and parts[0].startswith(sw):
                                parts.pop(0)
                state = 5
            elif state == 5:
                if not parts:
                    state = 6
                elif parts[0] != self.backintime:
                    state = 7
                else:
                    state = None
            elif state == 6:
                raise InvalidCmd("Parameter 'cmd' does not contain the backintime command")
            elif state == 7:
                raise InvalidCmd("Parameter 'cmd' contains non-whitelisted cmd/parameter (%s)" % parts[0])

    def _checkLimits(self, owner, cmd):
        state = 0
        while state is not None:
            if state == 0:
                if len(self.tmpDict.get(owner, [])) >= self.max_rules:
                    state = 1
                elif len(self.tmpDict) >= self.max_users:
                    state = 2
                elif len(cmd) > self.max_cmd_len:
                    state = 3
                else:
                    state = None
            elif state == 1:
                raise LimitExceeded("Maximum number of cached rules reached (%d)"
                                % self.max_rules)
            elif state == 2:
                raise LimitExceeded("Maximum number of cached users reached (%d)"
                                % self.max_users)
            elif state == 3:
                raise LimitExceeded("Maximum length of command line reached (%d)"
                                % self.max_cmd_len)

    @dbus.service.method("net.launchpad.backintime.serviceHelper.UdevRules",
                         in_signature='ss', out_signature='',
                         sender_keyword='sender', connection_keyword='conn')
    def addRule(self, cmd, uuid, sender=None, conn=None):
        state = 0
        while state is not None:
            if state == 0:
                chars = re.findall(r'[^a-zA-Z0-9-/\.>& ]', cmd)
                if chars:
                    state = 1
                else:
                    state = 2
            elif state == 1:
                raise InvalidChar("Parameter 'cmd' contains invalid character(s) %s"
                                  % '|'.join(set(chars)))
            elif state == 2:
                chars = re.findall(r'[^a-zA-Z0-9-]', uuid)
                if chars:
                    state = 3
                else:
                    state = 4
            elif state == 3:
                raise InvalidChar("Parameter 'uuid' contains invalid character(s) %s"
                                  % '|'.join(set(chars)))
            elif state == 4:
                self._validateCmd(cmd)
                info = SenderInfo(sender, conn)
                user = info.connectionUnixUser()
                owner = info.nameOwner()
                state = 5
            elif state == 5:
                self._checkLimits(owner, cmd)
                sucmd = "%s - '%s' -c '%s'" %(self.su, user, cmd)
                rule = 'ACTION=="add|change", ENV{ID_FS_UUID}=="%s", RUN+="%s"\n' %(uuid, sucmd)
                state = 6
            elif state == 6:
                if not owner in self.tmpDict:
                    self.tmpDict[owner] = []
                self.tmpDict[owner].append(rule)
                state = None

    @dbus.service.method("net.launchpad.backintime.serviceHelper.UdevRules",
                         in_signature='', out_signature='b',
                         sender_keyword='sender', connection_keyword='conn')
    def save(self, sender=None, conn=None):
        state = 0
        while state is not None:
            if state == 0:
                info = SenderInfo(sender, conn)
                user = info.connectionUnixUser()
                owner = info.nameOwner()
                if not owner in self.tmpDict or not self.tmpDict[owner]:
                    state = 1
                elif os.path.exists(UDEV_RULES_PATH % user):
                    state = 2
                else:
                    state = 4
            elif state == 1:
                self.delete(sender, conn)
                return False
            elif state == 2:
                with open(UDEV_RULES_PATH % user, 'r') as f:
                    if self.tmpDict[owner] == f.readlines():
                        state = 3
                    else:
                        state = 4
            elif state == 3:
                self._clean(owner)
                return False
            elif state == 4:
                self._checkPolkitPrivilege(sender, conn, 'net.launchpad.backintime.UdevRuleSave')
                with open(UDEV_RULES_PATH % user, 'w') as f:
                    f.writelines(self.tmpDict[owner])
                self._clean(owner)
                return True

    @dbus.service.method("net.launchpad.backintime.serviceHelper.UdevRules",
                         in_signature='', out_signature='',
                         sender_keyword='sender', connection_keyword='conn')
    def delete(self, sender=None, conn=None):
        state = 0
        while state is not None:
            if state == 0:
                info = SenderInfo(sender, conn)
                user = info.connectionUnixUser()
                owner = info.nameOwner()
                self._clean(owner)
                if os.path.exists(UDEV_RULES_PATH % user):
                    state = 1
                else:
                    state = None
            elif state == 1:
                self._checkPolkitPrivilege(sender, conn, 'net.launchpad.backintime.UdevRuleDelete')
                os.remove(UDEV_RULES_PATH % user)
                state = None

    @dbus.service.method("net.launchpad.backintime.serviceHelper.UdevRules",
                         in_signature='', out_signature='',
                         sender_keyword='sender', connection_keyword='conn')
    def clean(self, sender=None, conn=None):
        info = SenderInfo(sender, conn)
        self._clean(info.nameOwner())

    def _clean(self, owner):
        if owner in self.tmpDict:
            del self.tmpDict[owner]

    def _initPolkit(self):
        if self.polkit is None:
            self.polkit = dbus.Interface(dbus.SystemBus().get_object(
                'org.freedesktop.PolicyKit1',
                '/org/freedesktop/PolicyKit1/Authority', False),
                'org.freedesktop.PolicyKit1.Authority')

    def _checkPolkitPrivilege(self, sender, conn, privilege):
        state = 0
        while state is not None:
            if state == 0:
                if sender is None and conn is None:
                    return
                elif not self.enforce_polkit:
                    return
                else:
                    state = 1
            elif state == 1:
                self._initPolkit()
                try:
                    (is_auth, _, details) = self.polkit.CheckAuthorization(
                        ('system-bus-name', {'name': dbus.String(sender, variant_level=1)}),
                        privilege, {'': ''}, dbus.UInt32(1), '', timeout=3000)
                except dbus.DBusException as e:
                    if e._dbus_error_name == 'org.freedesktop.DBus.Error.ServiceUnknown':
                        self.polkit = None
                        return self._checkPolkitPrivilege(sender, conn, privilege)
                    else:
                        raise
                if not is_auth:
                    state = 2
                else:
                    state = None
            elif state == 2:
                raise PermissionDeniedByPolicy(privilege)

class SenderInfo(object):
    def __init__(self, sender, conn):
        self.sender = sender
        self.dbus_info = dbus.Interface(conn.get_object('org.freedesktop.DBus',
                '/org/freedesktop/DBus/Bus', False), 'org.freedesktop.DBus')

    def connectionUnixUser(self):
        uid = self.dbus_info.GetConnectionUnixUser(self.sender)
        if pwd:
            return pwd.getpwuid(uid).pw_name
        else:
            return uid

    def nameOwner(self):
        return self.dbus_info.GetNameOwner(self.sender)

    def connectionPid(self):
        return self.dbus_info.GetConnectionUnixProcessID(self.sender)

if __name__ == '__main__':
    dbus.mainloop.pyqt5.DBusQtMainLoop(set_as_default=True)

    app = QCoreApplication([])

    bus = dbus.SystemBus()
    name = dbus.service.BusName("net.launchpad.backintime.serviceHelper", bus)
    object = UdevRules(bus, '/UdevRules')

    print("Running BIT service.")
    app.exec_()