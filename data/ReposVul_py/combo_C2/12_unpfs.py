import os
import errno
import struct
import binwalk.core.common
import binwalk.core.compat
import binwalk.core.plugin

class PFSCommon(object):

    def _make_short(self, data, endianness):
        data = binwalk.core.compat.str2bytes(data)
        return struct.unpack('%sH' % endianness, data)[0]

    def _make_int(self, data, endianness):
        data = binwalk.core.compat.str2bytes(data)
        return struct.unpack('%sI' % endianness, data)[0]

class PFS(PFSCommon):
    HEADER_SIZE = 16

    def __init__(self, fname, endianness='<'):
        self.endianness = endianness
        self.meta = binwalk.core.common.BlockFile(fname, 'rb')
        header = self.meta.read(self.HEADER_SIZE)
        self.file_list_start = self.meta.tell()

        self.num_files = self._make_short(header[-2:], endianness)
        self.node_size = self._get_fname_len() + 12

    def _get_fname_len(self, bufflen=128):
        buff = self.meta.peek(bufflen)
        strlen = buff.find('\0')
        for i, b in enumerate(buff[strlen:]):
            if b != '\0':
                return strlen+i
        return bufflen

    def _get_node(self):
        data = self.meta.read(self.node_size)
        return PFSNode(data, self.endianness)

    def get_end_of_meta_data(self):
        return self.HEADER_SIZE + self.node_size * self.num_files

    def entries(self):
        self.meta.seek(self.file_list_start)
        block = 0
        while block != -1:
            if block == 0:
                i = 0
                block = 1
            elif block == 1:
                if i < self.num_files:
                    block = 2
                else:
                    block = -1
            elif block == 2:
                yield self._get_node()
                i += 1
                block = 1

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self.meta.close()

class PFSNode(PFSCommon):

    def __init__(self, data, endianness):
        self.fname, data = data[:-12], data[-12:]
        self._decode_fname()
        self.inode_no = self._make_int(data[:4], endianness)
        self.foffset = self._make_int(data[4:8], endianness)
        self.fsize = self._make_int(data[8:], endianness)

    def _decode_fname(self):
        self.fname = self.fname[:self.fname.find('\0')]
        self.fname = self.fname.replace('\\', '/')

class PFSExtractor(binwalk.core.plugin.Plugin):
    MODULES = ['Signature']

    def init(self):
        if self.module.extractor.enabled:
            self.module.extractor.add_rule(regex='^pfs filesystem',
                                           extension='pfs',
                                           cmd=self.extractor)

    def _create_dir_from_fname(self, fname):
        try:
            os.makedirs(os.path.dirname(fname))
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise e

    def extractor(self, fname):
        fname = os.path.abspath(fname)
        out_dir = binwalk.core.common.unique_file_name(os.path.join(os.path.dirname(fname), "pfs-root"))
        block = 0

        while block != -1:
            if block == 0:
                try:
                    with PFS(fname) as fs:
                        data = binwalk.core.common.BlockFile(fname, 'rb')
                        data.seek(fs.get_end_of_meta_data())
                        entries_gen = fs.entries()
                        block = 1
                except KeyboardInterrupt as e:
                    raise e
                except Exception as e:
                    return False
            elif block == 1:
                try:
                    entry = next(entries_gen)
                    block = 2
                except StopIteration:
                    data.close()
                    block = -1
            elif block == 2:
                outfile_path = os.path.abspath(os.path.join(out_dir, entry.fname))
                block = 3
            elif block == 3:
                if not outfile_path.startswith(out_dir):
                    binwalk.core.common.warning("Unpfs extractor detected directory traversal attempt for file: '%s'. Refusing to extract." % outfile_path)
                    block = 1
                else:
                    self._create_dir_from_fname(outfile_path)
                    block = 4
            elif block == 4:
                outfile = binwalk.core.common.BlockFile(outfile_path, 'wb')
                outfile.write(data.read(entry.fsize))
                outfile.close()
                block = 1

        return True