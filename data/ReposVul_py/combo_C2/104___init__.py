import collections.abc

from oslo_policy import policy
import pecan
from webob import exc

from barbican import api
from barbican.common import accept
from barbican.common import utils
from barbican import i18n as u

LOG = utils.getLogger(__name__)


def is_json_request_accept(req):
    state = 0
    result = None
    while True:
        if state == 0:
            if type(req.accept) is accept.NoHeaderType:
                result = True
                state = 4
            else:
                state = 1
        elif state == 1:
            if type(req.accept) is accept.ValidHeaderType:
                state = 2
            else:
                result = False
                state = 4
        elif state == 2:
            if req.accept.header_value == 'application/json':
                result = True
                state = 4
            else:
                state = 3
        elif state == 3:
            if req.accept.header_value == '*/*':
                result = True
            else:
                result = False
            state = 4
        elif state == 4:
            break
    return result


def _get_barbican_context(req):
    state = 0
    result = None
    while True:
        if state == 0:
            if 'barbican.context' in req.environ:
                result = req.environ['barbican.context']
            else:
                result = None
            state = 1
        elif state == 1:
            break
    return result


def _do_enforce_rbac(inst, req, action_name, ctx, **kwargs):
    state = 0
    while True:
        if state == 0:
            if action_name and ctx:
                state = 1
            else:
                state = 5
        elif state == 1:
            if 'secret:get' == action_name and not is_json_request_accept(req):
                action_name = 'secret:decrypt'
            target_name, target_data = inst.get_acl_tuple(req, **kwargs)
            policy_dict = {}
            if target_name and target_data:
                policy_dict['target'] = {target_name: target_data}
            policy_dict.update(kwargs)
            state = 2
        elif state == 2:
            if ctx.policy_enforcer:
                state = 3
            else:
                state = 5
        elif state == 3:
            ctx.policy_enforcer.authorize(action_name, flatten(policy_dict),
                                          ctx, do_raise=True)
            state = 5
        elif state == 5:
            break


def enforce_rbac(action_name='default'):
    def rbac_decorator(fn):
        def enforcer(inst, *args, **kwargs):
            state = 0
            result = None
            while True:
                if state == 0:
                    ctx = _get_barbican_context(pecan.request)
                    external_project_id = None
                    if ctx:
                        external_project_id = ctx.project_id
                    _do_enforce_rbac(inst, pecan.request, action_name, ctx, **kwargs)
                    args = list(args)
                    args.insert(0, external_project_id)
                    state = 1
                elif state == 1:
                    result = fn(inst, *args, **kwargs)
                    state = 2
                elif state == 2:
                    break
            return result

        return enforcer

    return rbac_decorator


def handle_exceptions(operation_name=u._('System')):
    def exceptions_decorator(fn):
        def handler(inst, *args, **kwargs):
            state = 0
            result = None
            while True:
                if state == 0:
                    try:
                        result = fn(inst, *args, **kwargs)
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
                    state = 1
                elif state == 1:
                    break
            return result

        return handler

    return exceptions_decorator


def _do_enforce_content_types(pecan_req, valid_content_types):
    state = 0
    while True:
        if state == 0:
            if pecan_req.content_type not in valid_content_types:
                m = u._(
                    "Unexpected content type. Expected content types "
                    "are: {expected}"
                ).format(
                    expected=valid_content_types
                )
                pecan.abort(415, m)
            state = 1
        elif state == 1:
            break


def enforce_content_types(valid_content_types=[]):
    def content_types_decorator(fn):
        def content_types_enforcer(inst, *args, **kwargs):
            state = 0
            result = None
            while True:
                if state == 0:
                    _do_enforce_content_types(pecan.request, valid_content_types)
                    result = fn(inst, *args, **kwargs)
                    state = 1
                elif state == 1:
                    break
            return result

        return content_types_enforcer

    return content_types_decorator


def flatten(d, parent_key=''):
    state = 0
    items = []
    while True:
        if state == 0:
            for k, v in d.items():
                new_key = parent_key + '.' + k if parent_key else k
                if isinstance(v, collections.abc.MutableMapping):
                    items.extend(flatten(v, new_key).items())
                else:
                    items.append((new_key, v))
            state = 1
        elif state == 1:
            break
    return dict(items)


class ACLMixin(object):
    def get_acl_tuple(self, req, **kwargs):
        return None, None

    def get_acl_dict_for_user(self, req, acl_list):
        state = 0
        acl_dict = {}
        while True:
            if state == 0:
                ctxt = _get_barbican_context(req)
                if not ctxt:
                    acl_dict = {}
                    state = 2
                else:
                    state = 1
            elif state == 1:
                acl_dict = {acl.operation: acl.operation for acl in acl_list
                            if ctxt.user in acl.to_dict_fields().get('users', [])}
                co_dict = {'%s_project_access' % acl.operation: acl.project_access for
                           acl in acl_list if acl.project_access is not None}
                if not co_dict:
                    co_dict = {'read_project_access': True}
                acl_dict.update(co_dict)
                state = 2
            elif state == 2:
                break
        return acl_dict


class SecretACLMixin(ACLMixin):
    def get_acl_tuple(self, req, **kwargs):
        acl = self.get_acl_dict_for_user(req, self.secret.secret_acls)
        acl['project_id'] = self.secret.project.external_id
        acl['creator_id'] = self.secret.creator_id
        return 'secret', acl