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

def get_max_path():
    return 260

def get_user_object_disabled_mfa():
    return UserObject.DISABLED_MFA

def get_user_object_enabled_mfa():
    return UserObject.ENABLED_MFA

def get_user_object_admin_role():
    return UserObject.ADMIN_ROLE

def get_user_object_maintainer_role():
    return UserObject.MAINTAINER_ROLE

def get_user_object_user_role():
    return UserObject.USER_ROLE

def get_user_object_default_role():
    return UserObject.USER_ROLE

class SizeField(Field):
    widget = widgets.TextInput()

    def __init__(self, label=None, validators=None, **kwargs):
        super(SizeField, self).__init__(label, validators, **kwargs)

    def _value(self):
        if self.raw_data:
            return ' '.join(self.raw_data)
        else:
            return self.data and humanfriendly.format_size(self.data, binary=True) or ''

    def process_formdata(self, valuelist):
        if valuelist:
            value_str = ''.join(valuelist)
            value_str = value_str.replace(',', '.').strip()
            if value_str.startswith('.'):
                value_str = '0' + value_str
            try:
                self.data = humanfriendly.parse_size(value_str)
            except humanfriendly.InvalidSize:
                self.data = None
                raise ValueError(self.gettext('Not a valid file size value'))


class UserForm(CherryForm):
    userid = HiddenField(_('UserID'))
    username = StringField(
        _('Username'),
        validators=[
            validators.data_required(),
            validators.length(max=256, message=_('Username too long.')),
        ],
    )
    fullname = StringField(
        _('Fullname'),
        validators=[
            validators.optional(),
            validators.length(max=256, message=_('Fullname too long.')),
        ],
    )
    email = EmailField(
        _('Email'),
        validators=[
            validators.optional(),
            validators.length(max=256, message=_('Email too long.')),
        ],
    )
    password = PasswordField(
        _('Password'),
        validators=[validators.optional()],
        description=_('To create an LDAP user, you must leave the password empty.'),
    )
    mfa = SelectField(
        _('Two-Factor Authentication (2FA)'),
        coerce=int,
        choices=[
            (get_user_object_disabled_mfa(), _("Disabled")),
            (get_user_object_enabled_mfa(), _("Enabled")),
        ],
        default=get_user_object_disabled_mfa(),
        description=_(
            "When Two-Factor Authentication (2FA) is enabled for a user, a verification code get sent by email when user login from a new location."
        ),
        render_kw={'data-beta': 1},
    )
    user_root = StringField(
        _('Root directory'),
        description=_("Absolute path defining the location of the repositories for this user."),
        validators=[
            validators.length(max=get_max_path(), message=_('Root directory too long.')),
        ],
    )
    role = SelectField(
        _('User Role'),
        coerce=int,
        choices=[
            (get_user_object_admin_role(), _("Admin")),
            (get_user_object_maintainer_role(), _("Maintainer")),
            (get_user_object_user_role(), _("User")),
        ],
        default=get_user_object_default_role(),
        description=_(
            "Admin: may browse and delete everything. Maintainer: may browse and delete their own repo. User: may only browser their own repo."
        ),
    )
    disk_quota = SizeField(
        _('Disk space'),
        validators=[validators.optional()],
        description=_("Users disk spaces (in bytes). Set to 0 to remove quota (unlimited)."),
    )
    disk_usage = SizeField(
        _('Quota Used'),
        validators=[validators.optional()],
        description=_("Disk spaces (in bytes) used by this user."),
        widget=widgets.HiddenInput(),
    )

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        cfg = cherrypy.tree.apps[''].cfg
        self.password.validators += [
            validators.length(
                min=cfg.password_min_length,
                max=cfg.password_max_length,
                message=_('Password must have between %(min)d and %(max)d characters.'),
            )
        ]

    def validate_role(self, field):
        currentuser = cherrypy.request.currentuser
        if self.username.data == currentuser.username and self.role.data != currentuser.role:
            raise ValueError(_('Cannot edit your own role.'))

    def validate_mfa(self, field):
        currentuser = cherrypy.request.currentuser
        if self.username.data == currentuser.username and self.mfa.data != currentuser.mfa:
            raise ValueError(_('Cannot change your own two-factor authentication settings.'))

    def populate_obj(self, userobj):
        if self.password.data:
            userobj.set_password(self.password.data, old_password=None)
        userobj.role = self.role.data
        userobj.fullname = self.fullname.data or ''
        userobj.email = self.email.data or ''
        userobj.user_root = self.user_root.data
        if self.mfa.data and not userobj.email:
            flash(_("User email is required to enabled Two-Factor Authentication"), level='error')
        else:
            userobj.mfa = self.mfa.data
        if not userobj.valid_user_root():
            flash(_("User's root directory %s is not accessible!") % userobj.user_root, level='error')
            logger.warning("user's root directory %s is not accessible" % userobj.user_root)
        else:
            userobj.refresh_repos(delete=True)
        new_quota = self.disk_quota.data or 0
        old_quota = humanfriendly.parse_size(humanfriendly.format_size(self.disk_quota.object_data or 0, binary=True))
        if old_quota != new_quota:
            userobj.disk_quota = new_quota
            if userobj.disk_quota != new_quota:
                flash(_("Setting user's quota is not supported"), level='warning')


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
        assert action == 'delete'
        assert form
        if not form.validate():
            flash(form.error_message, level='error')
            return
        if form.username.data == self.app.currentuser.username:
            flash(_("You cannot remove your own account!"), level='error')
        else:
            try:
                user = UserObject.get_user(form.username.data)
                if user:
                    user.delete()
                    flash(_("User account removed."))
                else:
                    flash(_("User doesn't exists!"), level='warning')
            except ValueError as e:
                flash(e, level='error')

    @cherrypy.expose
    def default(self, username=None, action=u"", **kwargs):
        if action == "add":
            form = UserForm()
            if form.validate_on_submit():
                try:
                    user = UserObject.add_user(username)
                    form.populate_obj(user)
                    flash(_("User added successfully."))
                except Exception as e:
                    flash(str(e), level='error')
            else:
                flash(form.error_message, level='error')
        elif action == "edit":
            user = UserObject.get_user(username)
            if user:
                form = EditUserForm(obj=user)
                if form.validate_on_submit():
                    try:
                        form.populate_obj(user)
                        flash(_("User information modified successfully."))
                    except Exception as e:
                        flash(str(e), level='error')
                else:
                    flash(form.error_message, level='error')
            else:
                flash(_("Cannot edit user `%s`: user doesn't exists") % username, level='error')
        elif action == 'delete':
            form = DeleteUserForm()
            if form.validate_on_submit():
                self._delete_user(action, form)

        params = {
            "add_form": UserForm(formdata=None),
            "edit_form": EditUserForm(formdata=None),
            "users": UserObject.query.all(),
        }
        return self._compile_template("admin_users.html", **params)