import collections.abc
from oslo_policy import policy
import pecan
from webob import exc
from barbican import api
from barbican.common import accept
from barbican.common import utils
from barbican import i18n as u

LOG = utils.getLogger(__name__)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running and self.pc < len(self.instructions):
            instr, *args = self.instructions[self.pc]
            getattr(self, f'op_{instr}')(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.stack.pop()

    def op_JMP(self, address):
        self.pc = address - 1

    def op_JZ(self, address):
        if self.stack.pop() == 0:
            self.pc = address - 1

    def op_HALT(self):
        self.running = False

def is_json_request_accept(req):
    vm = VM()
    instructions = [
        ('PUSH', req.accept),
        ('LOAD', 0),
        ('PUSH', accept.NoHeaderType),
        ('LOAD', 1),
        ('PUSH', accept.ValidHeaderType),
        ('LOAD', 2),
        ('JZ', 10),
        ('STORE', 0),
        ('LOAD', 0),
        ('PUSH', 'application/json'),
        ('LOAD', 3),
        ('PUSH', '*/*'),
        ('ADD',),
        ('STORE', 1),
        ('HALT',)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return bool(vm.stack[0])

def _get_barbican_context(req):
    vm = VM()
    instructions = [
        ('PUSH', 'barbican.context'),
        ('LOAD', 0),
        ('PUSH', req.environ),
        ('LOAD', 1),
        ('JZ', 5),
        ('LOAD', 2),
        ('PUSH', req.environ['barbican.context']),
        ('LOAD', 3),
        ('HALT',)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.stack[0] if vm.stack else None

def _do_enforce_rbac(inst, req, action_name, ctx, **kwargs):
    vm = VM()
    instructions = [
        ('PUSH', action_name),
        ('LOAD', 0),
        ('PUSH', ctx),
        ('LOAD', 1),
        ('JZ', 17),
        ('PUSH', 'secret:get'),
        ('LOAD', 2),
        ('JZ', 10),
        ('PUSH', is_json_request_accept(req)),
        ('LOAD', 3),
        ('JZ', 10),
        ('PUSH', 'secret:decrypt'),
        ('LOAD', 4),
        ('STORE', 0),
        ('PUSH', inst),
        ('LOAD', 5),
        ('PUSH', req),
        ('LOAD', 6),
        ('PUSH', kwargs),
        ('LOAD', 7),
        ('PUSH', inst.get_acl_tuple(req, **kwargs)),
        ('LOAD', 8),
        ('STORE', 1),
        ('PUSH', {}),
        ('STORE', 2),
        ('PUSH', vm.stack[1]),
        ('LOAD', 9),
        ('PUSH', vm.stack[2]),
        ('LOAD', 10),
        ('JZ', 30),
        ('PUSH', 'target'),
        ('LOAD', 11),
        ('PUSH', {vm.stack[1]: vm.stack[2]}),
        ('LOAD', 12),
        ('STORE', 3),
        ('PUSH', vm.stack[3]),
        ('LOAD', 13),
        ('PUSH', kwargs),
        ('LOAD', 14),
        ('ADD',),
        ('STORE', 4),
        ('PUSH', ctx.policy_enforcer),
        ('LOAD', 15),
        ('JZ', 40),
        ('PUSH', ctx.policy_enforcer.authorize),
        ('LOAD', 16),
        ('PUSH', vm.stack[0]),
        ('LOAD', 17),
        ('PUSH', vm.stack[4]),
        ('LOAD', 18),
        ('PUSH', ctx),
        ('LOAD', 19),
        ('PUSH', True),
        ('LOAD', 20),
        ('HALT',)
    ]
    vm.load_instructions(instructions)
    vm.run()

def enforce_rbac(action_name='default'):
    def rbac_decorator(fn):
        def enforcer(inst, *args, **kwargs):
            vm = VM()
            instructions = [
                ('PUSH', _get_barbican_context(pecan.request)),
                ('LOAD', 0),
                ('STORE', 0),
                ('PUSH', None),
                ('STORE', 1),
                ('PUSH', vm.stack[0]),
                ('LOAD', 2),
                ('JZ', 10),
                ('PUSH', vm.stack[0].project_id),
                ('LOAD', 3),
                ('STORE', 1),
                ('PUSH', _do_enforce_rbac),
                ('LOAD', 4),
                ('PUSH', inst),
                ('LOAD', 5),
                ('PUSH', pecan.request),
                ('LOAD', 6),
                ('PUSH', action_name),
                ('LOAD', 7),
                ('PUSH', vm.stack[0]),
                ('LOAD', 8),
                ('PUSH', kwargs),
                ('LOAD', 9),
                ('PUSH', list(args)),
                ('LOAD', 10),
                ('STORE', 2),
                ('PUSH', 0),
                ('LOAD', 11),
                ('PUSH', vm.stack[1]),
                ('LOAD', 12),
                ('ADD',),
                ('STORE', 2),
                ('PUSH', fn),
                ('LOAD', 13),
                ('PUSH', inst),
                ('LOAD', 14),
                ('PUSH', vm.stack[2]),
                ('LOAD', 15),
                ('PUSH', kwargs),
                ('LOAD', 16),
                ('HALT',)
            ]
            vm.load_instructions(instructions)
            vm.run()
            return vm.stack[2][-1]

        return enforcer

    return rbac_decorator

def handle_exceptions(operation_name=u._('System')):
    def exceptions_decorator(fn):
        def handler(inst, *args, **kwargs):
            vm = VM()
            instructions = [
                ('PUSH', fn),
                ('LOAD', 0),
                ('PUSH', inst),
                ('LOAD', 1),
                ('PUSH', args),
                ('LOAD', 2),
                ('PUSH', kwargs),
                ('LOAD', 3),
                ('HALT',)
            ]
            try:
                vm.load_instructions(instructions)
                vm.run()
                return vm.stack[0][-1]
            except exc.HTTPError:
                LOG.exception('Webob error seen')
                raise
            except policy.PolicyNotAuthorized as pna:
                status, message = api.generate_safe_exception_message(
                    operation_name, pna)
                LOG.error(message)
                pecan.abort(status, message)
            except Exception as e:
                LOG.logger.disabled = False
                status, message = api.generate_safe_exception_message(
                    operation_name, e)
                LOG.exception(message)
                pecan.abort(status, message)

        return handler

    return exceptions_decorator

def _do_enforce_content_types(pecan_req, valid_content_types):
    vm = VM()
    instructions = [
        ('PUSH', pecan_req.content_type),
        ('LOAD', 0),
        ('PUSH', valid_content_types),
        ('LOAD', 1),
        ('JZ', 5),
        ('PUSH', u._(
            "Unexpected content type. Expected content types "
            "are: {expected}"
        ).format(
            expected=valid_content_types
        )),
        ('LOAD', 2),
        ('STORE', 0),
        ('PUSH', 415),
        ('LOAD', 3),
        ('PUSH', vm.stack[0]),
        ('LOAD', 4),
        ('HALT',)
    ]
    vm.load_instructions(instructions)
    vm.run()
    pecan.abort(vm.stack[1], vm.stack[0])

def enforce_content_types(valid_content_types=[]):
    def content_types_decorator(fn):
        def content_types_enforcer(inst, *args, **kwargs):
            vm = VM()
            instructions = [
                ('PUSH', _do_enforce_content_types),
                ('LOAD', 0),
                ('PUSH', pecan.request),
                ('LOAD', 1),
                ('PUSH', valid_content_types),
                ('LOAD', 2),
                ('HALT',)
            ]
            vm.load_instructions(instructions)
            vm.run()
            return fn(inst, *args, **kwargs)

        return content_types_enforcer

    return content_types_decorator

def flatten(d, parent_key=''):
    vm = VM()
    instructions = [
        ('PUSH', []),
        ('STORE', 0),
        ('PUSH', d.items()),
        ('LOAD', 1),
        ('JZ', 25),
        ('PUSH', vm.stack[1].pop(0)),
        ('LOAD', 2),
        ('STORE', 1),
        ('PUSH', parent_key + '.' + vm.stack[1][0] if parent_key else vm.stack[1][0]),
        ('LOAD', 3),
        ('STORE', 2),
        ('PUSH', isinstance(vm.stack[1][1], collections.abc.MutableMapping)),
        ('LOAD', 4),
        ('JZ', 21),
        ('PUSH', flatten(vm.stack[1][1], vm.stack[2]).items()),
        ('LOAD', 5),
        ('PUSH', vm.stack[0].extend),
        ('LOAD', 6),
        ('ADD',),
        ('STORE', 0),
        ('JMP', 10),
        ('PUSH', (vm.stack[2], vm.stack[1][1])),
        ('LOAD', 7),
        ('PUSH', vm.stack[0].append),
        ('LOAD', 8),
        ('ADD',),
        ('STORE', 0),
        ('JMP', 4),
        ('PUSH', dict(vm.stack[0])),
        ('LOAD', 9),
        ('HALT',)
    ]
    vm.load_instructions(instructions)
    vm.run()
    return vm.stack[0]

class ACLMixin(object):
    def get_acl_tuple(self, req, **kwargs):
        return None, None

    def get_acl_dict_for_user(self, req, acl_list):
        vm = VM()
        instructions = [
            ('PUSH', _get_barbican_context(req)),
            ('LOAD', 0),
            ('STORE', 0),
            ('PUSH', {}),
            ('STORE', 1),
            ('PUSH', vm.stack[0]),
            ('LOAD', 2),
            ('JZ', 30),
            ('PUSH', {acl.operation: acl.operation for acl in acl_list
                        if vm.stack[0].user in acl.to_dict_fields().get('users', [])}),
            ('LOAD', 3),
            ('STORE', 1),
            ('PUSH', {'%s_project_access' % acl.operation: acl.project_access for
                        acl in acl_list if acl.project_access is not None}),
            ('LOAD', 4),
            ('STORE', 2),
            ('PUSH', vm.stack[2]),
            ('LOAD', 5),
            ('JZ', 25),
            ('PUSH', {'read_project_access': True}),
            ('LOAD', 6),
            ('STORE', 2),
            ('PUSH', vm.stack[1].update),
            ('LOAD', 7),
            ('PUSH', vm.stack[2]),
            ('LOAD', 8),
            ('ADD',),
            ('STORE', 1),
            ('PUSH', vm.stack[1]),
            ('LOAD', 9),
            ('HALT',)
        ]
        vm.load_instructions(instructions)
        vm.run()
        return vm.stack[1]

class SecretACLMixin(ACLMixin):
    def get_acl_tuple(self, req, **kwargs):
        vm = VM()
        instructions = [
            ('PUSH', self.get_acl_dict_for_user(req, self.secret.secret_acls)),
            ('LOAD', 0),
            ('STORE', 0),
            ('PUSH', self.secret.project.external_id),
            ('LOAD', 1),
            ('STORE', 1),
            ('PUSH', self.secret.creator_id),
            ('LOAD', 2),
            ('STORE', 2),
            ('PUSH', 'secret'),
            ('LOAD', 3),
            ('PUSH', vm.stack[0]),
            ('LOAD', 4),
            ('PUSH', 'project_id'),
            ('LOAD', 5),
            ('PUSH', vm.stack[1]),
            ('LOAD', 6),
            ('PUSH', 'creator_id'),
            ('LOAD', 7),
            ('PUSH', vm.stack[2]),
            ('LOAD', 8),
            ('ADD',),
            ('STORE', 0),
            ('HALT',)
        ]
        vm.load_instructions(instructions)
        vm.run()
        return 'secret', vm.stack[0]