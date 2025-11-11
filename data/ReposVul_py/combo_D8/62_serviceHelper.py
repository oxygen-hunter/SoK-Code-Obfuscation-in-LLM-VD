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

def getUdevRulesPath():
    return '/etc/udev/rules.d/99-backintime-%s.rules'

def getWhitelist():
    return (
        (getNice(), ("-n")),
        (getIonice(), ("-c", "-n")),
    )

def getMaxRules():
    return 100

def getMaxUsers():
    return 20

def getMaxCmdLen():
    return 100

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

    def _which(self, exe, fallback):
        proc = Popen(['which', exe], stdout=PIPE)
        ret = proc.communicate()[0].strip().decode()
        if proc.returncode or not ret:
            return fallback
        return ret

    def _validateCmd(self, cmd):
        if cmd.find("&&") != -1:
            raise InvalidCmd("Parameter 'cmd' contains '&&' concatenation")
        elif not cmd.startswith(os.path.sep):
            raise InvalidCmd("Parameter 'cmd' does not start with '/'")

        parts = cmd.split()

        whitelist = getWhitelist()

        for c, switches in whitelist:
            if parts and parts[0] == c:
                parts.pop(0)
                for sw in switches:
                    while parts and parts[0].startswith(sw):
                        parts.pop(0)

        if not parts:
            raise InvalidCmd("Parameter 'cmd' does not contain the backintime command")
        elif parts[0] != self.backintime:
            raise InvalidCmd("Parameter 'cmd' contains non-whitelisted cmd/parameter (%s)" % parts[0])

    def _checkLimits(self, owner, cmd):
        if len(self.tmpDict.get(owner, [])) >= getMaxRules():
            raise LimitExceeded("Maximum number of cached rules reached (%d)" % getMaxRules())
        elif len(self.tmpDict) >= getMaxUsers():
            raise LimitExceeded("Maximum number of cached users reached (%d)" % getMaxUsers())
        elif len(cmd) > getMaxCmdLen():
            raise LimitExceeded("Maximum length of command line reached (%d)" % getMaxCmdLen())

    @dbus.service.method("net.launchpad.backintime.serviceHelper.UdevRules",
                         in_signature='ss', out_signature='',
                         sender_keyword='sender', connection_keyword='conn')
    def addRule(self, cmd, uuid, sender=None, conn=None):
        chars = re.findall(r'[^a-zA-Z0-9-/\.>& ]', cmd)
        if chars:
            raise InvalidChar("Parameter 'cmd' contains invalid character(s) %s"
                              % '|'.join(set(chars)))
        chars = re.findall(r'[^a-zA-Z0-9-]', uuid)
        if chars:
            raise InvalidChar("Parameter 'uuid' contains invalid character(s) %s"
                              % '|'.join(set(chars)))

        self._validateCmd(cmd)

        info = SenderInfo(sender, conn)
        user = info.connectionUnixUser()
        owner = info.nameOwner()

        self._checkLimits(owner, cmd)

        sucmd = "%s - '%s' -c '%s'" % (self.su, user, cmd)
        rule = 'ACTION=="add|change", ENV{ID_FS_UUID}=="%s", RUN+="%s"\n' % (uuid, sucmd)

        if not owner in self.tmpDict:
            self.tmpDict[owner] = []
        self.tmpDict[owner].append(rule)

    @dbus.service.method("net.launchpad.backintime.serviceHelper.UdevRules",
                         in_signature='', out_signature='b',
                         sender_keyword='sender', connection_keyword='conn')
    def save(self, sender=None, conn=None):
        info = SenderInfo(sender, conn)
        user = info.connectionUnixUser()
        owner = info.nameOwner()

        if not owner in self.tmpDict or not self.tmpDict[owner]:
            self.delete(sender, conn)
            return False
        if os.path.exists(getUdevRulesPath() % user):
            with open(getUdevRulesPath() % user, 'r') as f:
                if self.tmpDict[owner] == f.readlines():
                    self._clean(owner)
                    return False
        self._checkPolkitPrivilege(sender, conn, 'net.launchpad.backintime.UdevRuleSave')
        with open(getUdevRulesPath() % user, 'w') as f:
            f.writelines(self.tmpDict[owner])
        self._clean(owner)
        return True

    @dbus.service.method("net.launchpad.backintime.serviceHelper.UdevRules",
                         in_signature='', out_signature='',
                         sender_keyword='sender', connection_keyword='conn')
    def delete(self, sender=None, conn=None):
        info = SenderInfo(sender, conn)
        user = info.connectionUnixUser()
        owner = info.nameOwner()
        self._clean(owner)
        if os.path.exists(getUdevRulesPath() % user):
            self._checkPolkitPrivilege(sender, conn, 'net.launchpad.backintime.UdevRuleDelete')
            os.remove(getUdevRulesPath() % user)

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
        if sender is None and conn is None:
            return
        if not self.enforce_polkit:
            return

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

def getNice():
    return '/usr/bin/nice'

def getIonice():
    return '/usr/bin/ionice'

if __name__ == '__main__':
    dbus.mainloop.pyqt5.DBusQtMainLoop(set_as_default=True)

    app = QCoreApplication([])

    bus = dbus.SystemBus()
    name = dbus.service.BusName("net.launchpad.backintime.serviceHelper", bus)
    object = UdevRules(bus, '/UdevRules')

    print("Running BIT service.")
    app.exec_()