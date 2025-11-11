# -*- coding: utf-8 -*-

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.memory = {}

    def run(self, bytecode):
        while self.pc < len(bytecode):
            instruction = bytecode[self.pc]
            if instruction[0] == 'PUSH':
                self.stack.append(instruction[1])
            elif instruction[0] == 'POP':
                self.stack.pop()
            elif instruction[0] == 'LOAD':
                self.stack.append(self.memory[instruction[1]])
            elif instruction[0] == 'STORE':
                self.memory[instruction[1]] = self.stack.pop()
            elif instruction[0] == 'JMP':
                self.pc = instruction[1] - 1
            elif instruction[0] == 'JZ':
                if self.stack.pop() == 0:
                    self.pc = instruction[1] - 1
            elif instruction[0] == 'ADD':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif instruction[0] == 'SUB':
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            self.pc += 1

class Bindings:
    def __init__(self, pulp_connection):
        vm = VM()
        bytecode = [
            ('PUSH', pulp_connection),
            ('STORE', 'conn'),

            ('LOAD', 'conn'),
            ('PUSH', ActionsAPI),
            ('STORE', 'actions'),

            ('LOAD', 'conn'),
            ('PUSH', BindingsAPI),
            ('STORE', 'bind'),

            ('LOAD', 'conn'),
            ('PUSH', BindingSearchAPI),
            ('STORE', 'bindings'),

            ('LOAD', 'conn'),
            ('PUSH', ProfilesAPI),
            ('STORE', 'profile'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerAPI),
            ('STORE', 'consumer'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerContentAPI),
            ('STORE', 'consumer_content'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerContentSchedulesAPI),
            ('STORE', 'consumer_content_schedules'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerGroupAPI),
            ('STORE', 'consumer_group'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerGroupSearchAPI),
            ('STORE', 'consumer_group_search'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerGroupActionAPI),
            ('STORE', 'consumer_group_actions'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerGroupBindAPI),
            ('STORE', 'consumer_group_bind'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerGroupContentAPI),
            ('STORE', 'consumer_group_content'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerHistoryAPI),
            ('STORE', 'consumer_history'),

            ('LOAD', 'conn'),
            ('PUSH', ConsumerSearchAPI),
            ('STORE', 'consumer_search'),

            ('LOAD', 'conn'),
            ('PUSH', OrphanContentAPI),
            ('STORE', 'content_orphan'),

            ('LOAD', 'conn'),
            ('PUSH', ContentSourceAPI),
            ('STORE', 'content_source'),

            ('LOAD', 'conn'),
            ('PUSH', ContentCatalogAPI),
            ('STORE', 'content_catalog'),

            ('LOAD', 'conn'),
            ('PUSH', EventListenerAPI),
            ('STORE', 'event_listener'),

            ('LOAD', 'conn'),
            ('PUSH', PermissionAPI),
            ('STORE', 'permission'),

            ('LOAD', 'conn'),
            ('PUSH', RepositoryAPI),
            ('STORE', 'repo'),

            ('LOAD', 'conn'),
            ('PUSH', RepositoryActionsAPI),
            ('STORE', 'repo_actions'),

            ('LOAD', 'conn'),
            ('PUSH', RepositoryDistributorAPI),
            ('STORE', 'repo_distributor'),

            ('LOAD', 'conn'),
            ('PUSH', RepoGroupAPI),
            ('STORE', 'repo_group'),

            ('LOAD', 'conn'),
            ('PUSH', RepoGroupActionAPI),
            ('STORE', 'repo_group_actions'),

            ('LOAD', 'conn'),
            ('PUSH', RepoGroupDistributorAPI),
            ('STORE', 'repo_group_distributor'),

            ('LOAD', 'conn'),
            ('PUSH', RepoGroupSearchAPI),
            ('STORE', 'repo_group_distributor_search'),

            ('LOAD', 'conn'),
            ('PUSH', RepoGroupSearchAPI),
            ('STORE', 'repo_group_search'),

            ('LOAD', 'conn'),
            ('PUSH', RepositoryHistoryAPI),
            ('STORE', 'repo_history'),

            ('LOAD', 'conn'),
            ('PUSH', RepositoryImporterAPI),
            ('STORE', 'repo_importer'),

            ('LOAD', 'conn'),
            ('PUSH', RepositoryPublishSchedulesAPI),
            ('STORE', 'repo_publish_schedules'),

            ('LOAD', 'conn'),
            ('PUSH', RepositorySearchAPI),
            ('STORE', 'repo_search'),

            ('LOAD', 'conn'),
            ('PUSH', RepositorySyncSchedulesAPI),
            ('STORE', 'repo_sync_schedules'),

            ('LOAD', 'conn'),
            ('PUSH', RepositoryUnitAPI),
            ('STORE', 'repo_unit'),

            ('LOAD', 'conn'),
            ('PUSH', RoleAPI),
            ('STORE', 'role'),

            ('LOAD', 'conn'),
            ('PUSH', ServerInfoAPI),
            ('STORE', 'server_info'),

            ('LOAD', 'conn'),
            ('PUSH', StaticRequest),
            ('STORE', 'static'),

            ('LOAD', 'conn'),
            ('PUSH', TasksAPI),
            ('STORE', 'tasks'),

            ('LOAD', 'conn'),
            ('PUSH', TaskSearchAPI),
            ('STORE', 'tasks_search'),

            ('LOAD', 'conn'),
            ('PUSH', UploadAPI),
            ('STORE', 'uploads'),

            ('LOAD', 'conn'),
            ('PUSH', UserAPI),
            ('STORE', 'user'),

            ('LOAD', 'conn'),
            ('PUSH', UserSearchAPI),
            ('STORE', 'user_search')
        ]
        vm.run(bytecode)
        
        self.actions = vm.memory['actions']
        self.bind = vm.memory['bind']
        self.bindings = vm.memory['bindings']
        self.profile = vm.memory['profile']
        self.consumer = vm.memory['consumer']
        self.consumer_content = vm.memory['consumer_content']
        self.consumer_content_schedules = vm.memory['consumer_content_schedules']
        self.consumer_group = vm.memory['consumer_group']
        self.consumer_group_search = vm.memory['consumer_group_search']
        self.consumer_group_actions = vm.memory['consumer_group_actions']
        self.consumer_group_bind = vm.memory['consumer_group_bind']
        self.consumer_group_content = vm.memory['consumer_group_content']
        self.consumer_history = vm.memory['consumer_history']
        self.consumer_search = vm.memory['consumer_search']
        self.content_orphan = vm.memory['content_orphan']
        self.content_source = vm.memory['content_source']
        self.content_catalog = vm.memory['content_catalog']
        self.event_listener = vm.memory['event_listener']
        self.permission = vm.memory['permission']
        self.repo = vm.memory['repo']
        self.repo_actions = vm.memory['repo_actions']
        self.repo_distributor = vm.memory['repo_distributor']
        self.repo_group = vm.memory['repo_group']
        self.repo_group_actions = vm.memory['repo_group_actions']
        self.repo_group_distributor = vm.memory['repo_group_distributor']
        self.repo_group_distributor_search = vm.memory['repo_group_distributor_search']
        self.repo_group_search = vm.memory['repo_group_search']
        self.repo_history = vm.memory['repo_history']
        self.repo_importer = vm.memory['repo_importer']
        self.repo_publish_schedules = vm.memory['repo_publish_schedules']
        self.repo_search = vm.memory['repo_search']
        self.repo_sync_schedules = vm.memory['repo_sync_schedules']
        self.repo_unit = vm.memory['repo_unit']
        self.role = vm.memory['role']
        self.server_info = vm.memory['server_info']
        self.static = vm.memory['static']
        self.tasks = vm.memory['tasks']
        self.tasks_search = vm.memory['tasks_search']
        self.uploads = vm.memory['uploads']
        self.user = vm.memory['user']
        self.user_search = vm.memory['user_search']