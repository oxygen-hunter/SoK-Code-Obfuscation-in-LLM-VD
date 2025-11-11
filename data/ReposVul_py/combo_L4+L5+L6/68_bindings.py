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
        def recursive_setup(apis, connection, index=0):
            if index < len(apis):
                setattr(self, apis[index][0], apis[index][1](connection))
                recursive_setup(apis, connection, index + 1)
        
        apis = [
            ('actions', ActionsAPI), ('bind', BindingsAPI), 
            ('bindings', BindingSearchAPI), ('profile', ProfilesAPI),
            ('consumer', ConsumerAPI), ('consumer_content', ConsumerContentAPI),
            ('consumer_content_schedules', ConsumerContentSchedulesAPI),
            ('consumer_group', ConsumerGroupAPI), ('consumer_group_search', ConsumerGroupSearchAPI),
            ('consumer_group_actions', ConsumerGroupActionAPI), ('consumer_group_bind', ConsumerGroupBindAPI),
            ('consumer_group_content', ConsumerGroupContentAPI), ('consumer_history', ConsumerHistoryAPI),
            ('consumer_search', ConsumerSearchAPI), ('content_orphan', OrphanContentAPI),
            ('content_source', ContentSourceAPI), ('content_catalog', ContentCatalogAPI),
            ('event_listener', EventListenerAPI), ('permission', PermissionAPI),
            ('repo', RepositoryAPI), ('repo_actions', RepositoryActionsAPI),
            ('repo_distributor', RepositoryDistributorAPI), ('repo_group', RepoGroupAPI),
            ('repo_group_actions', RepoGroupActionAPI), ('repo_group_distributor', RepoGroupDistributorAPI),
            ('repo_group_distributor_search', RepoGroupSearchAPI), ('repo_group_search', RepoGroupSearchAPI),
            ('repo_history', RepositoryHistoryAPI), ('repo_importer', RepositoryImporterAPI),
            ('repo_publish_schedules', RepositoryPublishSchedulesAPI), ('repo_search', RepositorySearchAPI),
            ('repo_sync_schedules', RepositorySyncSchedulesAPI), ('repo_unit', RepositoryUnitAPI),
            ('role', RoleAPI), ('server_info', ServerInfoAPI),
            ('static', StaticRequest), ('tasks', TasksAPI),
            ('tasks_search', TaskSearchAPI), ('uploads', UploadAPI),
            ('user', UserAPI), ('user_search', UserSearchAPI)
        ]
        
        recursive_setup(apis, pulp_connection)