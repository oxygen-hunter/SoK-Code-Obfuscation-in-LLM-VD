from enum import Enum
from typing import Dict, List, Optional

from fides.common.api.scope_registry import (
    CLI_OBJECTS_READ,
    CLIENT_READ,
    CONFIG_READ,
    CONFIG_UPDATE,
    CONNECTION_READ,
    CONNECTION_TYPE_READ,
    CONNECTOR_TEMPLATE_REGISTER,
    CONSENT_READ,
    CONSENT_SETTINGS_READ,
    CTL_DATASET_READ,
    CTL_POLICY_READ,
    DATA_CATEGORY_READ,
    DATA_QUALIFIER_READ,
    DATA_SUBJECT_READ,
    DATA_USE_READ,
    DATASET_READ,
    EVALUATION_READ,
    MASKING_EXEC,
    MASKING_READ,
    MESSAGING_CREATE_OR_UPDATE,
    MESSAGING_DELETE,
    MESSAGING_READ,
    ORGANIZATION_READ,
    POLICY_READ,
    PRIVACY_EXPERIENCE_READ,
    PRIVACY_NOTICE_READ,
    PRIVACY_REQUEST_CALLBACK_RESUME,
    PRIVACY_REQUEST_NOTIFICATIONS_CREATE_OR_UPDATE,
    PRIVACY_REQUEST_NOTIFICATIONS_READ,
    PRIVACY_REQUEST_READ,
    PRIVACY_REQUEST_REVIEW,
    PRIVACY_REQUEST_UPLOAD_DATA,
    PRIVACY_REQUEST_VIEW_DATA,
    REGISTRY_READ,
    RULE_READ,
    SAAS_CONFIG_READ,
    SCOPE_READ,
    SCOPE_REGISTRY,
    STORAGE_CREATE_OR_UPDATE,
    STORAGE_DELETE,
    STORAGE_READ,
    SYSTEM_MANAGER_READ,
    SYSTEM_READ,
    USER_PERMISSION_ASSIGN_OWNERS,
    USER_READ,
    WEBHOOK_READ,
)

APPROVER = "approver"
CONTRIBUTOR = "contributor"
OWNER = "owner"
VIEWER = "viewer"
VIEWER_AND_APPROVER = "viewer_and_approver"

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.running = True
        self.instructions = {
            'PUSH': self.push,
            'POP': self.pop,
            'ADD': self.add,
            'SUB': self.sub,
            'JZ': self.jz,
            'JMP': self.jmp,
            'LOAD': self.load,
            'STORE': self.store,
            'HALT': self.halt
        }
        
    def run(self, program):
        while self.running:
            op, *args = program[self.pc]
            self.instructions[op](*args)
            self.pc += 1

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        if self.stack:
            return self.stack.pop()

    def add(self):
        b, a = self.pop(), self.pop()
        self.push(a + b)

    def sub(self):
        b, a = self.pop(), self.pop()
        self.push(a - b)

    def jz(self, addr):
        if self.pop() == 0:
            self.pc = addr - 1

    def jmp(self, addr):
        self.pc = addr - 1

    def load(self, value):
        self.push(value)

    def store(self, index):
        value = self.pop()
        self.stack[index] = value

    def halt(self):
        self.running = False

class RoleRegistryEnum(Enum):
    owner = OWNER
    viewer_approver = VIEWER_AND_APPROVER
    viewer = VIEWER
    approver = APPROVER
    contributor = CONTRIBUTOR

approver_scopes = [
    PRIVACY_REQUEST_REVIEW,
    PRIVACY_REQUEST_READ,
    PRIVACY_REQUEST_CALLBACK_RESUME,
    PRIVACY_REQUEST_UPLOAD_DATA,
    PRIVACY_REQUEST_VIEW_DATA,
]

viewer_scopes = [
    CLI_OBJECTS_READ,
    CLIENT_READ,
    CONNECTION_READ,
    CONSENT_READ,
    CONSENT_SETTINGS_READ,
    CONNECTION_TYPE_READ,
    CTL_DATASET_READ,
    DATA_CATEGORY_READ,
    CTL_POLICY_READ,
    DATA_QUALIFIER_READ,
    DATASET_READ,
    DATA_SUBJECT_READ,
    DATA_USE_READ,
    EVALUATION_READ,
    MASKING_EXEC,
    MASKING_READ,
    ORGANIZATION_READ,
    POLICY_READ,
    PRIVACY_EXPERIENCE_READ,
    PRIVACY_NOTICE_READ,
    PRIVACY_REQUEST_READ,
    PRIVACY_REQUEST_NOTIFICATIONS_READ,
    REGISTRY_READ,
    RULE_READ,
    SCOPE_READ,
    STORAGE_READ,
    SYSTEM_READ,
    MESSAGING_READ,
    WEBHOOK_READ,
    SYSTEM_MANAGER_READ,
    SAAS_CONFIG_READ,
    USER_READ,
]

not_contributor_scopes = [
    CONNECTOR_TEMPLATE_REGISTER,
    STORAGE_CREATE_OR_UPDATE,
    STORAGE_DELETE,
    MESSAGING_CREATE_OR_UPDATE,
    MESSAGING_DELETE,
    PRIVACY_REQUEST_NOTIFICATIONS_CREATE_OR_UPDATE,
    CONFIG_READ,
    CONFIG_UPDATE,
    USER_PERMISSION_ASSIGN_OWNERS,
]

ROLES_TO_SCOPES_MAPPING: Dict[str, List] = {
    OWNER: sorted(SCOPE_REGISTRY),
    VIEWER_AND_APPROVER: sorted(list(set(viewer_scopes + approver_scopes))),
    VIEWER: sorted(viewer_scopes),
    APPROVER: sorted(approver_scopes),
    CONTRIBUTOR: sorted(list(set(SCOPE_REGISTRY) - set(not_contributor_scopes))),
}

def get_scopes_from_roles(roles: Optional[List[str]]) -> List[str]:
    vm = VM()
    program = [
        ('PUSH', []),
        ('STORE', 0),
        ('LOAD', roles),
        ('JZ', 7),
        ('LOAD', []),
        ('STORE', 1),
        ('LOAD', 0),
        ('HALT',)
    ]
    
    vm.run(program)
    if not vm.stack[0]:
        return []

    scope_list: List[str] = []
    for role in roles:
        scope_list += ROLES_TO_SCOPES_MAPPING.get(role, [])
    return [*set(scope_list)]