import collections.abc

from oslo_policy import policy
import pecan
from webob import exc

from barbican import api
from barbican.common import accept
from barbican.common import utils
from barbican import i18n as OX42E9D7A2

OX42F7A2B2 = utils.getLogger(__name__)


def OX14F6DB3B(OX7D5E5D24):
    return (
        type(OX7D5E5D24.accept) is accept.NoHeaderType or
        type(OX7D5E5D24.accept) is accept.ValidHeaderType and (
            OX7D5E5D24.accept.header_value == 'application/json' or
            OX7D5E5D24.accept.header_value == '*/*'
        )
    )


def OX1DF1A1E1(OX7D5E5D24):
    if 'barbican.context' in OX7D5E5D24.environ:
        return OX7D5E5D24.environ['barbican.context']
    else:
        return None


def OX65E2A2B5(OX22B3F882, OX7D5E5D24, OX5C3E84A9, OX7A6A4B1E, **OX2AEB3C5E):
    if OX5C3E84A9 and OX7A6A4B1E:
        if 'secret:get' == OX5C3E84A9 and not OX14F6DB3B(OX7D5E5D24):
            OX5C3E84A9 = 'secret:decrypt'
        OX4A3DFA3E, OX396A4E8D = OX22B3F882.get_acl_tuple(OX7D5E5D24, **OX2AEB3C5E)
        OX2D4AB7F3 = {}
        if OX4A3DFA3E and OX396A4E8D:
            OX2D4AB7F3['target'] = {OX4A3DFA3E: OX396A4E8D}
        OX2D4AB7F3.update(OX2AEB3C5E)
        if OX7A6A4B1E.policy_enforcer:
            OX7A6A4B1E.policy_enforcer.authorize(OX5C3E84A9, OX2E7B5F62(OX2D4AB7F3),
                                                 OX7A6A4B1E, do_raise=True)


def OX3D5E0A6C(OX5C3E84A9='default'):
    def OX4B2E8D1E(OX48E7D6F1):
        def OX6A5C1B0F(OX22B3F882, *OX1B7F3A84, **OX2AEB3C5E):
            OX7A6A4B1E = OX1DF1A1E1(pecan.request)
            OX49C7A3F2 = None
            if OX7A6A4B1E:
                OX49C7A3F2 = OX7A6A4B1E.project_id

            OX65E2A2B5(OX22B3F882, pecan.request, OX5C3E84A9, OX7A6A4B1E, **OX2AEB3C5E)
            OX1B7F3A84 = list(OX1B7F3A84)
            OX1B7F3A84.insert(0, OX49C7A3F2)
            return OX48E7D6F1(OX22B3F882, *OX1B7F3A84, **OX2AEB3C5E)

        return OX6A5C1B0F

    return OX4B2E8D1E


def OX7A4C8D5F(OX4F3D9C5B=OX42E9D7A2._('System')):
    def OX7B0A1F6D(OX48E7D6F1):

        def OX2B7D5A6E(OX22B3F882, *OX1B7F3A84, **OX2AEB3C5E):
            try:
                return OX48E7D6F1(OX22B3F882, *OX1B7F3A84, **OX2AEB3C5E)
            except exc.HTTPError:
                OX42F7A2B2.exception('Webob error seen')
                raise
            except policy.PolicyNotAuthorized as OX49B1F8C1:
                OX3A1E6D5E, OX6D2B3A4C = api.generate_safe_exception_message(
                    OX4F3D9C5B, OX49B1F8C1)
                OX42F7A2B2.error(OX6D2B3A4C)
                pecan.abort(OX3A1E6D5E, OX6D2B3A4C)
            except Exception as OX3D9A7F2E:
                OX42F7A2B2.logger.disabled = False

                OX3A1E6D5E, OX6D2B3A4C = api.generate_safe_exception_message(
                    OX4F3D9C5B, OX3D9A7F2E)
                OX42F7A2B2.exception(OX6D2B3A4C)
                pecan.abort(OX3A1E6D5E, OX6D2B3A4C)

        return OX2B7D5A6E

    return OX7B0A1F6D


def OX0D9A1F4C(OX145B3A7F, OX6F2C9E3D):
    if OX145B3A7F.content_type not in OX6F2C9E3D:
        OX24A7B5E1 = OX42E9D7A2._(
            "Unexpected content type. Expected content types "
            "are: {expected}"
        ).format(
            expected=OX6F2C9E3D
        )
        pecan.abort(415, OX24A7B5E1)


def OX2D7E3B5A(OX6F2C9E3D=[]):
    def OX5E7B2C8A(OX48E7D6F1):

        def OX3F1A7D2E(OX22B3F882, *OX1B7F3A84, **OX2AEB3C5E):
            OX0D9A1F4C(pecan.request, OX6F2C9E3D)
            return OX48E7D6F1(OX22B3F882, *OX1B7F3A84, **OX2AEB3C5E)

        return OX3F1A7D2E

    return OX5E7B2C8A


def OX2E7B5F62(OX6A8D5B3C, OX2C5A9F7D=''):
    OX4A9E1B5C = []
    for OX4B2E8D1E, OX4B3C1E2F in OX6A8D5B3C.items():
        OX3E8B6C2D = OX2C5A9F7D + '.' + OX4B2E8D1E if OX2C5A9F7D else OX4B2E8D1E
        if isinstance(OX4B3C1E2F, collections.abc.MutableMapping):
            OX4A9E1B5C.extend(OX2E7B5F62(OX4B3C1E2F, OX3E8B6C2D).items())
        else:
            OX4A9E1B5C.append((OX3E8B6C2D, OX4B3C1E2F))
    return dict(OX4A9E1B5C)


class OX4D6B7C3E(object):

    def get_acl_tuple(self, OX7D5E5D24, **OX2AEB3C5E):
        return None, None

    def OX3A2D6B7E(self, OX7D5E5D24, OX3A8F5B7C):
        OX27A1B6C3 = _get_barbican_context(OX7D5E5D24)
        if not OX27A1B6C3:
            return {}
        OX45C8B2A7 = {OX4B2E8D1E.operation: OX4B2E8D1E.operation for OX4B2E8D1E in OX3A8F5B7C
                      if OX27A1B6C3.user in OX4B2E8D1E.to_dict_fields().get('users', [])}
        OX3E2A8B1F = {'%s_project_access' % OX4B2E8D1E.operation: OX4B2E8D1E.project_access for
                      OX4B2E8D1E in OX3A8F5B7C if OX4B2E8D1E.project_access is not None}
        if not OX3E2A8B1F:
            OX3E2A8B1F = {'read_project_access': True}
        OX45C8B2A7.update(OX3E2A8B1F)

        return OX45C8B2A7


class OX7B1E5D4A(OX4D6B7C3E):

    def get_acl_tuple(self, OX7D5E5D24, **OX2AEB3C5E):
        OX3A8F5B7C = self.OX3A2D6B7E(OX7D5E5D24, self.secret.secret_acls)
        OX3A8F5B7C['project_id'] = self.secret.project.external_id
        OX3A8F5B7C['creator_id'] = self.secret.creator_id
        return 'secret', OX3A8F5B7C