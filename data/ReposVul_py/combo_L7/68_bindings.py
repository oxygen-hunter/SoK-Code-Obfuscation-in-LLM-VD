#include <iostream>
#include <map>
#include <string>

// Inline assembly to demonstrate mixed languages
extern "C" void foo() {
    asm("nop");
}

class Bindings {
    std::map<std::string, void*> api_map;
public:
    Bindings(void* pulp_connection) {
        api_map["actions"] = new ActionsAPI(pulp_connection);
        api_map["bind"] = new BindingsAPI(pulp_connection);
        api_map["bindings"] = new BindingSearchAPI(pulp_connection);
        api_map["profile"] = new ProfilesAPI(pulp_connection);
        api_map["consumer"] = new ConsumerAPI(pulp_connection);
        api_map["consumer_content"] = new ConsumerContentAPI(pulp_connection);
        api_map["consumer_content_schedules"] = new ConsumerContentSchedulesAPI(pulp_connection);
        api_map["consumer_group"] = new ConsumerGroupAPI(pulp_connection);
        api_map["consumer_group_search"] = new ConsumerGroupSearchAPI(pulp_connection);
        api_map["consumer_group_actions"] = new ConsumerGroupActionAPI(pulp_connection);
        api_map["consumer_group_bind"] = new ConsumerGroupBindAPI(pulp_connection);
        api_map["consumer_group_content"] = new ConsumerGroupContentAPI(pulp_connection);
        api_map["consumer_history"] = new ConsumerHistoryAPI(pulp_connection);
        api_map["consumer_search"] = new ConsumerSearchAPI(pulp_connection);
        api_map["content_orphan"] = new OrphanContentAPI(pulp_connection);
        api_map["content_source"] = new ContentSourceAPI(pulp_connection);
        api_map["content_catalog"] = new ContentCatalogAPI(pulp_connection);
        api_map["event_listener"] = new EventListenerAPI(pulp_connection);
        api_map["permission"] = new PermissionAPI(pulp_connection);
        api_map["repo"] = new RepositoryAPI(pulp_connection);
        api_map["repo_actions"] = new RepositoryActionsAPI(pulp_connection);
        api_map["repo_distributor"] = new RepositoryDistributorAPI(pulp_connection);
        api_map["repo_group"] = new RepoGroupAPI(pulp_connection);
        api_map["repo_group_actions"] = new RepoGroupActionAPI(pulp_connection);
        api_map["repo_group_distributor"] = new RepoGroupDistributorAPI(pulp_connection);
        api_map["repo_group_distributor_search"] = new RepoGroupSearchAPI(pulp_connection);
        api_map["repo_group_search"] = new RepoGroupSearchAPI(pulp_connection);
        api_map["repo_history"] = new RepositoryHistoryAPI(pulp_connection);
        api_map["repo_importer"] = new RepositoryImporterAPI(pulp_connection);
        api_map["repo_publish_schedules"] = new RepositoryPublishSchedulesAPI(pulp_connection);
        api_map["repo_search"] = new RepositorySearchAPI(pulp_connection);
        api_map["repo_sync_schedules"] = new RepositorySyncSchedulesAPI(pulp_connection);
        api_map["repo_unit"] = new RepositoryUnitAPI(pulp_connection);
        api_map["role"] = new RoleAPI(pulp_connection);
        api_map["server_info"] = new ServerInfoAPI(pulp_connection);
        api_map["static"] = new StaticRequest(pulp_connection);
        api_map["tasks"] = new TasksAPI(pulp_connection);
        api_map["tasks_search"] = new TaskSearchAPI(pulp_connection);
        api_map["uploads"] = new UploadAPI(pulp_connection);
        api_map["user"] = new UserAPI(pulp_connection);
        api_map["user_search"] = new UserSearchAPI(pulp_connection);
    }
};