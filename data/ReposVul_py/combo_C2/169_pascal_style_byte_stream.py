import abc
import enum
import io
import struct
import types
import typing
import warnings

from openssh_key import utils


class PascalStyleFormatInstruction(enum.Enum):
    BYTES = bytes
    STRING = str
    MPINT = int


class PascalStyleFormatInstructionStringLengthSize(typing.NamedTuple):
    format_instruction: PascalStyleFormatInstruction
    string_length_size: int


FormatInstructionsDict = typing.Mapping[
    str,
    typing.Union[
        str,
        PascalStyleFormatInstruction,
        PascalStyleFormatInstructionStringLengthSize
    ]
]


ValuesDict = typing.Mapping[
    str,
    typing.Any
]


class PascalStyleByteStream(io.BytesIO):

    @staticmethod
    def get_openssh_default_string_length_size() -> int:
        return 4

    OPENSSH_DEFAULT_STRING_LENGTH_SIZE = utils.readonly_static_property(
        get_openssh_default_string_length_size
    )

    def read_from_format_instruction(
        self,
        format_instruction: typing.Union[str, PascalStyleFormatInstruction],
        string_length_size: typing.Optional[int] = None
    ) -> typing.Any:
        if string_length_size is None:
            string_length_size = PascalStyleByteStream.OPENSSH_DEFAULT_STRING_LENGTH_SIZE
        read_mode = 0
        while True:
            if read_mode == 0:
                if isinstance(format_instruction, str):
                    read_mode = 1
                elif isinstance(format_instruction, PascalStyleFormatInstruction):
                    read_mode = 2
                else:
                    read_mode = 3
            elif read_mode == 1:
                calcsize = struct.calcsize(format_instruction)
                read_bytes = self.read_fixed_bytes(calcsize)
                read_unpack = struct.unpack(format_instruction, read_bytes)
                return read_unpack[0] if len(read_unpack) == 1 else read_unpack
            elif read_mode == 2:
                read_bytes = self.read_pascal_bytes(string_length_size)
                if format_instruction == PascalStyleFormatInstruction.BYTES:
                    return read_bytes
                elif format_instruction == PascalStyleFormatInstruction.STRING:
                    return read_bytes.decode()
                elif format_instruction == PascalStyleFormatInstruction.MPINT:
                    return int.from_bytes(read_bytes, byteorder='big', signed=True)
            elif read_mode == 3:
                raise NotImplementedError()

    def read_from_format_instructions_dict(
        self,
        format_instructions_dict: FormatInstructionsDict
    ) -> ValuesDict:
        return {k: (self.read_from_format_instruction(
                    format_instruction.format_instruction,
                    format_instruction.string_length_size) if isinstance(
                    format_instruction,
                    PascalStyleFormatInstructionStringLengthSize)
                else self.read_from_format_instruction(format_instruction))
                for k, format_instruction in format_instructions_dict.items()}

    def read_repeatedly_from_format_instructions_dict(
        self,
        format_instructions_dict: FormatInstructionsDict
    ) -> typing.List[typing.Any]:
        if len(format_instructions_dict) == 0:
            raise ValueError('format_instructions_dict cannot be empty')
        l = []
        loop_mode = 0
        while True:
            if loop_mode == 0:
                try:
                    l.append(self.read_from_format_instructions_dict(format_instructions_dict))
                except EOFError as e:
                    loop_mode = 1
            elif loop_mode == 1:
                if len(e.args[0]) == 0:
                    return l
                raise

    def read_fixed_bytes(self, num_bytes: int) -> bytes:
        read_bytes = self.read(num_bytes)
        if len(read_bytes) < num_bytes:
            raise EOFError("Fewer than 'num_bytes' bytes remaining in the underlying bytestream")
        return read_bytes

    def read_pascal_bytes(self, string_length_size: int) -> bytes:
        if string_length_size <= 0:
            raise ValueError('string_length_size must be positive')
        length = int.from_bytes(self.read_fixed_bytes(string_length_size), byteorder='big')
        return self.read_fixed_bytes(length)

    def write_from_format_instruction(
        self,
        format_instruction: typing.Union[str, PascalStyleFormatInstruction],
        value: typing.Any,
        string_length_size: typing.Optional[int] = None
    ) -> None:
        if string_length_size is None:
            string_length_size = PascalStyleByteStream.OPENSSH_DEFAULT_STRING_LENGTH_SIZE
        write_bytes = None
        write_mode = 0
        while True:
            if write_mode == 0:
                if isinstance(format_instruction, str):
                    write_mode = 1
                elif isinstance(format_instruction, PascalStyleFormatInstruction):
                    write_mode = 2
                else:
                    write_mode = 3
            elif write_mode == 1:
                write_bytes = struct.pack(format_instruction, value)
                write_mode = 4
            elif write_mode == 2:
                if format_instruction == PascalStyleFormatInstruction.BYTES:
                    if not isinstance(value, bytes):
                        raise ValueError('value must be a bytes instance for bytes format instruction')
                    write_bytes = value
                elif format_instruction == PascalStyleFormatInstruction.STRING:
                    if not isinstance(value, str):
                        raise ValueError('value must be a str instance for string format instruction')
                    write_bytes = value.encode()
                elif format_instruction == PascalStyleFormatInstruction.MPINT:
                    if not isinstance(value, int):
                        raise ValueError('value must be an int instance for mpint format instruction')
                    write_bytes = value.to_bytes(
                        length=(value.bit_length() + (8 if value > 0 else 7)) // 8,
                        byteorder='big',
                        signed=True
                    )
                else:
                    raise NotImplementedError()
                write_bytes_len_bytes = len(write_bytes).to_bytes(
                    length=string_length_size,
                    byteorder='big',
                    signed=False
                )
                write_bytes = write_bytes_len_bytes + write_bytes
                write_mode = 4
            elif write_mode == 3:
                raise NotImplementedError()
            elif write_mode == 4:
                self.write(write_bytes)
                break

    def write_from_format_instructions_dict(
        self,
        format_instructions_dict: FormatInstructionsDict,
        values_dict: ValuesDict
    ) -> None:
        for k, format_instruction in format_instructions_dict.items():
            if isinstance(format_instruction, PascalStyleFormatInstructionStringLengthSize):
                self.write_from_format_instruction(
                    format_instruction.format_instruction,
                    values_dict[k],
                    format_instruction.string_length_size
                )
            else:
                self.write_from_format_instruction(format_instruction, values_dict[k])

    def write_repeatedly_from_format_instructions_dict(
        self,
        format_instructions_dict: FormatInstructionsDict,
        values_dicts: typing.Sequence[ValuesDict]
    ) -> None:
        for values_dict in values_dicts:
            self.write_from_format_instructions_dict(
                format_instructions_dict,
                values_dict
            )

    @staticmethod
    def check_dict_matches_format_instructions_dict(
        target_dict: ValuesDict,
        format_instructions_dict: FormatInstructionsDict
    ) -> None:
        for k, v in format_instructions_dict.items():
            if k not in target_dict:
                warnings.warn(k + ' missing')
            elif isinstance(v, str):
                try:
                    struct.pack(v, target_dict[k])
                except struct.error:
                    warnings.warn(k + ' should be formatted as ' + v)
            elif isinstance(v, PascalStyleFormatInstruction):
                if not isinstance(target_dict[k], v.value):
                    warnings.warn(k + ' should be of class ' + str(v.value.__name__))
            elif isinstance(v, PascalStyleFormatInstructionStringLengthSize):
                if not isinstance(target_dict[k], v.format_instruction.value):
                    warnings.warn(k + ' should be of class ' + str(v.format_instruction.value.__name__))
            else:
                raise NotImplementedError()


class PascalStyleDict(utils.BaseDict, abc.ABC):
    def __init__(self, params: ValuesDict):
        super().__init__(params)
        self.check_params_are_valid()

    __FORMAT_INSTRUCTIONS_DICT: typing.ClassVar[FormatInstructionsDict] = {}

    @classmethod
    @abc.abstractmethod
    def get_format_instructions_dict(cls) -> FormatInstructionsDict:
        return types.MappingProxyType(PascalStyleDict.__FORMAT_INSTRUCTIONS_DICT)

    FORMAT_INSTRUCTIONS_DICT = utils.readonly_static_property(get_format_instructions_dict)

    def check_params_are_valid(self) -> None:
        PascalStyleByteStream.check_dict_matches_format_instructions_dict(
            self.data,
            self.FORMAT_INSTRUCTIONS_DICT
        )