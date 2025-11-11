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

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
        self.registers = {}

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            instr = self.program[self.pc]
            getattr(self, f'op_{instr[0]}')(*instr[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        a = self.stack.pop()
        b = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, addr):
        self.pc = addr - 1

    def op_JZ(self, addr):
        if self.stack.pop() == 0:
            self.pc = addr - 1

    def op_LOAD(self, reg):
        self.stack.append(self.registers[reg])

    def op_STORE(self, reg):
        self.registers[reg] = self.stack.pop()

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
        self.vm = VirtualMachine()

    def _which(self, exe, fallback):
        proc = Popen(['which', exe], stdout=PIPE)
        ret = proc.communicate()[0].strip().decode()
        if proc.returncode or not ret:
            return fallback
        return ret

    def _validateCmd(self, cmd):
        self.vm.load_program([
            ('PUSH', cmd.find("&&")),
            ('JZ', 4),
            ('PUSH', "Parameter 'cmd' contains '&&' concatenation"),
            ('POP',),
            ('JMP', 8),
            ('PUSH', cmd.startswith(os.path.sep)),
            ('JZ', 10),
            ('PUSH', "Parameter 'cmd' does not start with '/'"),
            ('POP',)
        ])
        self.vm.run()

        parts = cmd.split()
        whitelist = (
            (self.nice, ("-n")),
            (self.ionice, ("-c", "-n")),
        )
        for c, switches in whitelist:
            if parts and parts[0] == c:
                parts.pop(0)
                for sw in switches:
                    while parts and parts[0].startswith(sw):
                        parts.pop(0)
        self.vm.load_program([
            ('PUSH', not parts),
            ('JZ', 4),
            ('PUSH', "Parameter 'cmd' does not contain the backintime command"),
            ('POP',),
            ('JMP', 8),
            ('PUSH', parts[0] != self.backintime),
            ('JZ', 10),
            ('PUSH', f"Parameter 'cmd' contains non-whitelisted cmd/parameter ({parts[0]})"),
            ('POP',)
        ])
        self.vm.run()

    def _checkLimits(self, owner, cmd):
        self.vm.load_program([
            ('PUSH', len(self.tmpDict.get(owner, []))),
            ('PUSH', self.max_rules),
            ('JZ', 4),
            ('PUSH', f"Maximum number of cached rules reached ({self.max_rules})"),
            ('POP',),
            ('PUSH', len(self.tmpDict)),
            ('PUSH', self.max_users),
            ('JZ', 10),
            ('PUSH', f"Maximum number of cached users reached ({self.max_users})"),
            ('POP',),
            ('PUSH', len(cmd)),
            ('PUSH', self.max_cmd_len),
            ('JZ', 16),
            ('PUSH', f"Maximum length of command line reached ({self.max_cmd_len})"),
            ('POP',)
        ])
        self.vm.run()

    @dbus.service.method("net.launchpad.backintime.serviceHelper.UdevRules",
                         in_signature='ss', out_signature='',
                         sender_keyword='sender', connection_keyword='conn')
    def addRule(self, cmd, uuid, sender=None, conn=None):
        chars = re.findall(r'[^a-zA-Z0-9-/\.>& ]', cmd)
        if chars:
            raise InvalidChar("Parameter 'cmd' contains invalid character(s) %s" % '|'.join(set(chars)))
        chars = re.findall(r'[^a-zA-Z0-9-]', uuid)
        if chars:
            raise InvalidChar("Parameter 'uuid' contains invalid character(s) %s" % '|'.join(set(chars)))

        self._validateCmd(cmd)
        info = SenderInfo(sender, conn)
        user = info.connectionUnixUser()
        owner = info.nameOwner()
        self._checkLimits(owner, cmd)

        sucmd = "%s - '%s' -c '%s'" % (self.su, user, cmd)
        rule = 'ACTION=="add|change", ENV{ID_FS_UUID}=="%s", RUN+="%s"\n' % (uuid, sucmd)

        if owner not in self.tmpDict:
            self.tmpDict[owner] = []
        self.tmpDict[owner].append(rule)

    @dbus.service.method("net.launchpad.backintime.serviceHelper.UdevRules",
                         in_signature='', out_signature='b',
                         sender_keyword='sender', connection_keyword='conn')
    def save(self, sender=None, conn=None):
        info = SenderInfo(sender, conn)
        user = info.connectionUnixUser()
        owner = info.nameOwner()

        if owner not in self.tmpDict or not self.tmpDict[owner]:
            self.delete(sender, conn)
            return False

        if os.path.exists(UDEV_RULES_PATH % user):
            with open(UDEV_RULES_PATH % user, 'r') as f:
                if self.tmpDict[owner] == f.readlines():
                    self._clean(owner)
                    return False

        self._checkPolkitPrivilege(sender, conn, 'net.launchpad.backintime.UdevRuleSave')
        with open(UDEV_RULES_PATH % user, 'w') as f:
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
        if os.path.exists(UDEV_RULES_PATH % user):
            self._checkPolkitPrivilege(sender, conn, 'net.launchpad.backintime.UdevRuleDelete')
            os.remove(UDEV_RULES_PATH % user)

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

if __name__ == '__main__':
    dbus.mainloop.pyqt5.DBusQtMainLoop(set_as_default=True)

    app = QCoreApplication([])

    bus = dbus.SystemBus()
    name = dbus.service.BusName("net.launchpad.backintime.serviceHelper", bus)
    object = UdevRules(bus, '/UdevRules')

    print("Running BIT service.")
    app.exec_()