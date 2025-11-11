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
    name: str
    content_type: str

    @abstractmethod
    def encode(self, data: typing.Any) -> bytes:
        raise NotImplementedError()

    @abstractmethod
    def decode(self, raw_data: bytes) -> typing.Any:
        raise NotImplementedError()


class JSONSerializer(BaseSerializer):
    name = "json"
    content_type = "application/json"

    def __init__(
        self,
        default_encode: typing.Callable = None,
        default_decode: typing.Callable = None,
    ) -> None:
        self.default_encode = default_encode
        self.default_decode = default_decode

    def encode(self, data: typing.Any) -> bytes:
        return json.dumps(
            data,
            ensure_ascii=False,
            default=self.default_encode,
        ).encode("utf8")

    def decode(self, data: bytes) -> typing.Any:
        return json.loads(
            data.decode("utf8"),
            object_hook=self.default_decode,
        )


class PickleSerializer(BaseSerializer):
    name = "pickle"
    content_type = "application/x-pickle"

    def encode(self, data: typing.Any) -> bytes:
        return pickle.dumps(data)

    def decode(self, data: bytes) -> typing.Any:
        return pickle.loads(data)


class MsgpackSerializer(BaseSerializer):
    name = "msgpack"
    content_type = "application/x-msgpack"

    def __init__(
        self,
        default_encode: typing.Callable = None,
        default_decode: typing.Callable = None,
    ) -> None:
        self.default_encode = default_encode
        self.default_decode = default_decode

    def encode(self, data: typing.Any) -> bytes:
        return msgpack.packb(data, default=self.default_encode)

    def decode(self, data: bytes) -> typing.Any:
        return msgpack.unpackb(data, object_hook=self.default_decode)


class CBORSerializer(BaseSerializer):
    name = "cbor"
    content_type = "application/x-cbor"

    def encode(self, data: typing.Any) -> bytes:
        return cbor.dumps(data)

    def decode(self, data: bytes) -> typing.Any:
        return cbor.loads(data)


SERIALIZER_NAMES = {
    JSONSerializer.name: JSONSerializer(),
    MsgpackSerializer.name: MsgpackSerializer(),
    CBORSerializer.name: CBORSerializer(),
}

SERIALIZER_TYPES = {
    JSONSerializer.content_type: JSONSerializer(),
    MsgpackSerializer.content_type: MsgpackSerializer(),
    CBORSerializer.content_type: CBORSerializer(),
}


def recursive_get_serializer(headers: typing.Mapping, keys: list) -> BaseSerializer:
    if not keys:
        raise SerializerNotFound(
            "You must set a value for header `serializer` or `content-type`"
        )
    key = keys[0]
    serializer_value = headers.get(key, None)

    if key == "serializer":
        if serializer_value and serializer_value in SERIALIZER_NAMES:
            return SERIALIZER_NAMES[serializer_value]
    elif key == "content-type":
        if serializer_value and serializer_value in SERIALIZER_TYPES:
            return SERIALIZER_TYPES[serializer_value]

    return recursive_get_serializer(headers, keys[1:])


def get_serializer(headers: typing.Mapping) -> BaseSerializer:
    return recursive_get_serializer(headers, ["serializer", "content-type"])