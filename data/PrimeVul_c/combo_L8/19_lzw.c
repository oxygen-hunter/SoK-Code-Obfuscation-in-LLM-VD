import ctypes
import os

# Load the shared library
lzw_lib = ctypes.CDLL(os.path.join(os.path.dirname(__file__), 'lzw.so'))

class LZWDecoder(ctypes.Structure):
    pass

LZWDecoder._fields_ = [
    ('parent_instance', ctypes.c_void_p),
    ('min_code_size', ctypes.c_int),
    ('code_size', ctypes.c_int),
    ('clear_code', ctypes.c_int),
    ('eoi_code', ctypes.c_int),
    ('code_table', ctypes.c_byte * (1 << 12)),  # Assuming LZW_CODE_MAX is 12
    ('code_table_size', ctypes.c_int),
    ('code', ctypes.c_int),
    ('code_bits', ctypes.c_int),
    ('last_code', ctypes.c_int)
]

def add_code(self, code):
    c = code
    while self.code_table[c].extends != self.eoi_code:
        c = self.code_table[c].extends

    self.code_table[self.code_table_size].index = self.code_table[c].index
    self.code_table[self.code_table_size].extends = self.last_code
    self.code_table_size += 1

def write_indexes(self, output, output_length):
    c = self.code
    index_count = 1
    offset = 0

    if self.code >= self.code_table_size:
        return 0

    while self.code_table[c].extends != self.eoi_code:
        c = self.code_table[c].extends
        index_count += 1

    c = self.code
    offset = index_count - 1
    while True:
        if offset < output_length:
            output[offset] = self.code_table[c].index

        if self.code_table[c].extends == self.eoi_code:
            return index_count

        c = self.code_table[c].extends
        offset -= 1

def lzw_decoder_init(self):
    pass

def lzw_decoder_new(code_size):
    self = LZWDecoder()
    self.min_code_size = code_size
    self.code_size = code_size

    self.clear_code = 1 << (code_size - 1)
    self.eoi_code = self.clear_code + 1

    for i in range(self.eoi_code + 1):
        self.code_table[i].index = i
        self.code_table[i].extends = self.eoi_code
        self.code_table_size += 1

    self.code = 0
    self.last_code = self.clear_code

    return self

def lzw_decoder_feed(self, input_data, output, output_length):
    n_written = 0

    if self.last_code == self.eoi_code:
        return 0

    for byte in input_data:
        d = byte
        n_available = 8

        while n_available > 0:
            n_bits = min(self.code_size - self.code_bits, n_available)
            new_bits = d & ((1 << n_bits) - 1)
            d = d >> n_bits
            n_available -= n_bits

            self.code = new_bits << self.code_bits | self.code
            self.code_bits += n_bits
            if self.code_bits < self.code_size:
                continue

            if self.code == self.eoi_code:
                self.last_code = self.code
                return n_written

            if self.code == self.clear_code:
                self.code_table_size = self.eoi_code + 1
                self.code_size = self.min_code_size
            else:
                if self.last_code != self.clear_code and self.code_table_size < (1 << 12):
                    if self.code < self.code_table_size:
                        add_code(self, self.code)
                    elif self.code == self.code_table_size:
                        add_code(self, self.last_code)
                    else:
                        self.last_code = self.eoi_code
                        return output_length

                    if self.code_table_size == (1 << self.code_size) and self.code_size < 12:
                        self.code_size += 1

                n_written += write_indexes(self, output[n_written:], output_length - n_written)

            self.last_code = self.code
            self.code = 0
            self.code_bits = 0

            if n_written >= output_length:
                return output_length

    return n_written

lzw_decoder_class_init = ctypes.CFUNCTYPE(None, ctypes.POINTER(LZWDecoder))
lzw_decoder_init = ctypes.CFUNCTYPE(None, ctypes.POINTER(LZWDecoder))