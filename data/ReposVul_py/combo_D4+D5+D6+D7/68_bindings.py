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
        
        _api_objects = [
            ActionsAPI(pulp_connection),
            BindingsAPI(pulp_connection),
            BindingSearchAPI(pulp_connection),
            ProfilesAPI(pulp_connection),
            ConsumerAPI(pulp_connection),
            ConsumerContentAPI(pulp_connection),
            ConsumerContentSchedulesAPI(pulp_connection),
            ConsumerGroupAPI(pulp_connection),
            ConsumerGroupSearchAPI(pulp_connection),
            ConsumerGroupActionAPI(pulp_connection),
            ConsumerGroupBindAPI(pulp_connection),
            ConsumerGroupContentAPI(pulp_connection),
            ConsumerHistoryAPI(pulp_connection),
            ConsumerSearchAPI(pulp_connection),
            OrphanContentAPI(pulp_connection),
            ContentSourceAPI(pulp_connection),
            ContentCatalogAPI(pulp_connection),
            EventListenerAPI(pulp_connection),
            PermissionAPI(pulp_connection),
            RepositoryAPI(pulp_connection),
            RepositoryActionsAPI(pulp_connection),
            RepositoryDistributorAPI(pulp_connection),
            RepoGroupAPI(pulp_connection),
            RepoGroupActionAPI(pulp_connection),
            RepoGroupDistributorAPI(pulp_connection),
            RepoGroupSearchAPI(pulp_connection),
            RepoGroupSearchAPI(pulp_connection),
            RepositoryHistoryAPI(pulp_connection),
            RepositoryImporterAPI(pulp_connection),
            RepositoryPublishSchedulesAPI(pulp_connection),
            RepositorySearchAPI(pulp_connection),
            RepositorySyncSchedulesAPI(pulp_connection),
            RepositoryUnitAPI(pulp_connection),
            RoleAPI(pulp_connection),
            ServerInfoAPI(pulp_connection),
            StaticRequest(pulp_connection),
            TasksAPI(pulp_connection),
            TaskSearchAPI(pulp_connection),
            UploadAPI(pulp_connection),
            UserAPI(pulp_connection),
            UserSearchAPI(pulp_connection)
        ]

        self.actions, self.bind, self.bindings, self.profile, self.consumer, \
        self.consumer_content, self.consumer_content_schedules, self.consumer_group, \
        self.consumer_group_search, self.consumer_group_actions, self.consumer_group_bind, \
        self.consumer_group_content, self.consumer_history, self.consumer_search, \
        self.content_orphan, self.content_source, self.content_catalog, self.event_listener, \
        self.permission, self.repo, self.repo_actions, self.repo_distributor, self.repo_group, \
        self.repo_group_actions, self.repo_group_distributor, self.repo_group_distributor_search, \
        self.repo_group_search, self.repo_history, self.repo_importer, self.repo_publish_schedules, \
        self.repo_search, self.repo_sync_schedules, self.repo_unit, self.role, self.server_info, \
        self.static, self.tasks, self.tasks_search, self.uploads, self.user, self.user_search = _api_objects