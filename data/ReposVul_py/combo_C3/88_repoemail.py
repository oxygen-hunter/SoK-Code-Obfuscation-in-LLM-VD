import logging
from flask import request, abort
from endpoints.api import (
    resource,
    nickname,
    require_repo_admin,
    RepositoryParamResource,
    log_action,
    validate_json_request,
    internal_only,
    path_param,
    show_if,
)
from endpoints.api.repoemail_models_pre_oci import pre_oci_model as model
from endpoints.exception import NotFound
from app import tf
from data.database import db
from util.useremails import send_repo_authorization_email
import features

logger = logging.getLogger(__name__)

class VM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.instructions = []
        self.registers = {}

    def load(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.program_counter < len(self.instructions):
            instr, *args = self.instructions[self.program_counter]
            getattr(self, f"op_{instr}")(*args)
            self.program_counter += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        return self.stack.pop()

    def op_ADD(self):
        self.stack.append(self.stack.pop() + self.stack.pop())

    def op_SUB(self):
        b, a = self.stack.pop(), self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, target):
        self.program_counter = target - 1

    def op_JZ(self, target):
        if self.stack.pop() == 0:
            self.program_counter = target - 1

    def op_LOAD(self, var):
        self.stack.append(self.registers[var])

    def op_STORE(self, var):
        self.registers[var] = self.stack.pop()

    def op_CALL(self, func, *args):
        result = func(*args)
        if result is not None:
            self.stack.append(result)

@internal_only
@resource("/v1/repository/<apirepopath:repository>/authorizedemail/<email>")
@show_if(features.MAILING)
@path_param("repository", "The full path of the repository. e.g. namespace/name")
@path_param("email", "The e-mail address")
class RepositoryAuthorizedEmail(RepositoryParamResource):
    def vm_get(self, namespace, repository, email):
        vm = VM()
        instructions = [
            ('PUSH', namespace),
            ('PUSH', repository),
            ('PUSH', email),
            ('CALL', model.get_email_authorized_for_repo),
            ('STORE', 'record'),
            ('LOAD', 'record'),
            ('CALL', bool),
            ('JZ', 9),
            ('LOAD', 'record'),
            ('CALL', lambda rec: abort(404) if not rec else rec.to_dict()),
            ('STORE', 'response'),
            ('LOAD', 'response'),
            ('CALL', lambda resp: resp.pop("code", None)),
            ('LOAD', 'response')
        ]
        vm.load(instructions)
        vm.run()
        return vm.op_POP()

    def vm_post(self, namespace, repository, email):
        vm = VM()
        instructions = [
            ('PUSH', namespace),
            ('PUSH', repository),
            ('PUSH', email),
            ('CALL', model.get_email_authorized_for_repo),
            ('STORE', 'record'),
            ('LOAD', 'record'),
            ('CALL', lambda rec: rec and rec.confirmed),
            ('JZ', 14),
            ('LOAD', 'record'),
            ('CALL', lambda rec: rec.to_dict()),
            ('STORE', 'response'),
            ('LOAD', 'response'),
            ('CALL', lambda resp: resp.pop("code", None)),
            ('LOAD', 'response'),
            ('JMP', 28),
            ('LOAD', 'record'),
            ('CALL', bool),
            ('JZ', 20),
            ('PUSH', namespace),
            ('PUSH', repository),
            ('PUSH', email),
            ('CALL', model.create_email_authorization_for_repo),
            ('STORE', 'record'),
            ('LOAD', 'record'),
            ('CALL', lambda rec: send_repo_authorization_email(namespace, repository, email, rec.code)),
            ('LOAD', 'record'),
            ('CALL', lambda rec: rec.to_dict()),
            ('STORE', 'response'),
            ('LOAD', 'response'),
            ('CALL', lambda resp: resp.pop("code", None)),
            ('LOAD', 'response')
        ]
        vm.load(instructions)
        vm.run()
        return vm.op_POP()

    @require_repo_admin
    @nickname("checkRepoEmailAuthorized")
    def get(self, namespace, repository, email):
        return self.vm_get(namespace, repository, email)

    @require_repo_admin
    @nickname("sendAuthorizeRepoEmail")
    def post(self, namespace, repository, email):
        return self.vm_post(namespace, repository, email)