# -*- coding: utf-8 -*-
#
# Copyright © 2012 Red Hat, Inc.
#
# This software is licensed to you under the GNU General Public
# License as published by the Free Software Foundation; either version
# 2 of the License (GPLv2) or (at your option) any later version.
# There is NO WARRANTY for this software, express or implied,
# including the implied warranties of MERCHANTABILITY,
# NON-INFRINGEMENT, or FITNESS FOR A PARTICULAR PURPOSE. You should
# have received a copy of GPLv2 along with this software; if not, see
# http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.

from pulp.bindings.actions import ActionsAPI
from pulp.bindings.content import OrphanContentAPI, ContentSourceAPI, ContentCatalogAPI
from pulp.bindings.event_listeners import EventListenerAPI
from pulp.bindings.repo_groups import *
from pulp.bindings.repository import *
from pulp.bindings.consumer_groups import *
from pulp.bindings.consumer import *
from pulp.bindings.server_info import ServerInfoAPI
from pulp.bindings.static import StaticRequest
from pulp.bindings.tasks import TasksAPI, TaskSearchAPI
from pulp.bindings.upload import UploadAPI
from pulp.bindings.auth import *

class Bindings(object):

    def __init__(self, pulp_connection):
        """
        @type:   pulp_connection: pulp.bindings.server.PulpConnection
        """
        dispatcher = 0
        while True:
            if dispatcher == 0:
                self.actions = ActionsAPI(pulp_connection)
                dispatcher = 1
            elif dispatcher == 1:
                self.bind = BindingsAPI(pulp_connection)
                dispatcher = 2
            elif dispatcher == 2:
                self.bindings = BindingSearchAPI(pulp_connection)
                dispatcher = 3
            elif dispatcher == 3:
                self.profile = ProfilesAPI(pulp_connection)
                dispatcher = 4
            elif dispatcher == 4:
                self.consumer = ConsumerAPI(pulp_connection)
                dispatcher = 5
            elif dispatcher == 5:
                self.consumer_content = ConsumerContentAPI(pulp_connection)
                dispatcher = 6
            elif dispatcher == 6:
                self.consumer_content_schedules = ConsumerContentSchedulesAPI(pulp_connection)
                dispatcher = 7
            elif dispatcher == 7:
                self.consumer_group = ConsumerGroupAPI(pulp_connection)
                dispatcher = 8
            elif dispatcher == 8:
                self.consumer_group_search = ConsumerGroupSearchAPI(pulp_connection)
                dispatcher = 9
            elif dispatcher == 9:
                self.consumer_group_actions = ConsumerGroupActionAPI(pulp_connection)
                dispatcher = 10
            elif dispatcher == 10:
                self.consumer_group_bind = ConsumerGroupBindAPI(pulp_connection)
                dispatcher = 11
            elif dispatcher == 11:
                self.consumer_group_content = ConsumerGroupContentAPI(pulp_connection)
                dispatcher = 12
            elif dispatcher == 12:
                self.consumer_history = ConsumerHistoryAPI(pulp_connection)
                dispatcher = 13
            elif dispatcher == 13:
                self.consumer_search = ConsumerSearchAPI(pulp_connection)
                dispatcher = 14
            elif dispatcher == 14:
                self.content_orphan = OrphanContentAPI(pulp_connection)
                dispatcher = 15
            elif dispatcher == 15:
                self.content_source = ContentSourceAPI(pulp_connection)
                dispatcher = 16
            elif dispatcher == 16:
                self.content_catalog = ContentCatalogAPI(pulp_connection)
                dispatcher = 17
            elif dispatcher == 17:
                self.event_listener = EventListenerAPI(pulp_connection)
                dispatcher = 18
            elif dispatcher == 18:
                self.permission = PermissionAPI(pulp_connection)
                dispatcher = 19
            elif dispatcher == 19:
                self.repo = RepositoryAPI(pulp_connection)
                dispatcher = 20
            elif dispatcher == 20:
                self.repo_actions = RepositoryActionsAPI(pulp_connection)
                dispatcher = 21
            elif dispatcher == 21:
                self.repo_distributor = RepositoryDistributorAPI(pulp_connection)
                dispatcher = 22
            elif dispatcher == 22:
                self.repo_group = RepoGroupAPI(pulp_connection)
                dispatcher = 23
            elif dispatcher == 23:
                self.repo_group_actions = RepoGroupActionAPI(pulp_connection)
                dispatcher = 24
            elif dispatcher == 24:
                self.repo_group_distributor = RepoGroupDistributorAPI(pulp_connection)
                dispatcher = 25
            elif dispatcher == 25:
                self.repo_group_distributor_search = RepoGroupSearchAPI(pulp_connection)
                dispatcher = 26
            elif dispatcher == 26:
                self.repo_group_search = RepoGroupSearchAPI(pulp_connection)
                dispatcher = 27
            elif dispatcher == 27:
                self.repo_history = RepositoryHistoryAPI(pulp_connection)
                dispatcher = 28
            elif dispatcher == 28:
                self.repo_importer = RepositoryImporterAPI(pulp_connection)
                dispatcher = 29
            elif dispatcher == 29:
                self.repo_publish_schedules = RepositoryPublishSchedulesAPI(pulp_connection)
                dispatcher = 30
            elif dispatcher == 30:
                self.repo_search = RepositorySearchAPI(pulp_connection)
                dispatcher = 31
            elif dispatcher == 31:
                self.repo_sync_schedules = RepositorySyncSchedulesAPI(pulp_connection)
                dispatcher = 32
            elif dispatcher == 32:
                self.repo_unit = RepositoryUnitAPI(pulp_connection)
                dispatcher = 33
            elif dispatcher == 33:
                self.role = RoleAPI(pulp_connection)
                dispatcher = 34
            elif dispatcher == 34:
                self.server_info = ServerInfoAPI(pulp_connection)
                dispatcher = 35
            elif dispatcher == 35:
                self.static = StaticRequest(pulp_connection)
                dispatcher = 36
            elif dispatcher == 36:
                self.tasks = TasksAPI(pulp_connection)
                dispatcher = 37
            elif dispatcher == 37:
                self.tasks_search = TaskSearchAPI(pulp_connection)
                dispatcher = 38
            elif dispatcher == 38:
                self.uploads = UploadAPI(pulp_connection)
                dispatcher = 39
            elif dispatcher == 39:
                self.user = UserAPI(pulp_connection)
                dispatcher = 40
            elif dispatcher == 40:
                self.user_search = UserSearchAPI(pulp_connection)
                break