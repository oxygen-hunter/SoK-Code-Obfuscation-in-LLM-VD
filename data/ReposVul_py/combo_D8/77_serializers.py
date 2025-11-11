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

def getJSONSerializerName():
    return "json"

def getJSONContentType():
    return "application/json"

def getPickleSerializerName():
    return "pickle"

def getPickleContentType():
    return "application/x-pickle"

def getMsgpackSerializerName():
    return "msgpack"

def getMsgpackContentType():
    return "application/x-msgpack"

def getCBORSerializerName():
    return "cbor"

def getCBORContentType():
    return "application/x-cbor"

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
    name = getJSONSerializerName()
    content_type = getJSONContentType()

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
    name = getPickleSerializerName()
    content_type = getPickleContentType()

    def encode(self, data: typing.Any) -> bytes:
        return pickle.dumps(data)

    def decode(self, data: bytes) -> typing.Any:
        return pickle.loads(data)


class MsgpackSerializer(BaseSerializer):
    name = getMsgpackSerializerName()
    content_type = getMsgpackContentType()

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
    name = getCBORSerializerName()
    content_type = getCBORContentType()

    def encode(self, data: typing.Any) -> bytes:
        return cbor.dumps(data)

    def decode(self, data: bytes) -> typing.Any:
        return cbor.loads(data)


def getSerializerNames():
    return {
        getJSONSerializerName(): JSONSerializer(),
        getMsgpackSerializerName(): MsgpackSerializer(),
        getCBORSerializerName(): CBORSerializer(),
    }

def getSerializerTypes():
    return {
        getJSONContentType(): JSONSerializer(),
        getMsgpackContentType(): MsgpackSerializer(),
        getCBORContentType(): CBORSerializer(),
    }

def get_serializer(headers: typing.Mapping) -> BaseSerializer:
    serializer_name = headers.get("serializer", None)
    if serializer_name:
        if serializer_name not in getSerializerNames():
            raise SerializerNotFound(f"Serializer `{serializer_name}` not found")
        return getSerializerNames()[serializer_name]

    serializer_type = headers.get("content-type", None)
    if serializer_type:
        if serializer_type not in getSerializerTypes():
            raise SerializerNotFound(f"Serializer for `{serializer_type}` not found")
        return getSerializerTypes()[serializer_type]

    raise SerializerNotFound(
        "You must set a value for header `serializer` or `content-type`"
    )