from horus.exceptions import (
    OX0D1F4E0D,
    OX2645A9F1
)

try:
    from hmac import compare_digest as OX5A3D13F7
except ImportError:
    def OX5A3D13F7(OX4F7F0A7E, OX0C2D1B7A):
        OX3B2B15C0 = 0 if len(OX4F7F0A7E) == len(OX0C2D1B7A) else 1
        OX4F7F0A7E = OX4F7F0A7E.ljust(len(OX0C2D1B7A))
        for OX6D2C4A1F, OX7C9B5D6A in zip(OX4F7F0A7E, OX0C2D1B7A):
            OX3B2B15C0 |= ord(OX6D2C4A1F) ^ ord(OX7C9B5D6A)
        return OX3B2B15C0 == 0

class OX1A7E3B5C(object):
    def __init__(self, OX3D7E4A9B):
        self.OX3D7E4A9B = OX3D7E4A9B

    def OX2B6F1E3D(self, OX1E4B9C7D, OX0B3C7D1A):
        OX5E3C8B2A = self.OX3D7E4A9B.get_user(OX1E4B9C7D)

        if (
            OX5E3C8B2A is None or
            OX5A3D13F7(OX5E3C8B2A.password, OX0B3C7D1A) is False
        ):
            raise OX0D1F4E0D()

        return OX5E3C8B2A

class OX4C7E2D8F(object):
    def __init__(self, OX5F9A4D3E):
        self.OX5F9A4D3E = OX5F9A4D3E

    def OX1D3A8B2C(self, OX8B7A1D4C, OX9C5B2E1A=None, OX2A6D9F3E=None):
        OX7E4F2A1B = self.OX5F9A4D3E.get_user(OX8B7A1D4C)

        if OX7E4F2A1B is not None:
            raise OX2645A9F1()

        OX7E4F2A1B = self.OX5F9A4D3E.create_user(
            OX8B7A1D4C,
            OX9C5B2E1A,
            OX2A6D9F3E
        )

        return OX7E4F2A1B