import ctypes

# Load the C library
lib = ctypes.CDLL('./bitstream_lib.so')

class JpcBitstream:
    def __init__(self, stream, mode):
        self.lib = lib
        self.obj = self.lib.jpc_bitstream_sopen(stream, mode.encode('utf-8'))
        if not self.obj:
            raise RuntimeError("Failed to open bitstream")

    def close(self):
        if self.lib.jpc_bitstream_close(self.obj) != 0:
            raise RuntimeError("Failed to close bitstream")

    def getbit(self):
        ret = self.lib.jpc_bitstream_getbit_func(self.obj)
        if ret < 0:
            raise RuntimeError("Failed to get bit")
        return ret

    def putbit(self, b):
        if self.lib.jpc_bitstream_putbit_func(self.obj, b) != 0:
            raise RuntimeError("Failed to put bit")

    def getbits(self, n):
        ret = self.lib.jpc_bitstream_getbits(self.obj, n)
        if ret < 0:
            raise RuntimeError("Failed to get bits")
        return ret

    def putbits(self, n, v):
        if self.lib.jpc_bitstream_putbits(self.obj, n, v) != 0:
            raise RuntimeError("Failed to put bits")

    def align(self):
        if self.lib.jpc_bitstream_align(self.obj) != 0:
            raise RuntimeError("Failed to align bitstream")

# Example Usage
# stream = some_stream_pointer
# bitstream = JpcBitstream(stream, 'r')
# bit = bitstream.getbit()
# bitstream.putbit(1)
# bitstream.close()