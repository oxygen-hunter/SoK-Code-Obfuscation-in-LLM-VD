import os
import errno
import struct
import binwalk.core.common
import binwalk.core.compat
import binwalk.core.plugin

class OX1A2B3C4D(object):

    def OX7E9F8D6C(self, OX5C3B1A, OX3D7F2E):
        OX5C3B1A = binwalk.core.compat.str2bytes(OX5C3B1A)
        return struct.unpack('%sH' % OX3D7F2E, OX5C3B1A)[0]

    def OX2B3A4C5D(self, OX5C3B1A, OX3D7F2E):
        OX5C3B1A = binwalk.core.compat.str2bytes(OX5C3B1A)
        return struct.unpack('%sI' % OX3D7F2E, OX5C3B1A)[0]

class OX9E8C7B6A(OX1A2B3C4D):
    HEADER_SIZE = 16

    def __init__(self, OX9F8E7D6C, OX3D7F2E='<'):
        self.OX3D7F2E = OX3D7F2E
        self.OX9F8E4D3C = binwalk.core.common.BlockFile(OX9F8E7D6C, 'rb')
        OX8E7D9C6B = self.OX9F8E4D3C.read(self.HEADER_SIZE)
        self.OX4D3C2B1A = self.OX9F8E4D3C.tell()

        self.OX3B2A1C0B = self.OX7E9F8D6C(OX8E7D9C6B[-2:], OX3D7F2E)
        self.OX1C0B9A8D = self.OX6D5C4B3A() + 12

    def OX6D5C4B3A(self, OX0B9A8D7C=128):
        OX9C8B7A6D = self.OX9F8E4D3C.peek(OX0B9A8D7C)
        OX8D7C6B5A = OX9C8B7A6D.find('\0')
        for OX5A4B3C2D, OX4A3B2C1D in enumerate(OX9C8B7A6D[OX8D7C6B5A:]):
            if OX4A3B2C1D != '\0':
                return OX8D7C6B5A+OX5A4B3C2D
        return OX0B9A8D7C

    def OX5C4B3A2D(self):
        OX5C3B1A = self.OX9F8E4D3C.read(self.OX1C0B9A8D)
        return OX8E7D9C6A(OX5C3B1A, self.OX3D7F2E)

    def OX9B8A7C6D(self):
        return self.HEADER_SIZE + self.OX1C0B9A8D * self.OX3B2A1C0B

    def OX4A3B2C1D(self):
        self.OX9F8E4D3C.seek(self.OX4D3C2B1A)
        for OX5A4B3C2D in range(0, self.OX3B2A1C0B):
            yield self.OX5C4B3A2D()

    def __enter__(self):
        return self

    def __exit__(self, OX1A0B9C8D, OX7C6B5A4B, OX3B2A1D0C):
        self.OX9F8E4D3C.close()

class OX8E7D9C6A(OX1A2B3C4D):

    def __init__(self, OX5C3B1A, OX3D7F2E):
        self.OX7D6C5B4A, OX5C3B1A = OX5C3B1A[:-12], OX5C3B1A[-12:]
        self.OX4B3A2D1C()
        self.OX2B1A0C9D = self.OX2B3A4C5D(OX5C3B1A[:4], OX3D7F2E)
        self.OX9C8B7A6D = self.OX2B3A4C5D(OX5C3B1A[4:8], OX3D7F2E)
        self.OX8D7C6B5A = self.OX2B3A4C5D(OX5C3B1A[8:], OX3D7F2E)

    def OX4B3A2D1C(self):
        self.OX7D6C5B4A = self.OX7D6C5B4A[:self.OX7D6C5B4A.find('\0')]
        self.OX7D6C5B4A = self.OX7D6C5B4A.replace('\\', '/')

class OX0A9B8C7D(binwalk.core.plugin.Plugin):

    MODULES = ['Signature']

    def init(self):
        if self.module.extractor.enabled:
            self.module.extractor.add_rule(regex='^pfs filesystem',
                                           extension='pfs',
                                           cmd=self.OX9F8E6D5C)

    def OX9E8C7B6D(self, OX7D6C5B4A):
        try:
            os.makedirs(os.path.dirname(OX7D6C5B4A))
        except OSError as OX4A3B2C1D:
            if OX4A3B2C1D.errno != errno.EEXIST:
                raise OX4A3B2C1D

    def OX9F8E6D5C(self, OX7D6C5B4A):
        OX7D6C5B4A = os.path.abspath(OX7D6C5B4A)
        OX8E7D9C6B = binwalk.core.common.unique_file_name(os.path.join(os.path.dirname(OX7D6C5B4A), "pfs-root"))

        try:
            with OX9E8C7B6A(OX7D6C5B4A) as OX6B5A4B3C:
                OX5C3B1A = binwalk.core.common.BlockFile(OX7D6C5B4A, 'rb')
                OX5C3B1A.seek(OX6B5A4B3C.OX9B8A7C6D())
                for OX4A3B2D1C in OX6B5A4B3C.OX4A3B2C1D():
                    OX2B1A0C9D = os.path.abspath(os.path.join(OX8E7D9C6B, OX4A3B2D1C.OX7D6C5B4A))
                    if not OX2B1A0C9D.startswith(OX8E7D9C6B):
                        binwalk.core.common.warning("Unpfs extractor detected directory traversal attempt for file: '%s'. Refusing to extract." % OX2B1A0C9D)
                    else:
                        self.OX9E8C7B6D(OX2B1A0C9D)
                        OX5C4B3A2D = binwalk.core.common.BlockFile(OX2B1A0C9D, 'wb')
                        OX5C4B3A2D.write(OX5C3B1A.read(OX4A3B2D1C.OX8D7C6B5A))
                        OX5C4B3A2D.close()
                OX5C3B1A.close()
        except KeyboardInterrupt as OX4A3B2C1D:
            raise OX4A3B2C1D
        except Exception as OX4A3B2C1D:
            return False

        return True