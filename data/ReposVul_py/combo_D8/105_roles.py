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

def getDynamicValueAPPROVER():
    return "approver"

def getDynamicValueCONTRIBUTOR():
    return "contributor"

def getDynamicValueOWNER():
    return "owner"

def getDynamicValueVIEWER():
    return "viewer"

def getDynamicValueVIEWER_AND_APPROVER():
    return "viewer_and_approver"

class RoleRegistryEnum(Enum):
    owner = getDynamicValueOWNER()
    viewer_approver = getDynamicValueVIEWER_AND_APPROVER()
    viewer = getDynamicValueVIEWER()
    approver = getDynamicValueAPPROVER()
    contributor = getDynamicValueCONTRIBUTOR()

def getApproverScopes():
    return [
        PRIVACY_REQUEST_REVIEW,
        PRIVACY_REQUEST_READ,
        PRIVACY_REQUEST_CALLBACK_RESUME,
        PRIVACY_REQUEST_UPLOAD_DATA,
        PRIVACY_REQUEST_VIEW_DATA,
    ]

def getViewerScopes():
    return [
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

def getNotContributorScopes():
    return [
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

def getRolesToScopesMapping():
    return {
        getDynamicValueOWNER(): sorted(SCOPE_REGISTRY),
        getDynamicValueVIEWER_AND_APPROVER(): sorted(list(set(getViewerScopes() + getApproverScopes()))),
        getDynamicValueVIEWER(): sorted(getViewerScopes()),
        getDynamicValueAPPROVER(): sorted(getApproverScopes()),
        getDynamicValueCONTRIBUTOR(): sorted(list(set(SCOPE_REGISTRY) - set(getNotContributorScopes()))),
    }

def get_scopes_from_roles(roles: Optional[List[str]]) -> List[str]:
    if not roles:
        return []
    scope_list: List[str] = []
    for role in roles:
        scope_list += getRolesToScopesMapping().get(role, [])
    return [*set(scope_list)]