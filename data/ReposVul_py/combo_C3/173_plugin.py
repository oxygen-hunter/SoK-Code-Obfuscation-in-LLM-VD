import base64
import json
import logging
import requests
from fastapi import HTTPException
from fastapi.security.utils import get_authorization_scheme_param
from jose import JWTError, jwt
from jose.exceptions import JWKError
from starlette.requests import Request
from starlette.status import HTTP_401_UNAUTHORIZED
from dispatch.case import service as case_service
from dispatch.config import (
    DISPATCH_AUTHENTICATION_PROVIDER_HEADER_NAME,
    DISPATCH_AUTHENTICATION_PROVIDER_PKCE_JWKS,
    DISPATCH_JWT_AUDIENCE,
    DISPATCH_JWT_EMAIL_OVERRIDE,
    DISPATCH_JWT_SECRET,
    DISPATCH_PKCE_DONT_VERIFY_AT_HASH,
    DISPATCH_UI_URL,
)
from dispatch.database.core import Base
from dispatch.document.models import Document, DocumentRead
from dispatch.incident import service as incident_service
from dispatch.incident.models import Incident
from dispatch.individual import service as individual_service
from dispatch.individual.models import IndividualContact, IndividualContactRead
from dispatch.plugin import service as plugin_service
from dispatch.plugins import dispatch_core as dispatch_plugin
from dispatch.plugins.bases import (
    AuthenticationProviderPlugin,
    ContactPlugin,
    DocumentResolverPlugin,
    ParticipantPlugin,
    TicketPlugin,
)
from dispatch.route import service as route_service
from dispatch.service import service as service_service
from dispatch.service.models import Service, ServiceRead
from dispatch.team import service as team_service
from dispatch.team.models import TeamContact, TeamContactRead

log = logging.getLogger(__name__)

# VM Instruction Set
PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT, LOG, EXC, SPLT, JNE, GET, SET = range(15)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.memory = {}
        self.running = True

    def run(self, instructions):
        self.instructions = instructions
        self.pc = 0
        self.running = True
        while self.running:
            self.dispatch()

    def dispatch(self):
        inst = self.instructions[self.pc]
        if inst[0] == PUSH:
            self.stack.append(inst[1])
        elif inst[0] == POP:
            self.stack.pop()
        elif inst[0] == ADD:
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(a + b)
        elif inst[0] == SUB:
            a = self.stack.pop()
            b = self.stack.pop()
            self.stack.append(b - a)
        elif inst[0] == JMP:
            self.pc = inst[1] - 1
        elif inst[0] == JZ:
            if self.stack.pop() == 0:
                self.pc = inst[1] - 1
        elif inst[0] == LOAD:
            self.stack.append(self.memory[inst[1]])
        elif inst[0] == STORE:
            self.memory[inst[1]] = self.stack.pop()
        elif inst[0] == HALT:
            self.running = False
        elif inst[0] == LOG:
            log_method = inst[1]
            message = inst[2]
            if log_method == 'exception':
                log.exception(message)
            elif log_method == 'error':
                log.error(message)
            elif log_method == 'debug':
                log.debug(message)
        elif inst[0] == EXC:
            raise inst[1]
        elif inst[0] == SPLT:
            authorization = self.stack.pop()
            self.stack.append(authorization.split()[1])
        elif inst[0] == JNE:
            if self.stack.pop() != 0:
                self.pc = inst[1] - 1
        elif inst[0] == GET:
            request = self.stack.pop()
            header_key = inst[1]
            self.stack.append(request.headers.get(header_key))
        elif inst[0] == SET:
            self.memory[inst[1]] = self.stack.pop()
        self.pc += 1

class BasicAuthProviderPlugin(AuthenticationProviderPlugin):
    title = "Dispatch Plugin - Basic Authentication Provider"
    slug = "dispatch-auth-provider-basic"
    description = "Generic basic authentication provider."
    version = dispatch_plugin.__version__

    author = "Netflix"
    author_url = "https://github.com/netflix/dispatch.git"

    def get_current_user(self, request: Request, **kwargs):
        vm = VM()
        instructions = [
            (GET, "Authorization"), (SPLT,), (PUSH, "bearer"), (JNE, 8),
            (LOG, "exception", "Malformed authorization header."), (HALT,),
            (EXC, HTTPException(status_code=HTTP_401_UNAUTHORIZED, detail=[{"msg": "Could not validate credentials"}])),
            (STORE, "token"), (LOAD, "token"), (PUSH, DISPATCH_JWT_SECRET), 
            (EXC, jwt.decode), (STORE, "data"), (LOAD, "data"), (PUSH, "email"), 
            (EXC, lambda data, key: data[key]), (HALT,)
        ]
        try:
            vm.run(instructions)
            return vm.memory["email"]
        except (JWKError, JWTError):
            raise HTTPException(
                status_code=HTTP_401_UNAUTHORIZED,
                detail=[{"msg": "Could not validate credentials"}],
            ) from None

class PKCEAuthProviderPlugin(AuthenticationProviderPlugin):
    title = "Dispatch Plugin - PKCE Authentication Provider"
    slug = "dispatch-auth-provider-pkce"
    description = "Generic PCKE authentication provider."
    version = dispatch_plugin.__version__

    author = "Netflix"
    author_url = "https://github.com/netflix/dispatch.git"

    def get_current_user(self, request: Request, **kwargs):
        vm = VM()
        instructions = [
            (GET, "Authorization"), (PUSH, "bearer"), (JNE, 7),
            (EXC, HTTPException(status_code=HTTP_401_UNAUTHORIZED, detail=[{"msg": "Could not validate credentials"}])),
            (SPLT,), (STORE, "token"), 
            (LOAD, "token"), (PUSH, "========="), (ADD,), (EXC, base64.b64decode),
            (EXC, lambda token: json.loads(token.decode("utf-8"))), (STORE, "key_info"),
            (EXC, lambda: requests.get(DISPATCH_AUTHENTICATION_PROVIDER_PKCE_JWKS).json()["keys"]), (STORE, "keys"),
            (LOAD, "keys"), (LOAD, "key_info"), (PUSH, "kid"), (EXC, lambda keys, key_info, key: next(k for k in keys if k["kid"] == key_info[key])), 
            (STORE, "key"), (LOAD, "token"), (LOAD, "key"), (PUSH, DISPATCH_JWT_AUDIENCE), 
            (PUSH, DISPATCH_PKCE_DONT_VERIFY_AT_HASH), (PUSH, False), (JNE, 35), 
            (STORE, "jwt_opts"), (LOAD, "token"), (LOAD, "key"), (LOAD, "jwt_opts"),
            (JNE, 39), (EXC, lambda token, key, opts: jwt.decode(token, key, options=opts)), 
            (STORE, "data"), (LOAD, "data"), (PUSH, "email"), 
            (EXC, lambda data, key: data[key]), (HALT,)
        ]
        vm.memory["jwt_opts"] = {}
        if DISPATCH_PKCE_DONT_VERIFY_AT_HASH:
            vm.memory["jwt_opts"] = {"verify_at_hash": False}
        try:
            vm.run(instructions)
            return vm.memory["email"]
        except JWTError as err:
            log.debug("JWT Decode error: {}".format(err))
            raise HTTPException(
                status_code=HTTP_401_UNAUTHORIZED,
                detail=[{"msg": "Could not validate credentials"}],
            ) from err

class HeaderAuthProviderPlugin(AuthenticationProviderPlugin):
    title = "Dispatch Plugin - HTTP Header Authentication Provider"
    slug = "dispatch-auth-provider-header"
    description = "Authenticate users based on HTTP request header."
    version = dispatch_plugin.__version__

    author = "Filippo Giunchedi"
    author_url = "https://github.com/filippog"

    def get_current_user(self, request: Request, **kwargs):
        vm = VM()
        instructions = [
            (GET, DISPATCH_AUTHENTICATION_PROVIDER_HEADER_NAME), (STORE, "value"),
            (LOAD, "value"), (JNE, 5),
            (LOG, "error", f"Unable to authenticate. Header {DISPATCH_AUTHENTICATION_PROVIDER_HEADER_NAME} not found."),
            (EXC, HTTPException(status_code=HTTP_401_UNAUTHORIZED)), (HALT,)
        ]
        vm.run(instructions)
        return vm.memory["value"]

class DispatchTicketPlugin(TicketPlugin):
    title = "Dispatch Plugin - Ticket Management"
    slug = "dispatch-ticket"
    description = "Uses Dispatch itself to create a ticket."
    version = dispatch_plugin.__version__

    author = "Netflix"
    author_url = "https://github.com/netflix/dispatch.git"

    def create(
        self,
        incident_id: int,
        title: str,
        commander_email: str,
        reporter_email: str,
        plugin_metadata: dict,
        db_session=None,
    ):
        incident = incident_service.get(db_session=db_session, incident_id=incident_id)
        vm = VM()
        resource_id = f"dispatch-{incident.project.organization.slug}-{incident.project.slug}-{incident.id}"
        instructions = [
            (PUSH, resource_id), (PUSH, f"{DISPATCH_UI_URL}/{incident.project.organization.name}/incidents/{resource_id}?project={incident.project.name}"),
            (PUSH, "dispatch-internal-ticket"), (STORE, "ticket"), (HALT,)
        ]
        vm.run(instructions)
        return {
            "resource_id": resource_id,
            "weblink": vm.memory["ticket"],
            "resource_type": "dispatch-internal-ticket",
        }

    def update(
        self,
        ticket_id: str,
        title: str,
        description: str,
        incident_type: str,
        incident_severity: str,
        incident_priority: str,
        status: str,
        commander_email: str,
        reporter_email: str,
        conversation_weblink: str,
        document_weblink: str,
        storage_weblink: str,
        conference_weblink: str,
        cost: float,
        incident_type_plugin_metadata: dict = None,
    ):
        return

    def delete(
        self,
        ticket_id: str,
    ):
        return

    def create_case_ticket(
        self,
        case_id: int,
        title: str,
        assignee_email: str,
        case_type_plugin_metadata: dict,
        db_session=None,
    ):
        case = case_service.get(db_session=db_session, case_id=case_id)
        vm = VM()
        resource_id = f"dispatch-{case.project.organization.slug}-{case.project.slug}-{case.id}"
        instructions = [
            (PUSH, resource_id), (PUSH, f"{DISPATCH_UI_URL}/{case.project.organization.name}/cases/{resource_id}?project={case.project.name}"),
            (PUSH, "dispatch-internal-ticket"), (STORE, "ticket"), (HALT,)
        ]
        vm.run(instructions)
        return {
            "resource_id": resource_id,
            "weblink": vm.memory["ticket"],
            "resource_type": "dispatch-internal-ticket",
        }

    def update_case_ticket(
        self,
        ticket_id: str,
        title: str,
        description: str,
        resolution: str,
        case_type: str,
        case_severity: str,
        case_priority: str,
        status: str,
        assignee_email: str,
        document_weblink: str,
        storage_weblink: str,
        case_type_plugin_metadata: dict = None,
    ):
        return

class DispatchDocumentResolverPlugin(DocumentResolverPlugin):
    title = "Dispatch Plugin - Document Resolver"
    slug = "dispatch-document-resolver"
    description = "Uses dispatch itself to resolve incident documents."
    version = dispatch_plugin.__version__

    author = "Netflix"
    author_url = "https://github.com/netflix/dispatch.git"

    def get(
        self,
        incident: Incident,
        db_session=None,
    ):
        vm = VM()
        instructions = [
            (PUSH, incident.project_id), (PUSH, incident), 
            (EXC, lambda proj_id, inst: route_service.get(db_session=db_session, project_id=proj_id, class_instance=inst, models=[(Document, DocumentRead)])),
            (STORE, "recommendation"), (LOAD, "recommendation"), (LOG, "debug", "Fetched recommendation"), (HALT,)
        ]
        vm.run(instructions)
        return vm.memory["recommendation"].matches

class DispatchContactPlugin(ContactPlugin):
    title = "Dispatch Plugin - Contact plugin"
    slug = "dispatch-contact"
    description = "Uses dispatch itself to fetch incident participants contact info."
    version = dispatch_plugin.__version__

    author = "Netflix"
    author_url = "https://github.com/netflix/dispatch.git"

    def get(self, email, db_session=None):
        vm = VM()
        instructions = [
            (PUSH, email), (PUSH, self.project_id),
            (EXC, lambda em, pid: individual_service.get_by_email_and_project(db_session=db_session, email=em, project_id=pid)),
            (STORE, "individual"), (LOAD, "individual"), (PUSH, email), (JNE, 8),
            (PUSH, {"email": email, "fullname": email}), (HALT,),
            (LOAD, "individual"), (EXC, lambda ind: ind.dict()), 
            (PUSH, "fullname"), (PUSH, "name"), (LOAD, "individual"), 
            (EXC, lambda i, k: i[k]), (SET, "fullname"), (HALT,)
        ]
        vm.run(instructions)
        return vm.memory["individual"]

class DispatchParticipantResolverPlugin(ParticipantPlugin):
    title = "Dispatch Plugin - Participant Resolver"
    slug = "dispatch-participant-resolver"
    description = "Uses dispatch itself to resolve incident participants."
    version = dispatch_plugin.__version__

    author = "Netflix"
    author_url = "https://github.com/netflix/dispatch.git"

    def get(
        self,
        project_id: int,
        class_instance: Base,
        db_session=None,
    ):
        vm = VM()
        instructions = [
            (PUSH, project_id), (PUSH, class_instance), 
            (EXC, lambda pid, ci: route_service.get(db_session=db_session, project_id=pid, class_instance=ci, 
                     models=[(IndividualContact, IndividualContactRead), (Service, ServiceRead), (TeamContact, TeamContactRead)])),
            (STORE, "recommendation"), (LOAD, "recommendation"), (LOG, "debug", "Fetched recommendation"),
            (STORE, "matches"), (EXC, individual_service.get_or_create), (STORE, "individual_contacts"),
            (EXC, team_service.get_or_create), (STORE, "team_contacts"), (HALT,)
        ]
        vm.run(instructions)
        return vm.memory["individual_contacts"], vm.memory["team_contacts"]