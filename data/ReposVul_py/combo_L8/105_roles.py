from ctypes import CDLL, c_char_p
from enum import Enum
from typing import Dict, List, Optional

lib = CDLL('./libscope.so')  # Load your C compiled shared library

APPROVER = c_char_p(b"approver").value.decode('utf-8')
CONTRIBUTOR = c_char_p(b"contributor").value.decode('utf-8')
OWNER = c_char_p(b"owner").value.decode('utf-8')
VIEWER = c_char_p(b"viewer").value.decode('utf-8')
VIEWER_AND_APPROVER = c_char_p(b"viewer_and_approver").value.decode('utf-8')


class RoleRegistryEnum(Enum):
    owner = OWNER
    viewer_approver = VIEWER_AND_APPROVER
    viewer = VIEWER
    approver = APPROVER
    contributor = CONTRIBUTOR


approver_scopes = [
    lib.PRIVACY_REQUEST_REVIEW,
    lib.PRIVACY_REQUEST_READ,
    lib.PRIVACY_REQUEST_CALLBACK_RESUME,
    lib.PRIVACY_REQUEST_UPLOAD_DATA,
    lib.PRIVACY_REQUEST_VIEW_DATA,
]

viewer_scopes = [
    lib.CLI_OBJECTS_READ,
    lib.CLIENT_READ,
    lib.CONNECTION_READ,
    lib.CONSENT_READ,
    lib.CONSENT_SETTINGS_READ,
    lib.CONNECTION_TYPE_READ,
    lib.CTL_DATASET_READ,
    lib.DATA_CATEGORY_READ,
    lib.CTL_POLICY_READ,
    lib.DATA_QUALIFIER_READ,
    lib.DATASET_READ,
    lib.DATA_SUBJECT_READ,
    lib.DATA_USE_READ,
    lib.EVALUATION_READ,
    lib.MASKING_EXEC,
    lib.MASKING_READ,
    lib.ORGANIZATION_READ,
    lib.POLICY_READ,
    lib.PRIVACY_EXPERIENCE_READ,
    lib.PRIVACY_NOTICE_READ,
    lib.PRIVACY_REQUEST_READ,
    lib.PRIVACY_REQUEST_NOTIFICATIONS_READ,
    lib.REGISTRY_READ,
    lib.RULE_READ,
    lib.SCOPE_READ,
    lib.STORAGE_READ,
    lib.SYSTEM_READ,
    lib.MESSAGING_READ,
    lib.WEBHOOK_READ,
    lib.SYSTEM_MANAGER_READ,
    lib.SAAS_CONFIG_READ,
    lib.USER_READ,
]

not_contributor_scopes = [
    lib.CONNECTOR_TEMPLATE_REGISTER,
    lib.STORAGE_CREATE_OR_UPDATE,
    lib.STORAGE_DELETE,
    lib.MESSAGING_CREATE_OR_UPDATE,
    lib.MESSAGING_DELETE,
    lib.PRIVACY_REQUEST_NOTIFICATIONS_CREATE_OR_UPDATE,
    lib.CONFIG_READ,
    lib.CONFIG_UPDATE,
    lib.USER_PERMISSION_ASSIGN_OWNERS,
]

ROLES_TO_SCOPES_MAPPING: Dict[str, List] = {
    OWNER: sorted(lib.SCOPE_REGISTRY),
    VIEWER_AND_APPROVER: sorted(list(set(viewer_scopes + approver_scopes))),
    VIEWER: sorted(viewer_scopes),
    APPROVER: sorted(approver_scopes),
    CONTRIBUTOR: sorted(list(set(lib.SCOPE_REGISTRY) - set(not_contributor_scopes))),
}


def get_scopes_from_roles(roles: Optional[List[str]]) -> List[str]:
    if not roles:
        return []

    scope_list: List[str] = []
    for role in roles:
        scope_list += ROLES_TO_SCOPES_MAPPING.get(role, [])
    return [*set(scope_list)]