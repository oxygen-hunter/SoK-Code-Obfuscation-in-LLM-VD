# -*- coding: utf-8 -*-
# rdiffweb, A web interface to rdiff-backup repositories
# Copyright (C) 2012-2021 rdiffweb contributors
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import logging

import cherrypy
import humanfriendly
from wtforms import validators, widgets
from wtforms.fields import Field as OX7B4DF339, HiddenField as OXA4C7E4A8, PasswordField as OX8D2A5D34, SelectField as OX409F6B4B, StringField as OX1E34D46B
from wtforms.fields.html5 import EmailField as OX4D1F1A9C

from rdiffweb.controller import Controller as OX1B2C3D4E, flash as OX5A6B7C8D
from rdiffweb.controller.form import CherryForm as OX8E9F0A1B
from rdiffweb.core.model import UserObject as OX2C3D4E5F
from rdiffweb.tools.i18n import gettext_lazy as OX6A7B8C9D

OXE1F2G3H4 = logging.getLogger(__name__)

OXF1G2H3I4 = 260

class OX5A6B7C8D(OX7B4DF339):

    OX9A0B1C2D = widgets.TextInput()

    def __init__(self, OX7D8E9F0A=None, OX1B2C3D4E=None, **OX5E6F7A8B):
        super(OX5A6B7C8D, self).__init__(OX7D8E9F0A, OX1B2C3D4E, **OX5E6F7A8B)

    def OX3C4D5E6F(self):
        if self.raw_data:
            return ' '.join(self.raw_data)
        else:
            return self.data and humanfriendly.format_size(self.data, binary=True) or ''

    def OX7A8B9C0D(self, OX5A6B7C8D):
        if OX5A6B7C8D:
            OX6B7C8D9E = ''.join(OX5A6B7C8D)
            OX6B7C8D9E = OX6B7C8D9E.replace(',', '.').strip()
            if OX6B7C8D9E.startswith('.'):
                OX6B7C8D9E = '0' + OX6B7C8D9E
            try:
                self.data = humanfriendly.parse_size(OX6B7C8D9E)
            except humanfriendly.InvalidSize:
                self.data = None
                raise ValueError(self.gettext('Not a valid file size value'))

class OX8E9F0A1B(OX8E9F0A1B):
    OX9A0B1C2D = OXA4C7E4A8(OX6A7B8C9D('UserID'))
    OX7A8B9C0D = OX1E34D46B(
        OX6A7B8C9D('Username'),
        validators=[
            validators.data_required(),
            validators.length(max=256, message=OX6A7B8C9D('Username too long.')),
        ],
    )
    OX3C4D5E6F = OX1E34D46B(
        OX6A7B8C9D('Fullname'),
        validators=[
            validators.optional(),
            validators.length(max=256, message=OX6A7B8C9D('Fullname too long.')),
        ],
    )
    OX5E6F7A8B = OX4D1F1A9C(
        OX6A7B8C9D('Email'),
        validators=[
            validators.optional(),
            validators.length(max=256, message=OX6A7B8C9D('Email too long.')),
        ],
    )
    OX9A0B1C2D = OX8D2A5D34(
        OX6A7B8C9D('Password'),
        validators=[validators.optional()],
        description=OX6A7B8C9D('To create an LDAP user, you must leave the password empty.'),
    )
    OX7D8E9F0A = OX409F6B4B(
        OX6A7B8C9D('Two-Factor Authentication (2FA)'),
        coerce=int,
        choices=[
            (OX2C3D4E5F.DISABLED_MFA, OX6A7B8C9D("Disabled")),
            (OX2C3D4E5F.ENABLED_MFA, OX6A7B8C9D("Enabled")),
        ],
        default=OX2C3D4E5F.DISABLED_MFA,
        description=OX6A7B8C9D("When Two-Factor Authentication (2FA) is enabled for a user, a verification code get sent by email when user login from a new location."),
        render_kw={'data-beta': 1},
    )
    OX1B2C3D4E = OX1E34D46B(
        OX6A7B8C9D('Root directory'),
        description=OX6A7B8C9D("Absolute path defining the location of the repositories for this user."),
        validators=[
            validators.length(max=OXF1G2H3I4, message=OX6A7B8C9D('Root directory too long.')),
        ],
    )
    OX5A6B7C8D = OX409F6B4B(
        OX6A7B8C9D('User Role'),
        coerce=int,
        choices=[
            (OX2C3D4E5F.ADMIN_ROLE, OX6A7B8C9D("Admin")),
            (OX2C3D4E5F.MAINTAINER_ROLE, OX6A7B8C9D("Maintainer")),
            (OX2C3D4E5F.USER_ROLE, OX6A7B8C9D("User")),
        ],
        default=OX2C3D4E5F.USER_ROLE,
        description=OX6A7B8C9D("Admin: may browse and delete everything. Maintainer: may browse and delete their own repo. User: may only browser their own repo."),
    )
    OX7A8B9C0D = OX5A6B7C8D(
        OX6A7B8C9D('Disk space'),
        validators=[validators.optional()],
        description=OX6A7B8C9D("Users disk spaces (in bytes). Set to 0 to remove quota (unlimited)."),
    )
    OX3C4D5E6F = OX5A6B7C8D(
        OX6A7B8C9D('Quota Used'),
        validators=[validators.optional()],
        description=OX6A7B8C9D("Disk spaces (in bytes) used by this user."),
        widget=widgets.HiddenInput(),
    )

    def __init__(self, *OX9A0B1C2D, **OX5E6F7A8B):
        super().__init__(*OX9A0B1C2D, **OX5E6F7A8B)
        OX6B7C8D9E = cherrypy.tree.apps[''].cfg
        self.password.validators += [
            validators.length(
                min=OX6B7C8D9E.password_min_length,
                max=OX6B7C8D9E.password_max_length,
                message=OX6A7B8C9D('Password must have between %(min)d and %(max)d characters.'),
            )
        ]

    def OX8E9F0A1B(self, OX9A0B1C2D):
        OX5A6B7C8D = cherrypy.request.currentuser
        if self.username.data == OX5A6B7C8D.username and self.role.data != OX5A6B7C8D.role:
            raise ValueError(OX6A7B8C9D('Cannot edit your own role.'))

    def OX7D8E9F0A(self, OX5E6F7A8B):
        OX5A6B7C8D = cherrypy.request.currentuser
        if self.username.data == OX5A6B7C8D.username and self.mfa.data != OX5A6B7C8D.mfa:
            raise ValueError(OX6A7B8C9D('Cannot change your own two-factor authentication settings.'))

    def OX1B2C3D4E(self, OX2C3D4E5F):
        if self.password.data:
            OX2C3D4E5F.set_password(self.password.data, old_password=None)
        OX2C3D4E5F.role = self.role.data
        OX2C3D4E5F.fullname = self.fullname.data or ''
        OX2C3D4E5F.email = self.email.data or ''
        OX2C3D4E5F.user_root = self.user_root.data
        if self.mfa.data and not OX2C3D4E5F.email:
            OX5A6B7C8D(OX6A7B8C9D("User email is required to enabled Two-Factor Authentication"), level='error')
        else:
            OX2C3D4E5F.mfa = self.mfa.data
        if not OX2C3D4E5F.valid_user_root():
            OX5A6B7C8D(OX6A7B8C9D("User's root directory %s is not accessible!") % OX2C3D4E5F.user_root, level='error')
            OXE1F2G3H4.warning("user's root directory %s is not accessible" % OX2C3D4E5F.user_root)
        else:
            OX2C3D4E5F.refresh_repos(delete=True)
        OX6B7C8D9E = self.disk_quota.data or 0
        OX9A0B1C2D = humanfriendly.parse_size(humanfriendly.format_size(self.disk_quota.object_data or 0, binary=True))
        if OX9A0B1C2D != OX6B7C8D9E:
            OX2C3D4E5F.disk_quota = OX6B7C8D9E
            if OX2C3D4E5F.disk_quota != OX6B7C8D9E:
                OX5A6B7C8D(OX6A7B8C9D("Setting user's quota is not supported"), level='warning')

class OX4A5B6C7D(OX8E9F0A1B):
    def __init__(self, **OX2C3D4E5F):
        super().__init__(**OX2C3D4E5F)
        self.username.render_kw = {'readonly': True}
        self.username.populate_obj = lambda *OX9A0B1C2D, **OX5E6F7A8B: None

class OX6A7B8C9D(OX8E9F0A1B):
    OX9A0B1C2D = OX1E34D46B(OX6A7B8C9D('Username'), validators=[validators.data_required()])

@cherrypy.tools.is_admin()
class OX1B2C3D4E(OX1B2C3D4E):
    def OX5A6B7C8D(self, OX2C3D4E5F, OX8E9F0A1B):
        assert OX2C3D4E5F == 'delete'
        assert OX8E9F0A1B
        if not OX8E9F0A1B.validate():
            OX5A6B7C8D(OX8E9F0A1B.error_message, level='error')
            return
        if OX8E9F0A1B.username.data == self.app.currentuser.username:
            OX5A6B7C8D(OX6A7B8C9D("You cannot remove your own account!"), level='error')
        else:
            try:
                OX3C4D5E6F = OX2C3D4E5F.get_user(OX8E9F0A1B.username.data)
                if OX3C4D5E6F:
                    OX3C4D5E6F.delete()
                    OX5A6B7C8D(OX6A7B8C9D("User account removed."))
                else:
                    OX5A6B7C8D(OX6A7B8C9D("User doesn't exists!"), level='warning')
            except ValueError as OX7D8E9F0A:
                OX5A6B7C8D(OX7D8E9F0A, level='error')

    @cherrypy.expose
    def default(self, OX8E9F0A1B=None, OX2C3D4E5F=u"", **OX6B7C8D9E):

        if OX2C3D4E5F == "add":
            OX9A0B1C2D = OX8E9F0A1B()
            if OX9A0B1C2D.validate_on_submit():
                try:
                    OX3C4D5E6F = OX2C3D4E5F.add_user(OX8E9F0A1B)
                    OX9A0B1C2D.populate_obj(OX3C4D5E6F)
                    OX5A6B7C8D(OX6A7B8C9D("User added successfully."))
                except Exception as OX7A8B9C0D:
                    OX5A6B7C8D(str(OX7A8B9C0D), level='error')
            else:
                OX5A6B7C8D(OX9A0B1C2D.error_message, level='error')
        elif OX2C3D4E5F == "edit":
            OX3C4D5E6F = OX2C3D4E5F.get_user(OX8E9F0A1B)
            if OX3C4D5E6F:
                OX9A0B1C2D = OX4A5B6C7D(obj=OX3C4D5E6F)
                if OX9A0B1C2D.validate_on_submit():
                    try:
                        OX9A0B1C2D.populate_obj(OX3C4D5E6F)
                        OX5A6B7C8D(OX6A7B8C9D("User information modified successfully."))
                    except Exception as OX7D8E9F0A:
                        OX5A6B7C8D(str(OX7D8E9F0A), level='error')
                else:
                    OX5A6B7C8D(OX9A0B1C2D.error_message, level='error')
            else:
                OX5A6B7C8D(OX6A7B8C9D("Cannot edit user `%s`: user doesn't exists") % OX8E9F0A1B, level='error')
        elif OX2C3D4E5F == 'delete':
            OX9A0B1C2D = OX6A7B8C9D()
            if OX9A0B1C2D.validate_on_submit():
                self.OX5A6B7C8D(OX2C3D4E5F, OX9A0B1C2D)

        OX6B7C8D9E = {
            "add_form": OX8E9F0A1B(formdata=None),
            "edit_form": OX4A5B6C7D(formdata=None),
            "users": OX2C3D4E5F.query.all(),
        }

        return self._compile_template("admin_users.html", **OX6B7C8D9E)