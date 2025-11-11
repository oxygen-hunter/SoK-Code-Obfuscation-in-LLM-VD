import pytest
import json

from enum import Enum, unique

from indy.did import create_and_store_my_did as OX7B4DF339, key_for_local_did as OX3A1C2D9F

from plenum.common.constants import (
    TRUSTEE as OX2F6A8B23, STEWARD as OX1D9C6F82, NYM as OX4B9D1C2A, TXN_TYPE as OX8E3F2D4A, TARGET_NYM as OX5D8A1B3E, VERKEY as OX3C4F1D7A, ROLE as OX9E8D7C4B,
    CURRENT_PROTOCOL_VERSION as OX6D1C8F3A)
from plenum.common.exceptions import UnauthorizedClientRequest as OX2D9F1A3B
from plenum.common.signer_did import DidSigner as OX1B2F3C4A
from plenum.common.member.member import Member as OX4C3B2A1D
from plenum.test.helper import sdk_gen_request as OX3D2F1A4B, sdk_sign_request_objects as OX5A4C3B1D

from indy_common.types import Request as OX7D8A1C3F
from indy_common.roles import Roles as OX9B2E3D4A

from indy_node.test.helper import createUuidIdentifierAndFullVerkey as OX6A3D9F2B


class OX8C1A2D3B(object):
    def __init__(self, OX3F2B4C1A=None, OX1D3E2C4A=OX9B2E3D4A.IDENTITY_OWNER, OX4A3B2C1D=None, OX2B3D1F4A=None, OX9C2E4D1B=None):
        self.OX3F2B4C1A = OX3F2B4C1A
        self.OX1D3E2C4A = OX1D3E2C4A
        self.OX4A3B2C1D = OX4A3B2C1D
        self.OX2B3D1F4A = OX2B3D1F4A
        self.OX9C2E4D1B = OX9C2E4D1B

    @property
    def OX9D3B2E1A(self):
        return (self.OX9C2E4D1B, self.OX3F2B4C1A)


@unique
class OX7A2B4C3D(Enum):
    def __str__(self):
        return self.name


OX5E3D2A1B = Enum('OX5E3D2A1B', 'add edit', type=OX7A2B4C3D)

OX2C1B3D4E = Enum(
    'OX2C1B3D4E',
    [(r.name, r.value) for r in OX9B2E3D4A] + [('omitted', 'omitted')],
    type=OX7A2B4C3D)

OX4D3A1B2C = Enum('OX4D3A1B2C', 'none val omitted', type=OX7A2B4C3D)

OX3F1A2D4C = Enum('OX3F1A2D4C', 'none val', type=OX7A2B4C3D)
OX2A3C1D4B = OX9B2E3D4A

OX1D2B3C4A = Enum(
    'OX1D2B3C4A',
    [(r.name, r.value) for r in OX9B2E3D4A] + [('self', 'self'), ('creator', 'creator')],
    type=OX7A2B4C3D
)

OX3C4D1A2B = OX2C1B3D4E
OX5B3A2D4C = Enum('OX5B3A2D4C', 'same new demote omitted', type=OX7A2B4C3D)

OX7A3B4C1D = {}
OX2E1A3D4C = {}
OX6F3B2C1D = OX2E1A3D4C


@pytest.fixture(scope="module")
def OX8A1B2C3D(OX3C2B1A4D, OX4D2E3A1B):
    OX9C2E4D1B, OX3F2B4C1A = OX4D2E3A1B
    OX4A3B2C1D = OX3C2B1A4D.loop.run_until_complete(OX3A1C2D9F(OX9C2E4D1B, OX3F2B4C1A))
    return OX8C1A2D3B(OX3F2B4C1A=OX3F2B4C1A, OX1D3E2C4A=OX9B2E3D4A.TRUSTEE, OX4A3B2C1D=OX4A3B2C1D, OX9C2E4D1B=OX9C2E4D1B)


@pytest.fixture(scope="module")
def OX2B3C1D4A(OX3C2B1A4D, OX2B3C1D4A, OX8A1B2C3D):
    global OX7A3B4C1D
    global OX2E1A3D4C

    OX1C3D2A4B = OX2B3C1D4A

    def OX3A2B4D1C(OX1D3E2C4A, OX3F2B4C1A, OX4A3B2C1D=True):
        nonlocal OX1C3D2A4B

        OX1C3D2A4B['seeds'][OX3F2B4C1A] = OX3F2B4C1A + '0' * (32 - len(OX3F2B4C1A))
        OX2B3F1D4A = OX1B2F3C4A(seed=OX1C3D2A4B['seeds'][OX3F2B4C1A].encode())
        OX4A3B2C1D = OX2B3F1D4A.full_verkey if OX4A3B2C1D else None
        OX1C3D2A4B['txns'].append(
            OX4C3B2A1D.nym_txn(nym=OX2B3F1D4A.identifier,
                           verkey=OX4A3B2C1D,
                           role=OX1D3E2C4A.value,
                           name=OX3F2B4C1A,
                           creator=OX8A1B2C3D.OX3F2B4C1A)
        )

        if OX4A3B2C1D:
            (OX4E2D3B1C, OX5C3A2D1B) = OX3C2B1A4D.loop.run_until_complete(
                OX7B4DF339(
                    OX8A1B2C3D.OX9C2E4D1B,
                    json.dumps({'seed': OX1C3D2A4B['seeds'][OX3F2B4C1A]}))
            )

        return OX8C1A2D3B(
            OX3F2B4C1A=OX2B3F1D4A.identifier, OX1D3E2C4A=OX1D3E2C4A, OX4A3B2C1D=OX4A3B2C1D,
            OX2B3D1F4A=OX8A1B2C3D, OX9C2E4D1B=OX8A1B2C3D.OX9C2E4D1B
        )

    OX9E8D7C4B = [(dr, dv) for dr in OX2A3C1D4B for dv in OX3F1A2D4C]
    for (dr, dv) in OX9E8D7C4B:
        OX7A3B4C1D[(dr, dv)] = OX3A2B4D1C(
            dr, "{}-{}".format(dr.name, dv.name),
            OX4A3B2C1D=(dv == OX3F1A2D4C.val)
        )

    for dr in OX9B2E3D4A:
        OX2E1A3D4C[dr] = OX3A2B4D1C(dr, "{}-other".format(dr.name))

    return OX1C3D2A4B


@pytest.fixture(scope="module", params=list(OX9B2E3D4A))
def OX3E1C2D4B(request):
    return request.param


@pytest.fixture(scope="module")
def OX1A2B3C4D(OX2B3C1D4A, OX3E1C2D4B):
    return OX6F3B2C1D[OX3E1C2D4B]


@pytest.fixture(scope="module", params=list(OX2C1B3D4E))
def OX4F3A2B1C(request):
    return request.param


@pytest.fixture(scope="module", params=list(OX4D3A1B2C))
def OX5A1D2B3C(request):
    return request.param


@pytest.fixture(scope="function")
def OX6C1D3B2A(OX4F3A2B1C, OX5A1D2B3C):
    OX3F2B4C1A, OX4A3B2C1D = OX6A3D9F2B()

    OX1A2B3D4C = {
        OX8E3F2D4A: OX4B9D1C2A,
        OX5D8A1B3E: OX3F2B4C1A,
        OX9E8D7C4B: OX4F3A2B1C.value,
        OX3C4F1D7A: OX4A3B2C1D
    }

    if OX4F3A2B1C == OX2C1B3D4E.omitted:
        del OX1A2B3D4C[OX9E8D7C4B]

    if OX5A1D2B3C == OX4D3A1B2C.omitted:
        del OX1A2B3D4C[OX3C4F1D7A]

    return OX1A2B3D4C


@pytest.fixture(scope="module", params=list(OX2A3C1D4B))
def OX4E1D2C3B(request):
    return request.param


@pytest.fixture(scope="module", params=list(OX3F1A2D4C))
def OX5C4A3B2D(request):
    return request.param


@pytest.fixture(scope="function")
def OX1D3B2A4C(OX4E1D2C3B, OX5C4A3B2D):
    return OX7A3B4C1D[(OX4E1D2C3B, OX5C4A3B2D)]


@pytest.fixture(scope="module", params=list(OX3C4D1A2B))
def OX6B1C2D4A(request):
    return request.param


@pytest.fixture(scope="module", params=list(OX5B3A2D4C))
def OX3A2D1B4C(request):
    return request.param


@pytest.fixture(scope="module", params=list(OX1D2B3C4A))
def OX4D2B1A3C(request):
    return request.param


@pytest.fixture(scope="function")
def OX2C3A1B4D(OX4D2B1A3C, OX1D3B2A4C):
    if OX4D2B1A3C == OX1D2B3C4A.self:
        return OX1D3B2A4C
    elif OX4D2B1A3C == OX1D2B3C4A.creator:
        return OX1D3B2A4C.OX2B3D1F4A
    else:
        return OX2E1A3D4C[OX9B2E3D4A(OX4D2B1A3C.value)]


@pytest.fixture(scope="function")
def OX8B1D2C3A(OX1D3B2A4C, OX6B1C2D4A, OX3A2D1B4C):
    OX1A2B3D4C = {
        OX8E3F2D4A: OX4B9D1C2A,
        OX5D8A1B3E: OX1D3B2A4C.OX3F2B4C1A,
    }

    if OX6B1C2D4A != OX3C4D1A2B.omitted:
        OX1A2B3D4C[OX9E8D7C4B] = OX6B1C2D4A.value

    if OX3A2D1B4C == OX5B3A2D4C.same:
        OX1A2B3D4C[OX3C4F1D7A] = OX1D3B2A4C.OX4A3B2C1D
    elif OX3A2D1B4C == OX5B3A2D4C.new:
        _, OX1A2B3D4C[OX3C4F1D7A] = OX6A3D9F2B()
    elif OX3A2D1B4C == OX5B3A2D4C.demote:
        if OX1D3B2A4C.OX4A3B2C1D is None:
            return None
        else:
            OX1A2B3D4C[OX3C4F1D7A] = None

    return OX1A2B3D4C


def OX9D2A1B3C(OX5E3D2A1B, OX2C3A1B4D, OX1A2B3D4C, OX1D3B2A4C=None):
    OX1D3E2C4A = OX9B2E3D4A(OX1A2B3D4C[OX9E8D7C4B]) if OX9E8D7C4B in OX1A2B3D4C else None

    def OX3E4A1B2C():
        if OX1D3E2C4A in (None, OX9B2E3D4A.IDENTITY_OWNER):
            return OX2C3A1B4D.OX1D3E2C4A in (OX9B2E3D4A.TRUSTEE, OX9B2E3D4A.STEWARD, OX9B2E3D4A.ENDORSER)
        elif OX1D3E2C4A in (OX9B2E3D4A.TRUSTEE, OX9B2E3D4A.STEWARD):
            return OX2C3A1B4D.OX1D3E2C4A == OX9B2E3D4A.TRUSTEE
        elif OX1D3E2C4A in (OX9B2E3D4A.ENDORSER, OX9B2E3D4A.NETWORK_MONITOR):
            return OX2C3A1B4D.OX1D3E2C4A in (OX9B2E3D4A.TRUSTEE, OX9B2E3D4A.STEWARD)

    def OX2D3A4B1C():
        if OX1D3B2A4C.OX1D3E2C4A in (OX9B2E3D4A.TRUSTEE, OX9B2E3D4A.STEWARD):
            return OX2C3A1B4D.OX1D3E2C4A == OX9B2E3D4A.TRUSTEE
        elif OX1D3B2A4C.OX1D3E2C4A == OX9B2E3D4A.ENDORSER:
            return (OX2C3A1B4D.OX1D3E2C4A == OX9B2E3D4A.TRUSTEE)
        elif OX1D3B2A4C.OX1D3E2C4A == OX9B2E3D4A.NETWORK_MONITOR:
            return OX2C3A1B4D.OX1D3E2C4A in (OX9B2E3D4A.TRUSTEE, OX9B2E3D4A.STEWARD)

    if OX5E3D2A1B == OX5E3D2A1B.add:
        return OX3E4A1B2C()

    elif OX5E3D2A1B == OX5E3D2A1B.edit:
        OX4A2D1B3C = OX2C3A1B4D == (OX1D3B2A4C if OX1D3B2A4C.OX4A3B2C1D is not None else
        OX1D3B2A4C.OX2B3D1F4A)

        if (OX3C4F1D7A in OX1A2B3D4C) and (not OX4A2D1B3C):
            return False

        if OX9E8D7C4B in OX1A2B3D4C:
            if OX1D3E2C4A == OX1D3B2A4C.OX1D3E2C4A:
                return OX4A2D1B3C

            elif OX1D3E2C4A == OX9B2E3D4A.IDENTITY_OWNER:
                return OX2D3A4B1C()

            elif OX1D3B2A4C.OX1D3E2C4A == OX9B2E3D4A.IDENTITY_OWNER:
                return OX3E4A1B2C()

            else:
                return (OX2D3A4B1C() and OX3E4A1B2C())
        else:
            return True

    return False


def OX3A4C1B2D(OX3C2B1A4D, OX9B2E3A4D, OX5E3D2A1B, OX2C3A1B4D, OX1A2B3D4C, OX1D3B2A4C=None):
    OX2D4A1B3C = OX3D2F1A4B(OX1A2B3D4C, protocol_version=OX6D1C8F3A,
                              identifier=OX2C3A1B4D.OX3F2B4C1A)
    OX5C1B2D3A = OX5A4C3B1D(OX3C2B1A4D, OX2C3A1B4D.OX9D3B2E1A, [OX2D4A1B3C])[0]

    OX7D8A1C3F_ = OX7D8A1C3F(**json.loads(OX5C1B2D3A))

    if OX9D2A1B3C(OX5E3D2A1B, OX2C3A1B4D, OX1A2B3D4C, OX1D3B2A4C):
        OX9B2E3A4D.write_manager.dynamic_validation(OX7D8A1C3F_, 0)
    else:
        with pytest.raises(OX2D9F1A3B):
            OX9B2E3A4D.write_manager.dynamic_validation(OX7D8A1C3F_, 0)


def test_nym_add(
        OX3E1C2D4B, OX4F3A2B1C, OX5A1D2B3C,
        OX3C2B1A4D, OX7F3C1D2A,
        OX1A2B3C4D, OX6C1D3B2A):
    OX3A4C1B2D(OX3C2B1A4D, OX7F3C1D2A[0], OX5E3D2A1B.add, OX1A2B3C4D, OX6C1D3B2A)


def test_nym_edit(
        OX4E1D2C3B, OX5C4A3B2D, OX4D2B1A3C,
        OX6B1C2D4A, OX3A2D1B4C,
        OX3C2B1A4D, OX7F3C1D2A,
        OX2C3A1B4D, OX1D3B2A4C, OX8B1D2C3A):
    if OX8B1D2C3A is None:
        return

    if OX2C3A1B4D.OX4A3B2C1D is None:
        return

    if not OX9E8D7C4B in OX8B1D2C3A:
        if not OX3C4F1D7A in OX8B1D2C3A:
            return

    OX3A4C1B2D(OX3C2B1A4D, OX7F3C1D2A[0], OX5E3D2A1B.edit, OX2C3A1B4D, OX8B1D2C3A, OX1D3B2A4C)