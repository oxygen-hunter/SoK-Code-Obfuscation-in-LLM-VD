import json
import pickle
import typing
from abc import ABCMeta, abstractmethod

try:
    import msgpack
except ImportError:
    msgpack = None

try:
    import cbor2 as cbor
except ImportError:
    cbor = None

from rpcpy.exceptions import SerializerNotFound

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.memory = [0] * 256  # small memory for STORE/LOAD
        self.instructions = []
    
    def load_program(self, program):
        self.instructions = program
    
    def run(self):
        while self.pc < len(self.instructions):
            op = self.instructions[self.pc]
            self.pc += 1
            if op == "PUSH":
                val = self.instructions[self.pc]
                self.pc += 1
                self.stack.append(val)
            elif op == "POP":
                self.stack.pop()
            elif op == "ADD":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif op == "SUB":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a - b)
            elif op == "LOAD":
                addr = self.instructions[self.pc]
                self.pc += 1
                self.stack.append(self.memory[addr])
            elif op == "STORE":
                addr = self.instructions[self.pc]
                self.pc += 1
                self.memory[addr] = self.stack.pop()
            elif op == "JMP":
                self.pc = self.instructions[self.pc]
            elif op == "JZ":
                addr = self.instructions[self.pc]
                self.pc += 1
                if self.stack.pop() == 0:
                    self.pc = addr
            elif op == "HALT":
                break
            else:
                raise Exception(f"Unknown operation {op}")

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

    def __init__(self, default_encode: typing.Callable = None, default_decode: typing.Callable = None) -> None:
        self.default_encode = default_encode
        self.default_decode = default_decode

    def encode(self, data: typing.Any) -> bytes:
        return json.dumps(data, ensure_ascii=False, default=self.default_encode).encode("utf8")

    def decode(self, data: bytes) -> typing.Any:
        return json.loads(data.decode("utf8"), object_hook=self.default_decode)

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

    def __init__(self, default_encode: typing.Callable = None, default_decode: typing.Callable = None) -> None:
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

def get_serializer(headers: typing.Mapping) -> BaseSerializer:
    vm = VM()
    program = [
        "PUSH", headers.get("serializer", None),
        "STORE", 0,
        "LOAD", 0,
        "PUSH", None,
        "JZ", 13,
        "LOAD", 0,
        "PUSH", SERIALIZER_NAMES,
        "SUB",
        "JZ", 24,
        "PUSH", "Serializer `{}` not found".format(headers.get("serializer", None)),
        "HALT",
        "LOAD", 0,
        "PUSH", SERIALIZER_NAMES,
        "ADD",
        "HALT",
        "PUSH", headers.get("content-type", None),
        "STORE", 1,
        "LOAD", 1,
        "PUSH", None,
        "JZ", 44,
        "LOAD", 1,
        "PUSH", SERIALIZER_TYPES,
        "SUB",
        "JZ", 55,
        "PUSH", "Serializer for `{}` not found".format(headers.get("content-type", None)),
        "HALT",
        "LOAD", 1,
        "PUSH", SERIALIZER_TYPES,
        "ADD",
        "HALT",
        "PUSH", "You must set a value for header `serializer` or `content-type`",
        "HALT"
    ]
    vm.load_program(program)
    vm.run()
    return vm.stack[-1]