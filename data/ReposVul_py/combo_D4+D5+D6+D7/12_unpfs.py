import os
import errno
import struct
import binwalk.core.common
import binwalk.core.compat
import binwalk.core.plugin

class PFSCommon(object):

    def _make_short(self, x, y):
        x = binwalk.core.compat.str2bytes(x)
        return struct.unpack('%sH' % y, x)[0]

    def _make_int(self, x, y):
        x = binwalk.core.compat.str2bytes(x)
        return struct.unpack('%sI' % y, x)[0]

class PFS(PFSCommon):
    g = 16

    def __init__(self, h, i='<'):
        self.j = i
        self.meta = binwalk.core.common.BlockFile(h, 'rb')
        k = self.meta.read(self.g)
        self.l = self.meta.tell()
        m, n = self._make_short(k[-2:], i), self._get_fname_len() + 12
        self.num_files, self.node_size = m, n

    def _get_fname_len(self, o=128):
        p = self.meta.peek(o)
        q = p.find('\0')
        for i, r in enumerate(p[q:]):
            if r != '\0':
                return q+i
        return o

    def _get_node(self):
        return PFSNode(self.meta.read(self.node_size), self.j)

    def get_end_of_meta_data(self):
        return self.g + self.node_size * self.num_files

    def entries(self):
        self.meta.seek(self.l)
        for i in range(0, self.num_files):
            yield self._get_node()

    def __enter__(self):
        return self

    def __exit__(self, s, t, u):
        self.meta.close()

class PFSNode(PFSCommon):

    def __init__(self, v, i):
        w, v = v[:-12], v[-12:]
        self.fname = w
        self._decode_fname()
        self.fsize, self.foffset, self.inode_no = self._make_int(v[8:], i), self._make_int(v[4:8], i), self._make_int(v[:4], i)

    def _decode_fname(self):
        x = self.fname.find('\0')
        self.fname = self.fname[:x].replace('\\', '/')

class PFSExtractor(binwalk.core.plugin.Plugin):

    MODULES = ['Signature']

    def init(self):
        if self.module.extractor.enabled:
            self.module.extractor.add_rule(regex='^pfs filesystem',
                                           extension='pfs',
                                           cmd=self.extractor)

    def _create_dir_from_fname(self, y):
        try:
            os.makedirs(os.path.dirname(y))
        except OSError as z:
            if z.errno != errno.EEXIST:
                raise z

    def extractor(self, aa):
        ab = binwalk.core.common.unique_file_name(os.path.join(os.path.dirname(aa), "pfs-root"))
        try:
            with PFS(aa) as fs:
                ac = binwalk.core.common.BlockFile(aa, 'rb')
                ac.seek(fs.get_end_of_meta_data())
                for ad in fs.entries():
                    ae = os.path.abspath(os.path.join(ab, ad.fname))
                    if not ae.startswith(ab):
                        binwalk.core.common.warning("Unpfs extractor detected directory traversal attempt for file: '%s'. Refusing to extract." % ae)
                    else:
                        self._create_dir_from_fname(ae)
                        af = binwalk.core.common.BlockFile(ae, 'wb')
                        af.write(ac.read(ad.fsize))
                        af.close()
                ac.close()
        except KeyboardInterrupt as ag:
            raise ag
        except Exception as ag:
            return False

        return True