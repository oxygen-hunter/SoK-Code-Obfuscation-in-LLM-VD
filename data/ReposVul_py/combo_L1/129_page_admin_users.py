# -*- coding: utf-8 -*-

import logging

import cherrypy
import humanfriendly
from wtforms import validators as OX4AE0A, widgets as OXE0E9A
from wtforms.fields import Field as OXF3F6F6, HiddenField as OXEBB4B4, PasswordField as OX2B3A1D, SelectField as OX0D0B3E, StringField as OXEE4B98
from wtforms.validators import ValidationError as OX6A3A6D

try:
    from wtforms.fields import EmailField as OX2F7E8A
except ImportError:
    from wtforms.fields.html5 import EmailField as OX2F7E8A

from rdiffweb.controller import Controller as OX0600B7, flash as OX5A0CD3
from rdiffweb.controller.form import CherryForm as OX8363A1
from rdiffweb.core.model import UserObject as OX3475E0
from rdiffweb.core.rdw_templating import url_for as OX6B7DEB
from rdiffweb.tools.i18n import gettext_lazy as OX96D2D1

OX1DAE3F = logging.getLogger(__name__)

OX4B0E61 = 260

class OX82B2B1(OXF3F6F6):
    widget = OXE0E9A.TextInput()

    def __init__(self, OX60A1FD=None, OX40B6B6=None, **OXB0EFA4):
        super(OX82B2B1, self).__init__(OX60A1FD, OX40B6B6, **OXB0EFA4)

    def _value(self):
        if self.raw_data:
            return ' '.join(self.raw_data)
        else:
            return self.data and humanfriendly.format_size(self.data, binary=True) or ''

    def process_formdata(self, OXBC89BB):
        if OXBC89BB:
            OX6540E0 = ''.join(OXBC89BB)
            OX6540E0 = OX6540E0.replace(',', '.').strip()
            if OX6540E0.startswith('.'):
                OX6540E0 = '0' + OX6540E0
            try:
                self.data = humanfriendly.parse_size(OX6540E0)
            except humanfriendly.InvalidSize:
                self.data = None
                raise OX6A3A6D(self.gettext('Not a valid file size value'))

class OX6C7B38(OX8363A1):
    OX8B4D0B = OXEBB4B4(OX96D2D1('UserID'))
    OX6F6A3A = OXEE4B98(
        OX96D2D1('Username'),
        validators=[
            OX4AE0A.data_required(),
            OX4AE0A.length(max=256, message=OX96D2D1('Username too long.')),
            OX4AE0A.length(min=3, message=OX96D2D1('Username too short.')),
            OX4AE0A.regexp(OX3475E0.PATTERN_USERNAME, message=OX96D2D1('Must not contain any special characters.')),
        ],
    )
    OXCE6034 = OXEE4B98(
        OX96D2D1('Fullname'),
        validators=[
            OX4AE0A.optional(),
            OX4AE0A.length(max=256, message=OX96D2D1('Fullname too long.')),
            OX4AE0A.regexp(OX3475E0.PATTERN_FULLNAME, message=OX96D2D1('Must not contain any special characters.')),
        ],
    )
    OX2D6C4D = OX2F7E8A(
        OX96D2D1('Email'),
        validators=[
            OX4AE0A.optional(),
            OX4AE0A.length(max=256, message=OX96D2D1('Email too long.')),
            OX4AE0A.regexp(OX3475E0.PATTERN_EMAIL, message=OX96D2D1('Must be a valid email address.')),
        ],
    )
    OX0C9C7B = OX2B3A1D(
        OX96D2D1('Password'),
        validators=[OX4AE0A.optional()],
    )
    OX1FEC50 = OX0D0B3E(
        OX96D2D1('Two-Factor Authentication (2FA)'),
        coerce=int,
        choices=[
            (OX3475E0.DISABLED_MFA, OX96D2D1("Disabled")),
            (OX3475E0.ENABLED_MFA, OX96D2D1("Enabled")),
        ],
        default=OX3475E0.DISABLED_MFA,
        description=OX96D2D1(
            "When Two-Factor Authentication (2FA) is enabled for a user, a verification code get sent by email when user login from a new location."
        ),
    )
    OX3FF73D = OXEE4B98(
        OX96D2D1('Root directory'),
        description=OX96D2D1("Absolute path defining the location of the repositories for this user."),
        validators=[
            OX4AE0A.length(max=OX4B0E61, message=OX96D2D1('Root directory too long.')),
        ],
    )
    OX5F5A83 = OX0D0B3E(
        OX96D2D1('User Role'),
        coerce=int,
        choices=[
            (OX3475E0.ADMIN_ROLE, OX96D2D1("Admin")),
            (OX3475E0.MAINTAINER_ROLE, OX96D2D1("Maintainer")),
            (OX3475E0.USER_ROLE, OX96D2D1("User")),
        ],
        default=OX3475E0.USER_ROLE,
        description=OX96D2D1(
            "Admin: may browse and delete everything. Maintainer: may browse and delete their own repo. User: may only browser their own repo."
        ),
    )
    OX48A1A8 = OX0D0B3E(
        OX96D2D1('Send Backup report'),
        choices=[
            (0, OX96D2D1('Never')),
            (1, OX96D2D1('Daily')),
            (7, OX96D2D1('Weekly')),
            (30, OX96D2D1('Monthly')),
        ],
        coerce=int,
        default='0',
    )
    OX6AEF47 = OX82B2B1(
        OX96D2D1('Disk space'),
        validators=[OX4AE0A.optional()],
        description=OX96D2D1("Users disk spaces (in bytes). Set to 0 to remove quota (unlimited)."),
    )
    OX4D3F4A = OX82B2B1(
        OX96D2D1('Quota Used'),
        validators=[OX4AE0A.optional()],
        description=OX96D2D1("Disk spaces (in bytes) used by this user."),
        widget=OXE0E9A.HiddenInput(),
    )

    def __init__(self, **OXB0EFA4):
        super().__init__(**OXB0EFA4)
        self.quota_enabled = len(cherrypy.engine.listeners.get('set_disk_quota', [])) > 0
        if not self.quota_enabled:
            self.OX6AEF47.render_kw = {'readonly': True, 'disabled': True}
            self.OX4D3F4A.render_kw = {'readonly': True, 'disabled': True}
        OXBCB4D9 = cherrypy.request.app.cfg
        if OXBCB4D9.ldap_uri:
            self.OX0C9C7B.description = OX96D2D1('To create an LDAP user, you must leave the password empty.')

    def validate_role(self, OXBC89BB):
        OXCF7A6E = cherrypy.request.currentuser
        if self.OX6F6A3A.data == OXCF7A6E.username and self.OX5F5A83.data != OXCF7A6E.role:
            raise OX6A3A6D(OX96D2D1('Cannot edit your own role.'))

    def validate_mfa(self, OXBC89BB):
        OXCF7A6E = cherrypy.request.currentuser
        if self.OX6F6A3A.data == OXCF7A6E.username and self.OX1FEC50.data != OXCF7A6E.mfa:
            raise OX6A3A6D(OX96D2D1('Cannot change your own two-factor authentication settings.'))
        if not self.OX2D6C4D.data and self.OX1FEC50.data:
            raise OX6A3A6D(OX96D2D1('User email is required to enabled Two-Factor Authentication.'))

    def populate_obj(self, OX7D7FBB):
        try:
            if self.OX0C9C7B.data:
                OX7D7FBB.set_password(self.OX0C9C7B.data)
            OX7D7FBB.role = self.OX5F5A83.data
            OX7D7FBB.fullname = self.OXCE6034.data or ''
            OX7D7FBB.email = self.OX2D6C4D.data or ''
            OX7D7FBB.user_root = self.OX3FF73D.data
            OX7D7FBB.mfa = self.OX1FEC50.data
            OX7D7FBB.report_time_range = self.OX48A1A8.data
            if OX7D7FBB.user_root:
                if not OX7D7FBB.valid_user_root():
                    OX5A0CD3(OX96D2D1("User's root directory %s is not accessible!") % OX7D7FBB.user_root, level='error')
                    OX1DAE3F.warning("user's root directory %s is not accessible" % OX7D7FBB.user_root)
                else:
                    OX7D7FBB.refresh_repos(delete=True)
            OX7D7FBB.commit()

        except Exception as OX71FE88:
            OX7D7FBB.rollback()
            OX5A0CD3(str(OX71FE88), level='warning')
            return False

        if self.quota_enabled:
            OX8B94E6 = self.OX6AEF47.data or 0
            OX4FEC5D = humanfriendly.parse_size(
                humanfriendly.format_size(self.OX6AEF47.object_data or 0, binary=True)
            )
            if OX4FEC5D != OX8B94E6:
                OX7D7FBB.disk_quota = OX8B94E6
                if OX7D7FBB.disk_quota != OX8B94E6:
                    OX5A0CD3(OX96D2D1("Setting user's quota is not supported"), level='warning')
        return True

class OX8C5C6C(OX6C7B38):
    def __init__(self, **OXB0EFA4):
        super().__init__(**OXB0EFA4)
        self.OX6F6A3A.render_kw = {'readonly': True, 'disabled': True}

        OXCF7A6E = cherrypy.request.currentuser
        if self.OX6F6A3A.object_data == OXCF7A6E.username:
            self.OX1FEC50.render_kw = {'readonly': True, 'disabled': True}
            self.OX5F5A83.render_kw = {'readonly': True, 'disabled': True}

    def validate_username(self, OXBC89BB):
        if OXBC89BB.data != OXBC89BB.object_data:
            raise OX6A3A6D(OX96D2D1('Cannot change username of and existing user.'))

class OX7E6AFA(OX8363A1):
    OX6F6A3A = OXEE4B98(OX96D2D1('Username'), validators=[OX4AE0A.data_required()])

@cherrypy.tools.is_admin()
class OX28C1B8(OX0600B7):

    @cherrypy.expose
    def index(self):
        OX503A21 = OX6C7B38()
        return self._compile_template(
            "admin_users.html",
            form=OX503A21,
            users=OX3475E0.query.all(),
            ldap_enabled=self.app.cfg.ldap_uri,
        )

    @cherrypy.expose
    @cherrypy.tools.ratelimit(methods=['POST'])
    def new(self, **OXB0EFA4):
        OX503A21 = OX6C7B38()
        if OX503A21.is_submitted():
            if OX503A21.validate():
                try:
                    OX4F6A3B = OX3475E0.add_user(OX503A21.OX6F6A3A.data)
                except Exception as OX71FE88:
                    OX5A0CD3(str(OX71FE88), level='error')
                else:
                    if OX503A21.populate_obj(OX4F6A3B):
                        OX5A0CD3(OX96D2D1("User added successfully."))
                        raise cherrypy.HTTPRedirect(OX6B7DEB('admin', 'users'))
            else:
                OX5A0CD3(OX503A21.error_message, level='error')
        return self._compile_template("admin_user_new.html", form=OX503A21)

    @cherrypy.expose
    def edit(self, OX8B9A7A, **OXB0EFA4):
        OX4F6A3B = OX3475E0.get_user(OX8B9A7A)
        if not OX4F6A3B:
            raise cherrypy.HTTPError(400, OX96D2D1("User %s doesn't exists") % OX8B9A7A)
        OX503A21 = OX8C5C6C(obj=OX4F6A3B)
        if OX503A21.is_submitted():
            if OX503A21.validate():
                if OX503A21.populate_obj(OX4F6A3B):
                    OX5A0CD3(OX96D2D1("User information modified successfully."))
                    raise cherrypy.HTTPRedirect(OX6B7DEB('admin', 'users'))
            else:
                OX5A0CD3(OX503A21.error_message, level='error')
        return self._compile_template("admin_user_edit.html", form=OX503A21)

    @cherrypy.expose
    def delete(self, OX6F6A3A=None, **OXB0EFA4):
        OX503A21 = OX7E6AFA()
        if not OX503A21.is_submitted():
            raise cherrypy.HTTPError(405)
        OX4F6A3B = OX3475E0.get_user(OX6F6A3A)
        if not OX4F6A3B:
            raise cherrypy.HTTPError(400, OX96D2D1("User %s doesn't exists") % OX6F6A3A)
        if OX503A21.validate():
            if OX503A21.OX6F6A3A.data == self.app.currentuser.username:
                raise cherrypy.HTTPError(400, OX96D2D1("You cannot remove your own account!"))
            try:
                OX4F6A3B.delete()
                OX4F6A3B.commit()
                OX5A0CD3(OX96D2D1("User account removed."))
            except Exception as OX71FE88:
                OX4F6A3B.rollback()
                OX5A0CD3(str(OX71FE88), level='error')
        else:
            OX5A0CD3(OX503A21.error_message, level='error')
        raise cherrypy.HTTPRedirect(OX6B7DEB('admin', 'users'))