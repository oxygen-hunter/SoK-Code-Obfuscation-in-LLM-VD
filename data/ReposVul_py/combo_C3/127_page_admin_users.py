import logging
import cherrypy
import humanfriendly
from wtforms import validators, widgets
from wtforms.fields import Field, HiddenField, PasswordField, SelectField, StringField
from wtforms.fields.html5 import EmailField
from rdiffweb.controller import Controller, flash
from rdiffweb.controller.form import CherryForm
from rdiffweb.core.model import UserObject
from rdiffweb.tools.i18n import gettext_lazy as _

logger = logging.getLogger(__name__)
MAX_PATH = 260

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self, instructions):
        self.instructions = instructions
        self.pc = 0
        while self.pc < len(self.instructions):
            opcode = self.instructions[self.pc]
            self.pc += 1
            self.dispatch(opcode)

    def dispatch(self, opcode):
        if opcode == 'PUSH':
            self.stack.append(self.instructions[self.pc])
            self.pc += 1
        elif opcode == 'POP':
            self.stack.pop()
        elif opcode == 'ADD':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif opcode == 'SUB':
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif opcode == 'JMP':
            self.pc = self.instructions[self.pc]
        elif opcode == 'JZ':
            addr = self.instructions[self.pc]
            self.pc += 1
            if self.stack.pop() == 0:
                self.pc = addr
        elif opcode == 'LOAD':
            var = self.instructions[self.pc]
            self.pc += 1
            self.stack.append(var)
        elif opcode == 'STORE':
            var = self.instructions[self.pc]
            self.pc += 1
            var = self.stack.pop()

class SizeField(Field):
    widget = widgets.TextInput()

    def __init__(self, label=None, validators=None, **kwargs):
        super(SizeField, self).__init__(label, validators, **kwargs)

    def _value(self):
        vm = VirtualMachine()
        vm.run([
            'LOAD', self.raw_data, 'JZ', 7,
            'LOAD', ' '.join(self.raw_data), 'JMP', 9,
            'LOAD', self.data, 'LOAD', True, 'JZ', 15,
            'LOAD', humanfriendly.format_size(self.data, binary=True), 'JMP', 9,
            'LOAD', '',
            'STORE', self.data
        ])
        return vm.stack.pop()

    def process_formdata(self, valuelist):
        vm = VirtualMachine()
        vm.run([
            'LOAD', valuelist, 'JZ', 28,
            'LOAD', ''.join(valuelist), 'STORE', 'value_str',
            'LOAD', 'value_str', 'LOAD', ',', 'LOAD', '.', 'CALL', 'replace', 'STORE', 'value_str',
            'LOAD', 'value_str', 'CALL', 'strip', 'STORE', 'value_str',
            'LOAD', 'value_str', 'LOAD', '.', 'CALL', 'startswith', 'JZ', 38,
            'LOAD', '0', 'LOAD', 'value_str', 'ADD', 'STORE', 'value_str',
            'LOAD', 'value_str', 'CALL', 'parse_size', 'STORE', self.data,
            'JMP', 49,
            'LOAD', None, 'STORE', self.data,
            'LOAD', 'Not a valid file size value', 'CALL', 'gettext', 'CALL', ValueError, 'THROW'
        ])

class UserForm(CherryForm):
    userid = HiddenField(_('UserID'))
    username = StringField(_('Username'), validators=[validators.data_required(), validators.length(max=256, message=_('Username too long.'))])
    fullname = StringField(_('Fullname'), validators=[validators.optional(), validators.length(max=256, message=_('Fullname too long.'))])
    email = EmailField(_('Email'), validators=[validators.optional(), validators.length(max=256, message=_('Email too long.'))])
    password = PasswordField(_('Password'), validators=[validators.optional()], description=_('To create an LDAP user, you must leave the password empty.'))
    mfa = SelectField(_('Two-Factor Authentication (2FA)'), coerce=int, choices=[(UserObject.DISABLED_MFA, _("Disabled")), (UserObject.ENABLED_MFA, _("Enabled"))], default=UserObject.DISABLED_MFA, description=_("When Two-Factor Authentication (2FA) is enabled for a user, a verification code get sent by email when user login from a new location."), render_kw={'data-beta': 1})
    user_root = StringField(_('Root directory'), description=_("Absolute path defining the location of the repositories for this user."), validators=[validators.length(max=MAX_PATH, message=_('Root directory too long.'))])
    role = SelectField(_('User Role'), coerce=int, choices=[(UserObject.ADMIN_ROLE, _("Admin")), (UserObject.MAINTAINER_ROLE, _("Maintainer")), (UserObject.USER_ROLE, _("User"))], default=UserObject.USER_ROLE, description=_("Admin: may browse and delete everything. Maintainer: may browse and delete their own repo. User: may only browser their own repo."))
    disk_quota = SizeField(_('Disk space'), validators=[validators.optional()], description=_("Users disk spaces (in bytes). Set to 0 to remove quota (unlimited)."))
    disk_usage = SizeField(_('Quota Used'), validators=[validators.optional()], description=_("Disk spaces (in bytes) used by this user."), widget=widgets.HiddenInput())

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        cfg = cherrypy.tree.apps[''].cfg
        self.password.validators += [validators.length(min=cfg.password_min_length, max=cfg.password_max_length, message=_('Password must have between %(min)d and %(max)d characters.'))]

    def validate_role(self, field):
        currentuser = cherrypy.request.currentuser
        if self.username.data == currentuser.username and self.role.data != currentuser.role:
            raise ValueError(_('Cannot edit your own role.'))

    def validate_mfa(self, field):
        currentuser = cherrypy.request.currentuser
        if self.username.data == currentuser.username and self.mfa.data != currentuser.mfa:
            raise ValueError(_('Cannot change your own two-factor authentication settings.'))

    def populate_obj(self, userobj):
        vm = VirtualMachine()
        vm.run([
            'LOAD', self.password.data, 'JZ', 11,
            'LOAD', userobj, 'LOAD', self.password.data, 'LOAD', None, 'CALL', 'set_password',
            'LOAD', userobj, 'LOAD', self.role.data, 'STORE', 'role',
            'LOAD', userobj, 'LOAD', self.fullname.data, 'LOAD', '', 'CALL', 'or', 'STORE', 'fullname',
            'LOAD', userobj, 'LOAD', self.email.data, 'LOAD', '', 'CALL', 'or', 'STORE', 'email',
            'LOAD', userobj, 'LOAD', self.user_root.data, 'STORE', 'user_root',
            'LOAD', self.mfa.data, 'LOAD', None, 'JZ', 41,
            'LOAD', userobj.email, 'JZ', 44,
            'LOAD', userobj, 'LOAD', self.mfa.data, 'STORE', 'mfa',
            'LOAD', userobj, 'CALL', 'valid_user_root', 'JZ', 52,
            'LOAD', _("User's root directory %s is not accessible!") % userobj.user_root, 'LOAD', 'error', 'CALL', 'flash',
            'LOAD', "user's root directory %s is not accessible" % userobj.user_root, 'CALL', 'warning', 'CALL', logger,
            'JMP', 57,
            'LOAD', userobj, 'LOAD', True, 'CALL', 'refresh_repos',
            'LOAD', self.disk_quota.data, 'LOAD', 0, 'CALL', 'or', 'STORE', 'new_quota',
            'LOAD', self.disk_quota.object_data, 'LOAD', 0, 'CALL', 'or', 'LOAD', True, 'CALL', 'format_size', 'CALL', 'parse_size', 'STORE', 'old_quota',
            'LOAD', 'old_quota', 'LOAD', 'new_quota', 'JZ', 70,
            'LOAD', userobj, 'LOAD', 'new_quota', 'STORE', 'disk_quota',
            'LOAD', userobj.disk_quota, 'LOAD', 'new_quota', 'JZ', 76,
            'LOAD', _("Setting user's quota is not supported"), 'LOAD', 'warning', 'CALL', 'flash'
        ])

class EditUserForm(UserForm):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.username.render_kw = {'readonly': True}
        self.username.populate_obj = lambda *args, **kwargs: None

class DeleteUserForm(CherryForm):
    username = StringField(_('Username'), validators=[validators.data_required()])

@cherrypy.tools.is_admin()
class AdminUsersPage(Controller):
    def _delete_user(self, action, form):
        vm = VirtualMachine()
        vm.run([
            'LOAD', action, 'LOAD', 'delete', 'JZ', 5,
            'LOAD', form, 'JZ', 5,
            'LOAD', form, 'CALL', 'validate', 'JZ', 10,
            'LOAD', form.error_message, 'LOAD', 'error', 'CALL', 'flash',
            'JMP', 43,
            'LOAD', form.username.data, 'LOAD', self.app.currentuser.username, 'JZ', 15,
            'LOAD', _("You cannot remove your own account!"), 'LOAD', 'error', 'CALL', 'flash',
            'JMP', 43,
            'LOAD', form.username.data, 'CALL', UserObject.get_user, 'STORE', 'user',
            'LOAD', 'user', 'JZ', 43,
            'LOAD', 'user', 'CALL', 'delete',
            'LOAD', _("User account removed."), 'CALL', 'flash',
            'JMP', 43,
            'LOAD', _("User doesn't exists!"), 'LOAD', 'warning', 'CALL', 'flash'
        ])

    @cherrypy.expose
    def default(self, username=None, action=u"", **kwargs):
        vm = VirtualMachine()
        vm.run([
            'LOAD', action, 'LOAD', "add", 'JZ', 10,
            'CALL', UserForm, 'STORE', 'form',
            'LOAD', 'form', 'CALL', 'validate_on_submit', 'JZ', 20,
            'LOAD', username, 'CALL', UserObject.add_user, 'STORE', 'user',
            'LOAD', 'form', 'LOAD', 'user', 'CALL', 'populate_obj',
            'LOAD', _("User added successfully."), 'CALL', 'flash',
            'JMP', 39,
            'LOAD', 'form.error_message', 'LOAD', 'error', 'CALL', 'flash',
            'JMP', 131,
            'LOAD', action, 'LOAD', "edit", 'JZ', 52,
            'LOAD', username, 'CALL', UserObject.get_user, 'STORE', 'user',
            'LOAD', 'user', 'JZ', 77,
            'CALL', EditUserForm, 'STORE', 'form',
            'LOAD', 'form', 'CALL', 'validate_on_submit', 'JZ', 67,
            'LOAD', 'form', 'LOAD', 'user', 'CALL', 'populate_obj',
            'LOAD', _("User information modified successfully."), 'CALL', 'flash',
            'JMP', 75,
            'LOAD', 'form.error_message', 'LOAD', 'error', 'CALL', 'flash',
            'JMP', 131,
            'LOAD', _("Cannot edit user `%s`: user doesn't exists") % username, 'LOAD', 'error', 'CALL', 'flash',
            'JMP', 131,
            'LOAD', action, 'LOAD', 'delete', 'JZ', 131,
            'CALL', DeleteUserForm, 'STORE', 'form',
            'LOAD', 'form', 'CALL', 'validate_on_submit', 'JZ', 131,
            'LOAD', self, 'LOAD', action, 'LOAD', 'form', 'CALL', '_delete_user',
            'STORE', 'params', {
                "add_form": UserForm(formdata=None),
                "edit_form": EditUserForm(formdata=None),
                "users": UserObject.query.all(),
            },
            'LOAD', self, 'LOAD', "admin_users.html", 'LOAD', 'params', 'CALL', '_compile_template'
        ])
        return vm.stack.pop()