from enum import Enum
from typing import List, Optional

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

_roles = ["approver", "contributor", "owner", "viewer", "viewer_and_approver"]
APPROVER, CONTRIBUTOR, OWNER, VIEWER, VIEWER_AND_APPROVER = _roles

class RoleRegistryEnum(Enum):
    owner = _roles[2]
    viewer_approver = _roles[4]
    viewer = _roles[3]
    approver = _roles[0]
    contributor = _roles[1]

_scope_sets = [
    [
        PRIVACY_REQUEST_REVIEW,
        PRIVACY_REQUEST_READ,
        PRIVACY_REQUEST_CALLBACK_RESUME,
        PRIVACY_REQUEST_UPLOAD_DATA,
        PRIVACY_REQUEST_VIEW_DATA,
    ],
    [
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
    ],
    [
        CONNECTOR_TEMPLATE_REGISTER,
        STORAGE_CREATE_OR_UPDATE,
        STORAGE_DELETE,
        MESSAGING_CREATE_OR_UPDATE,
        MESSAGING_DELETE,
        PRIVACY_REQUEST_NOTIFICATIONS_CREATE_OR_UPDATE,
        CONFIG_READ,
        CONFIG_UPDATE,
        USER_PERMISSION_ASSIGN_OWNERS,
    ],
]
approver_scopes, viewer_scopes, not_contributor_scopes = _scope_sets

ROLES_TO_SCOPES_MAPPING = {
    _roles[2]: sorted(SCOPE_REGISTRY),
    _roles[4]: sorted(list(set(_scope_sets[1] + _scope_sets[0]))),
    _roles[3]: sorted(_scope_sets[1]),
    _roles[0]: sorted(_scope_sets[0]),
    _roles[1]: sorted(list(set(SCOPE_REGISTRY) - set(_scope_sets[2]))),
}

def get_scopes_from_roles(r: Optional[List[str]]) -> List[str]:
    return [] if not r else [*set(sum((ROLES_TO_SCOPES_MAPPING.get(role, []) for role in r), []))]