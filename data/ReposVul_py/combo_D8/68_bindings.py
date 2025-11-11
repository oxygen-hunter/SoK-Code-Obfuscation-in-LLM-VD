# -*- coding: utf-8 -*-

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

        def createActionsAPI():
            return ActionsAPI(pulp_connection)
        
        def createBind():
            return BindingsAPI(pulp_connection)
        
        def createBindings():
            return BindingSearchAPI(pulp_connection)
        
        def createProfile():
            return ProfilesAPI(pulp_connection)
        
        def createConsumer():
            return ConsumerAPI(pulp_connection)
        
        def createConsumerContent():
            return ConsumerContentAPI(pulp_connection)
        
        def createConsumerContentSchedules():
            return ConsumerContentSchedulesAPI(pulp_connection)
        
        def createConsumerGroup():
            return ConsumerGroupAPI(pulp_connection)
        
        def createConsumerGroupSearch():
            return ConsumerGroupSearchAPI(pulp_connection)
        
        def createConsumerGroupActions():
            return ConsumerGroupActionAPI(pulp_connection)
        
        def createConsumerGroupBind():
            return ConsumerGroupBindAPI(pulp_connection)
        
        def createConsumerGroupContent():
            return ConsumerGroupContentAPI(pulp_connection)
        
        def createConsumerHistory():
            return ConsumerHistoryAPI(pulp_connection)
        
        def createConsumerSearch():
            return ConsumerSearchAPI(pulp_connection)
        
        def createContentOrphan():
            return OrphanContentAPI(pulp_connection)
        
        def createContentSource():
            return ContentSourceAPI(pulp_connection)
        
        def createContentCatalog():
            return ContentCatalogAPI(pulp_connection)
        
        def createEventListener():
            return EventListenerAPI(pulp_connection)
        
        def createPermission():
            return PermissionAPI(pulp_connection)
        
        def createRepo():
            return RepositoryAPI(pulp_connection)
        
        def createRepoActions():
            return RepositoryActionsAPI(pulp_connection)
        
        def createRepoDistributor():
            return RepositoryDistributorAPI(pulp_connection)
        
        def createRepoGroup():
            return RepoGroupAPI(pulp_connection)
        
        def createRepoGroupActions():
            return RepoGroupActionAPI(pulp_connection)
        
        def createRepoGroupDistributor():
            return RepoGroupDistributorAPI(pulp_connection)
        
        def createRepoGroupDistributorSearch():
            return RepoGroupSearchAPI(pulp_connection)
        
        def createRepoGroupSearch():
            return RepoGroupSearchAPI(pulp_connection)
        
        def createRepoHistory():
            return RepositoryHistoryAPI(pulp_connection)
        
        def createRepoImporter():
            return RepositoryImporterAPI(pulp_connection)
        
        def createRepoPublishSchedules():
            return RepositoryPublishSchedulesAPI(pulp_connection)
        
        def createRepoSearch():
            return RepositorySearchAPI(pulp_connection)
        
        def createRepoSyncSchedules():
            return RepositorySyncSchedulesAPI(pulp_connection)
        
        def createRepoUnit():
            return RepositoryUnitAPI(pulp_connection)
        
        def createRole():
            return RoleAPI(pulp_connection)
        
        def createServerInfo():
            return ServerInfoAPI(pulp_connection)
        
        def createStatic():
            return StaticRequest(pulp_connection)
        
        def createTasks():
            return TasksAPI(pulp_connection)
        
        def createTasksSearch():
            return TaskSearchAPI(pulp_connection)
        
        def createUploads():
            return UploadAPI(pulp_connection)
        
        def createUser():
            return UserAPI(pulp_connection)
        
        def createUserSearch():
            return UserSearchAPI(pulp_connection)

        self.actions = createActionsAPI()
        self.bind = createBind()
        self.bindings = createBindings()
        self.profile = createProfile()
        self.consumer = createConsumer()
        self.consumer_content = createConsumerContent()
        self.consumer_content_schedules = createConsumerContentSchedules()
        self.consumer_group = createConsumerGroup()
        self.consumer_group_search = createConsumerGroupSearch()
        self.consumer_group_actions = createConsumerGroupActions()
        self.consumer_group_bind = createConsumerGroupBind()
        self.consumer_group_content = createConsumerGroupContent()
        self.consumer_history = createConsumerHistory()
        self.consumer_search = createConsumerSearch()
        self.content_orphan = createContentOrphan()
        self.content_source = createContentSource()
        self.content_catalog = createContentCatalog()
        self.event_listener = createEventListener()
        self.permission = createPermission()
        self.repo = createRepo()
        self.repo_actions = createRepoActions()
        self.repo_distributor = createRepoDistributor()
        self.repo_group = createRepoGroup()
        self.repo_group_actions = createRepoGroupActions()
        self.repo_group_distributor = createRepoGroupDistributor()
        self.repo_group_distributor_search = createRepoGroupDistributorSearch()
        self.repo_group_search = createRepoGroupSearch()
        self.repo_history = createRepoHistory()
        self.repo_importer = createRepoImporter()
        self.repo_publish_schedules = createRepoPublishSchedules()
        self.repo_search = createRepoSearch()
        self.repo_sync_schedules = createRepoSyncSchedules()
        self.repo_unit = createRepoUnit()
        self.role = createRole()
        self.server_info = createServerInfo()
        self.static = createStatic()
        self.tasks = createTasks()
        self.tasks_search = createTasksSearch()
        self.uploads = createUploads()
        self.user = createUser()
        self.user_search = createUserSearch()