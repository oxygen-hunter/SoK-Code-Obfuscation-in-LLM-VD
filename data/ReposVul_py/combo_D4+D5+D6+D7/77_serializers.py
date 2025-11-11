import json
import pickle
import typing
from abc import ABCMeta, abstractmethod

try:
    import msgpack
except ImportError:  # pragma: no cover
    msgpack = None  # type: ignore

try:
    import cbor2 as cbor
except ImportError:  # pragma: no cover
    cbor = None  # type: ignore

from rpcpy.exceptions import SerializerNotFound

class BaseSerializer(metaclass=ABCMeta):
    __slots__ = ('content_type', 'name') # obfuscating by using slots and changing order

    def encode(self, x: typing.Any) -> bytes:
        raise NotImplementedError()

    def decode(self, x: bytes) -> typing.Any:
        raise NotImplementedError()

class JSONSerializer(BaseSerializer):
    def __init__(
        self,
        x: typing.Callable = None,
        y: typing.Callable = None,
    ) -> None:
        self.e, self.d = x, y

    def encode(self, x: typing.Any) -> bytes:
        return json.dumps(x, ensure_ascii=False, default=self.e).encode("utf8")

    def decode(self, x: bytes) -> typing.Any:
        return json.loads(x.decode("utf8"), object_hook=self.d)

class PickleSerializer(BaseSerializer):
    def encode(self, x: typing.Any) -> bytes:
        return pickle.dumps(x)

    def decode(self, x: bytes) -> typing.Any:
        return pickle.loads(x)

class MsgpackSerializer(BaseSerializer):
    def __init__(
        self,
        x: typing.Callable = None,
        y: typing.Callable = None,
    ) -> None:
        self.e, self.d = x, y

    def encode(self, x: typing.Any) -> bytes:
        return msgpack.packb(x, default=self.e)

    def decode(self, x: bytes) -> typing.Any:
        return msgpack.unpackb(x, object_hook=self.d)

class CBORSerializer(BaseSerializer):
    def encode(self, x: typing.Any) -> bytes:
        return cbor.dumps(x)

    def decode(self, x: bytes) -> typing.Any:
        return cbor.loads(x)

def get_serializer(x: typing.Mapping) -> BaseSerializer:
    y, z = x.get("serializer", None), x.get("content-type", None)
    if y:
        if y not in a:
            raise SerializerNotFound(f"Serializer `{y}` not found")
        return a[y]
    if z:
        if z not in b:
            raise SerializerNotFound(f"Serializer for `{z}` not found")
        return b[z]
    raise SerializerNotFound("You must set a value for header `serializer` or `content-type`")

a = {JSONSerializer().name: JSONSerializer(), MsgpackSerializer().name: MsgpackSerializer(), CBORSerializer().name: CBORSerializer()}
b = {JSONSerializer().content_type: JSONSerializer(), MsgpackSerializer().content_type: MsgpackSerializer(), CBORSerializer().content_type: CBORSerializer()}