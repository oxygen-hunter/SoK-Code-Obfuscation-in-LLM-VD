import json as OX78E3D45A
import pickle as OX9A7B6C3B
import typing as OXC123F2D8
from abc import ABCMeta as OX6E5F4A1B, abstractmethod as OX1A2D4F7C

try:
    import msgpack as OX4B5A2E1C
except ImportError:  # pragma: no cover
    OX4B5A2E1C = None  # type: ignore

try:
    import cbor2 as OX3C8D7B2F
except ImportError:  # pragma: no cover
    OX3C8D7B2F = None  # type: ignore

from rpcpy.exceptions import SerializerNotFound as OX7F4E3C2A


class OX2D3A4B5C(metaclass=OX6E5F4A1B):
    name: str
    content_type: str

    @OX1A2D4F7C
    def OX7C8D9E0F(self, OX1A2B3C4D: OXC123F2D8.Any) -> bytes:
        raise NotImplementedError()

    @OX1A2D4F7C
    def OX6B7C8D9E(self, OX5A6B7C8D: bytes) -> OXC123F2D8.Any:
        raise NotImplementedError()


class OX5C6D7E8F(OX2D3A4B5C):
    name = "json"
    content_type = "application/json"

    def __init__(
        self,
        OX1F2E3D4C: OXC123F2D8.Callable = None,
        OX2E3D4C5B: OXC123F2D8.Callable = None,
    ) -> None:
        self.OX1F2E3D4C = OX1F2E3D4C
        self.OX2E3D4C5B = OX2E3D4C5B

    def OX7C8D9E0F(self, OX1A2B3C4D: OXC123F2D8.Any) -> bytes:
        return OX78E3D45A.dumps(
            OX1A2B3C4D,
            ensure_ascii=False,
            default=self.OX1F2E3D4C,
        ).encode("utf8")

    def OX6B7C8D9E(self, OX5A6B7C8D: bytes) -> OXC123F2D8.Any:
        return OX78E3D45A.loads(
            OX5A6B7C8D.decode("utf8"),
            object_hook=self.OX2E3D4C5B,
        )


class OX8F9A0B1C(OX2D3A4B5C):
    name = "pickle"
    content_type = "application/x-pickle"

    def OX7C8D9E0F(self, OX1A2B3C4D: OXC123F2D8.Any) -> bytes:
        return OX9A7B6C3B.dumps(OX1A2B3C4D)

    def OX6B7C8D9E(self, OX5A6B7C8D: bytes) -> OXC123F2D8.Any:
        return OX9A7B6C3B.loads(OX5A6B7C8D)


class OX1B2C3D4E(OX2D3A4B5C):
    name = "msgpack"
    content_type = "application/x-msgpack"

    def __init__(
        self,
        OX1F2E3D4C: OXC123F2D8.Callable = None,
        OX2E3D4C5B: OXC123F2D8.Callable = None,
    ) -> None:
        self.OX1F2E3D4C = OX1F2E3D4C
        self.OX2E3D4C5B = OX2E3D4C5B

    def OX7C8D9E0F(self, OX1A2B3C4D: OXC123F2D8.Any) -> bytes:
        return OX4B5A2E1C.packb(OX1A2B3C4D, default=self.OX1F2E3D4C)

    def OX6B7C8D9E(self, OX5A6B7C8D: bytes) -> OXC123F2D8.Any:
        return OX4B5A2E1C.unpackb(OX5A6B7C8D, object_hook=self.OX2E3D4C5B)


class OX2F3E4D5C(OX2D3A4B5C):
    name = "cbor"
    content_type = "application/x-cbor"

    def OX7C8D9E0F(self, OX1A2B3C4D: OXC123F2D8.Any) -> bytes:
        return OX3C8D7B2F.dumps(OX1A2B3C4D)

    def OX6B7C8D9E(self, OX5A6B7C8D: bytes) -> OXC123F2D8.Any:
        return OX3C8D7B2F.loads(OX5A6B7C8D)


OX7D8E9F0A = {
    OX5C6D7E8F.name: OX5C6D7E8F(),
    OX1B2C3D4E.name: OX1B2C3D4E(),
    OX2F3E4D5C.name: OX2F3E4D5C(),
}

OX6A7B8C9D = {
    OX5C6D7E8F.content_type: OX5C6D7E8F(),
    OX1B2C3D4E.content_type: OX1B2C3D4E(),
    OX2F3E4D5C.content_type: OX2F3E4D5C(),
}


def OX4C5D6E7F(OX9A8B7C6D: OXC123F2D8.Mapping) -> OX2D3A4B5C:
    OX3B2C1D0E = OX9A8B7C6D.get("serializer", None)
    if OX3B2C1D0E:
        if OX3B2C1D0E not in OX7D8E9F0A:
            raise OX7F4E3C2A(f"Serializer `{OX3B2C1D0E}` not found")
        return OX7D8E9F0A[OX3B2C1D0E]

    OX0F1E2D3C = OX9A8B7C6D.get("content-type", None)
    if OX0F1E2D3C:
        if OX0F1E2D3C not in OX6A7B8C9D:
            raise OX7F4E3C2A(f"Serializer for `{OX0F1E2D3C}` not found")
        return OX6A7B8C9D[OX0F1E2D3C]

    raise OX7F4E3C2A(
        "You must set a value for header `serializer` or `content-type`"
    )