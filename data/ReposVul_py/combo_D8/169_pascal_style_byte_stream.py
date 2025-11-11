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
        f_i: typing.Union[str, PascalStyleFormatInstruction],
        s_l_s: typing.Optional[int] = None
    ) -> typing.Any:
        if s_l_s is None:
            s_l_s = PascalStyleByteStream.OPENSSH_DEFAULT_STRING_LENGTH_SIZE
        if isinstance(f_i, str):
            c = struct.calcsize(f_i)
            r_b = self.read_fixed_bytes(c)
            r_u = struct.unpack(f_i, r_b)
            if len(r_u) == 1:
                return r_u[0]
            return r_u
        elif isinstance(f_i, PascalStyleFormatInstruction):
            r_b = self.read_pascal_bytes(s_l_s)
            if f_i == PascalStyleFormatInstruction.BYTES:
                return r_b
            elif f_i == PascalStyleFormatInstruction.STRING:
                return r_b.decode()
            elif f_i == PascalStyleFormatInstruction.MPINT:
                return int.from_bytes(
                    r_b,
                    byteorder='big',
                    signed=True
                )
        raise NotImplementedError()

    def read_from_format_instructions_dict(
        self,
        f_i_d: FormatInstructionsDict
    ) -> ValuesDict:
        return {
            k: (
                self.read_from_format_instruction(
                    f_i.format_instruction,
                    f_i.string_length_size
                ) if isinstance(
                    f_i,
                    PascalStyleFormatInstructionStringLengthSize
                )
                else self.read_from_format_instruction(f_i)
            )
            for k, f_i in f_i_d.items()
        }

    def read_repeatedly_from_format_instructions_dict(
        self,
        f_i_d: FormatInstructionsDict
    ) -> typing.List[typing.Any]:
        if len(f_i_d) == 0:
            raise ValueError('format_instructions_dict cannot be empty')
        l = []
        while True:
            try:
                l.append(
                    self.read_from_format_instructions_dict(
                        f_i_d
                    )
                )
            except EOFError as e:
                if len(e.args[0]) == 0:
                    return l
                raise

    def read_fixed_bytes(self, n_b: int) -> bytes:
        r_b = self.read(n_b)
        if len(r_b) < n_b:
            raise EOFError("Fewer than 'num_bytes' bytes remaining in the "
                    "underlying bytestream")
        return r_b

    def read_pascal_bytes(self, s_l_s: int) -> bytes:
        if s_l_s <= 0:
            raise ValueError('string_length_size must be positive')
        length = int.from_bytes(
            self.read_fixed_bytes(s_l_s),
            byteorder='big'
        )
        return self.read_fixed_bytes(length)

    def write_from_format_instruction(
        self,
        f_i: typing.Union[str, PascalStyleFormatInstruction],
        v: typing.Any,
        s_l_s: typing.Optional[int] = None
    ) -> None:
        if s_l_s is None:
            s_l_s = PascalStyleByteStream.OPENSSH_DEFAULT_STRING_LENGTH_SIZE
        w_b = None
        if isinstance(f_i, str):
            w_b = struct.pack(f_i, v)
        elif isinstance(f_i, PascalStyleFormatInstruction):
            if f_i == PascalStyleFormatInstruction.BYTES:
                if not isinstance(v, bytes):
                    raise ValueError(
                        'value must be a bytes instance for bytes '
                        'format instruction'
                    )
                w_b = v
            elif f_i == PascalStyleFormatInstruction.STRING:
                if not isinstance(v, str):
                    raise ValueError(
                        'value must be a str instance for string '
                        'format instruction'
                    )
                w_b = v.encode()
            elif f_i == PascalStyleFormatInstruction.MPINT:
                if not isinstance(v, int):
                    raise ValueError(
                        'value must be an int instance for mpint '
                        'format instruction'
                    )
                w_b = v.to_bytes(
                    length=(v.bit_length() + (8 if v > 0 else 7)) // 8,
                    byteorder='big',
                    signed=True
                )
            else:
                raise NotImplementedError()
            w_b_len_b = len(w_b).to_bytes(
                length=s_l_s,
                byteorder='big',
                signed=False
            )
            w_b = w_b_len_b + w_b
        else:
            raise NotImplementedError()
        self.write(w_b)

    def write_from_format_instructions_dict(
        self,
        f_i_d: FormatInstructionsDict,
        v_d: ValuesDict
    ) -> None:
        for k, f_i in f_i_d.items():
            if isinstance(
                f_i,
                PascalStyleFormatInstructionStringLengthSize
            ):
                self.write_from_format_instruction(
                    f_i.format_instruction,
                    v_d[k],
                    f_i.string_length_size
                )
            else:
                self.write_from_format_instruction(
                    f_i,
                    v_d[k]
                )

    def write_repeatedly_from_format_instructions_dict(
        self,
        f_i_d: FormatInstructionsDict,
        v_ds: typing.Sequence[ValuesDict]
    ) -> None:
        for v_d in v_ds:
            self.write_from_format_instructions_dict(
                f_i_d,
                v_d
            )

    @staticmethod
    def check_dict_matches_format_instructions_dict(
        t_d: ValuesDict,
        f_i_d: FormatInstructionsDict
    ) -> None:
        for k, v in f_i_d.items():
            if k not in t_d:
                warnings.warn(k + ' missing')
            elif isinstance(v, str):
                try:
                    struct.pack(v, t_d[k])
                except struct.error:
                    warnings.warn(
                        k + ' should be formatted as ' + v
                    )
            elif isinstance(v, PascalStyleFormatInstruction):
                if not isinstance(t_d[k], v.value):
                    warnings.warn(
                        k + ' should be of class ' + str(v.value.__name__)
                    )
            elif isinstance(v, PascalStyleFormatInstructionStringLengthSize):
                if not isinstance(t_d[k], v.format_instruction.value):
                    warnings.warn(
                        k + ' should be of class ' +
                            str(v.format_instruction.value.__name__)
                    )
            else:
                raise NotImplementedError()

class PascalStyleDict(utils.BaseDict, abc.ABC):
    def __init__(self, p: ValuesDict):
        super().__init__(p)
        self.check_params_are_valid()

    __F_I_D: typing.ClassVar[FormatInstructionsDict] = {}

    @classmethod
    @abc.abstractmethod
    def get_format_instructions_dict(cls) -> FormatInstructionsDict:
        return types.MappingProxyType(
            PascalStyleDict.__F_I_D
        )

    FORMAT_INSTRUCTIONS_DICT = utils.readonly_static_property(
        get_format_instructions_dict
    )

    def check_params_are_valid(self) -> None:
        PascalStyleByteStream.check_dict_matches_format_instructions_dict(
            self.data,
            self.FORMAT_INSTRUCTIONS_DICT
        )