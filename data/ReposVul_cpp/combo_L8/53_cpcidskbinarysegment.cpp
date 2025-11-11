import ctypes
import os

# Load the shared library into ctypes
lib = ctypes.CDLL(os.path.abspath("libcpcidsk.so"))

class CPCIDSKBinarySegment:
    def __init__(self, fileIn, segmentIn, segment_pointer, bLoad=True):
        self.fileIn = fileIn
        self.segmentIn = segmentIn
        self.segment_pointer = segment_pointer
        self.loaded_ = False
        self.mbModified = False

        if bLoad:
            self.Load()

    def __del__(self):
        pass

    def Load(self):
        if self.loaded_:
            return

        data_size = ctypes.c_uint64.in_dll(lib, "data_size").value
        if data_size - 1024 > ctypes.c_uint64(ctypes.c_int.max).value:
            self.ThrowPCIDSKException("too large data_size")
            return

        self.seg_data = ctypes.create_string_buffer(data_size - 1024)
        lib.ReadFromFile(self.seg_data, 0, data_size - 1024)

        self.loaded_ = True

    def Write(self):
        if not self.loaded_:
            return

        lib.WriteToFile(self.seg_data, 0, ctypes.sizeof(self.seg_data))

        self.mbModified = False

    def Synchronize(self):
        if self.mbModified:
            self.Write()

    def SetBuffer(self, pabyBuf, nBufSize):
        nNumBlocks = nBufSize // 512 + (0 if nBufSize % 512 == 0 else 1)
        nAllocBufSize = 512 * nNumBlocks

        self.seg_data = ctypes.create_string_buffer(nAllocBufSize)
        data_size = nAllocBufSize + 1024

        ctypes.memmove(self.seg_data, pabyBuf, nBufSize)

        if nBufSize < nAllocBufSize:
            ctypes.memset(ctypes.addressof(self.seg_data) + nBufSize, 0, nAllocBufSize - nBufSize)

        self.mbModified = True

    def ThrowPCIDSKException(self, message):
        raise Exception(message)