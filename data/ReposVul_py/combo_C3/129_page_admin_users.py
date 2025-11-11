import logging
import cherrypy
import humanfriendly
from wtforms import validators, widgets
from wtforms.fields import Field, HiddenField, PasswordField, SelectField, StringField
from wtforms.validators import ValidationError

try:
    from wtforms.fields import EmailField
except ImportError:
    from wtforms.fields.html5 import EmailField

from rdiffweb.controller import Controller, flash
from rdiffweb.controller.form import CherryForm
from rdiffweb.core.model import UserObject
from rdiffweb.core.rdw_templating import url_for
from rdiffweb.tools.i18n import gettext_lazy as _

logger = logging.getLogger(__name__)

MAX_PATH = 260

class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.program = []
        self.registers = {}

    def load_program(self, program):
        self.program = program

    def run(self):
        while self.pc < len(self.program):
            inst, *args = self.program[self.pc]
            getattr(self, f"op_{inst}")(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        a = self.op_POP()
        b = self.op_POP()
        self.op_PUSH(a + b)

    def op_SUB(self):
        a = self.op_POP()
        b = self.op_POP()
        self.op_PUSH(a - b)

    def op_LOAD(self, reg):
        self.op_PUSH(self.registers[reg])

    def op_STORE(self, reg):
        self.registers[reg] = self.op_POP()

    def op_JMP(self, addr):
        self.pc = addr - 1

    def op_JZ(self, addr):
        if self.op_POP() == 0:
            self.pc = addr - 1

class SizeField(Field):
    widget = widgets.TextInput()

    def __init__(self, label=None, validators=None, **kwargs):
        super(SizeField, self).__init__(label, validators, **kwargs)

    def _value(self):
        vm = SimpleVM()
        vm.load_program([
            ("PUSH", self.raw_data),
            ("JZ", 3),
            ("POP",),
            ("JMP", 5),
            ("LOAD", 'data'),
            ("STORE", 'formatted'),
            ("PUSH", self.data),
            ("JZ", 9),
            ("LOAD", 'formatted'),
            ("STORE", 'humanfriendly'),
            ("PUSH", " or "),
            ("ADD",),
            ("LOAD", 'humanfriendly'),
            ("ADD",),
        ])
        vm.run()
        return ''.join(vm.stack) or ''

    def process_formdata(self, valuelist):
        vm = SimpleVM()
        vm.load_program([
            ("PUSH", valuelist),
            ("JZ", 3),
            ("POP",),
            ("JMP", 30),
            ("PUSH", ''.join(valuelist)),
            ("STORE", 'value_str'),
            ("LOAD", 'value_str'),
            ("PUSH", ','),
            ("PUSH", '.'),
            ("ADD",),
            ("STORE", 'value_str'),
            ("LOAD", 'value_str'),
            ("PUSH", '.'),
            ("JZ", 15),
            ("LOAD", 'value_str'),
            ("PUSH", '0'),
            ("ADD",),
            ("STORE", 'value_str'),
            ("LOAD", 'value_str'),
            ("STORE", 'trimmed'),
            ("LOAD", 'trimmed'),
            ("STORE", 'parsed'),
            ("LOAD", 'parsed'),
            ("STORE", 'data'),
            ("JMP", 30),
            ("LOAD", 'humanfriendly'),
            ("STORE", 'None'),
            ("LOAD", 'ValidationError'),
            ("STORE", 'self'),
        ])
        vm.run()
        value_str = vm.registers.get('value_str', '')
        try:
            self.data = humanfriendly.parse_size(value_str)
        except humanfriendly.InvalidSize:
            self.data = None
            raise ValidationError(self.gettext('Not a valid file size value'))

class UserForm(CherryForm):
    userid = HiddenField(_('UserID'))
    username = StringField(_('Username'), validators=[validators.data_required(), validators.length(max=256, message=_('Username too long.')), validators.length(min=3, message=_('Username too short.')), validators.regexp(UserObject.PATTERN_USERNAME, message=_('Must not contain any special characters.')),])
    fullname = StringField(_('Fullname'), validators=[validators.optional(), validators.length(max=256, message=_('Fullname too long.')), validators.regexp(UserObject.PATTERN_FULLNAME, message=_('Must not contain any special characters.')),])
    email = EmailField(_('Email'), validators=[validators.optional(), validators.length(max=256, message=_('Email too long.')), validators.regexp(UserObject.PATTERN_EMAIL, message=_('Must be a valid email address.')),])
    password = PasswordField(_('Password'), validators=[validators.optional()],)
    mfa = SelectField(_('Two-Factor Authentication (2FA)'), coerce=int, choices=[(UserObject.DISABLED_MFA, _("Disabled")), (UserObject.ENABLED_MFA, _("Enabled")),], default=UserObject.DISABLED_MFA, description=_("When Two-Factor Authentication (2FA) is enabled for a user, a verification code get sent by email when user login from a new location."),)
    user_root = StringField(_('Root directory'), description=_("Absolute path defining the location of the repositories for this user."), validators=[validators.length(max=MAX_PATH, message=_('Root directory too long.')),],)
    role = SelectField(_('User Role'), coerce=int, choices=[(UserObject.ADMIN_ROLE, _("Admin")), (UserObject.MAINTAINER_ROLE, _("Maintainer")), (UserObject.USER_ROLE, _("User")),], default=UserObject.USER_ROLE, description=_("Admin: may browse and delete everything. Maintainer: may browse and delete their own repo. User: may only browser their own repo."),)
    report_time_range = SelectField(_('Send Backup report'), choices=[(0, _('Never')), (1, _('Daily')), (7, _('Weekly')), (30, _('Monthly')),], coerce=int, default='0',)
    disk_quota = SizeField(_('Disk space'), validators=[validators.optional()], description=_("Users disk spaces (in bytes). Set to 0 to remove quota (unlimited)."),)
    disk_usage = SizeField(_('Quota Used'), validators=[validators.optional()], description=_("Disk spaces (in bytes) used by this user."), widget=widgets.HiddenInput(),)

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.quota_enabled = len(cherrypy.engine.listeners.get('set_disk_quota', [])) > 0
        if not self.quota_enabled:
            self.disk_quota.render_kw = {'readonly': True, 'disabled': True}
            self.disk_usage.render_kw = {'readonly': True, 'disabled': True}
        cfg = cherrypy.request.app.cfg
        if cfg.ldap_uri:
            self.password.description = _('To create an LDAP user, you must leave the password empty.')

    def validate_role(self, field):
        currentuser = cherrypy.request.currentuser
        if self.username.data == currentuser.username and self.role.data != currentuser.role:
            raise ValidationError(_('Cannot edit your own role.'))

    def validate_mfa(self, field):
        currentuser = cherrypy.request.currentuser
        if self.username.data == currentuser.username and self.mfa.data != currentuser.mfa:
            raise ValidationError(_('Cannot change your own two-factor authentication settings.'))
        if not self.email.data and self.mfa.data:
            raise ValidationError(_('User email is required to enabled Two-Factor Authentication.'))

    def populate_obj(self, userobj):
        vm = SimpleVM()
        vm.load_program([
            ("PUSH", self.password.data),
            ("JZ", 5),
            ("LOAD", 'obj_password'),
            ("STORE", 'userobj'),
            ("LOAD", 'userobj'),
            ("STORE", 'set_password'),
            ("LOAD", 'self'),
            ("STORE", 'role_data'),
            ("LOAD", 'userobj'),
            ("STORE", 'role'),
            ("LOAD", 'self'),
            ("STORE", 'fullname_data'),
            ("LOAD", 'userobj'),
            ("STORE", 'fullname'),
            ("LOAD", 'self'),
            ("STORE", 'email_data'),
            ("LOAD", 'userobj'),
            ("STORE", 'email'),
            ("LOAD", 'self'),
            ("STORE", 'user_root_data'),
            ("LOAD", 'userobj'),
            ("STORE", 'user_root'),
            ("LOAD", 'self'),
            ("STORE", 'mfa_data'),
            ("LOAD", 'userobj'),
            ("STORE", 'mfa'),
            ("LOAD", 'self'),
            ("STORE", 'report_time_range_data'),
            ("LOAD", 'userobj'),
            ("STORE", 'report_time_range'),
            ("LOAD", 'userobj'),
            ("STORE", 'user_root'),
            ("LOAD", 'valid_user_root'),
            ("STORE", 'userobj'),
            ("LOAD", 'user_root'),
            ("STORE", 'userobj'),
            ("JZ", 55),
            ("LOAD", 'flash'),
            ("STORE", 'message'),
            ("LOAD", 'logger'),
            ("STORE", 'warning'),
            ("LOAD", 'userobj'),
            ("STORE", 'refresh_repos'),
            ("LOAD", 'delete'),
            ("STORE", 'True'),
            ("LOAD", 'userobj'),
            ("STORE", 'commit'),
            ("LOAD", 'False'),
            ("STORE", 'True'),
            ("LOAD", 'self'),
            ("STORE", 'quota_enabled'),
            ("JZ", 75),
            ("LOAD", 'self'),
            ("STORE", 'disk_quota_data'),
            ("LOAD", '0'),
            ("STORE", 'new_quota'),
            ("LOAD", 'humanfriendly'),
            ("STORE", 'parse_size'),
            ("LOAD", 'disk_quota'),
            ("STORE", 'self'),
            ("LOAD", 'object_data'),
            ("STORE", 'humanfriendly'),
            ("LOAD", 'binary'),
            ("STORE", 'True'),
            ("LOAD", 'old_quota'),
            ("STORE", 'humanfriendly'),
            ("LOAD", 'old_quota'),
            ("STORE", 'new_quota'),
            ("JZ", 95),
            ("LOAD", 'userobj'),
            ("STORE", 'disk_quota'),
            ("LOAD", 'new_quota'),
            ("STORE", 'set'),
            ("JZ", 105),
            ("LOAD", 'userobj'),
            ("STORE", 'disk_quota'),
            ("LOAD", 'new_quota'),
            ("STORE", 'not_equal'),
            ("LOAD", 'flash'),
            ("STORE", 'message'),
            ("LOAD", 'warning'),
            ("STORE", 'level'),
            ("JMP", 110),
            ("LOAD", 'True'),
            ("STORE", 'return'),
        ])
        vm.run()
        try:
            if self.password.data:
                userobj.set_password(self.password.data)
            userobj.role = self.role.data
            userobj.fullname = self.fullname.data or ''
            userobj.email = self.email.data or ''
            userobj.user_root = self.user_root.data
            userobj.mfa = self.mfa.data
            userobj.report_time_range = self.report_time_range.data
            if userobj.user_root:
                if not userobj.valid_user_root():
                    flash(_("User's root directory %s is not accessible!") % userobj.user_root, level='error')
                    logger.warning("user's root directory %s is not accessible" % userobj.user_root)
                else:
                    userobj.refresh_repos(delete=True)
            userobj.commit()
        except Exception as e:
            userobj.rollback()
            flash(str(e), level='warning')
            return False
        if self.quota_enabled:
            new_quota = self.disk_quota.data or 0
            old_quota = humanfriendly.parse_size(humanfriendly.format_size(self.disk_quota.object_data or 0, binary=True))
            if old_quota != new_quota:
                userobj.disk_quota = new_quota
                if userobj.disk_quota != new_quota:
                    flash(_("Setting user's quota is not supported"), level='warning')
        return True

class EditUserForm(UserForm):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.username.render_kw = {'readonly': True, 'disabled': True}
        currentuser = cherrypy.request.currentuser
        if self.username.object_data == currentuser.username:
            self.mfa.render_kw = {'readonly': True, 'disabled': True}
            self.role.render_kw = {'readonly': True, 'disabled': True}

    def validate_username(self, field):
        if field.data != field.object_data:
            raise ValidationError(_('Cannot change username of and existing user.'))

class DeleteUserForm(CherryForm):
    username = StringField(_('Username'), validators=[validators.data_required()])

@cherrypy.tools.is_admin()
class AdminUsersPage(Controller):
    @cherrypy.expose
    def index(self):
        form = UserForm()
        return self._compile_template("admin_users.html", form=form, users=UserObject.query.all(), ldap_enabled=self.app.cfg.ldap_uri,)

    @cherrypy.expose
    @cherrypy.tools.ratelimit(methods=['POST'])
    def new(self, **kwargs):
        form = UserForm()
        if form.is_submitted():
            if form.validate():
                try:
                    user = UserObject.add_user(form.username.data)
                except Exception as e:
                    flash(str(e), level='error')
                else:
                    if form.populate_obj(user):
                        flash(_("User added successfully."))
                        raise cherrypy.HTTPRedirect(url_for('admin', 'users'))
            else:
                flash(form.error_message, level='error')
        return self._compile_template("admin_user_new.html", form=form)

    @cherrypy.expose
    def edit(self, username_vpath, **kwargs):
        user = UserObject.get_user(username_vpath)
        if not user:
            raise cherrypy.HTTPError(400, _("User %s doesn't exists") % username_vpath)
        form = EditUserForm(obj=user)
        if form.is_submitted():
            if form.validate():
                if form.populate_obj(user):
                    flash(_("User information modified successfully."))
                    raise cherrypy.HTTPRedirect(url_for('admin', 'users'))
            else:
                flash(form.error_message, level='error')
        return self._compile_template("admin_user_edit.html", form=form)

    @cherrypy.expose
    def delete(self, username=None, **kwargs):
        form = DeleteUserForm()
        if not form.is_submitted():
            raise cherrypy.HTTPError(405)
        user = UserObject.get_user(username)
        if not user:
            raise cherrypy.HTTPError(400, _("User %s doesn't exists") % username)
        if form.validate():
            if form.username.data == self.app.currentuser.username:
                raise cherrypy.HTTPError(400, _("You cannot remove your own account!"))
            try:
                user.delete()
                user.commit()
                flash(_("User account removed."))
            except Exception as e:
                user.rollback()
                flash(str(e), level='error')
        else:
            flash(form.error_message, level='error')
        raise cherrypy.HTTPRedirect(url_for('admin', 'users'))