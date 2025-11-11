import os
import errno
import struct
import binwalk.core.common
import binwalk.core.compat
import binwalk.core.plugin

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_instructions(self, instructions):
        self.instructions = instructions

    def push(self, value):
        self.stack.append(value)

    def pop(self):
        return self.stack.pop()

    def fetch(self):
        if self.pc < len(self.instructions):
            instruction = self.instructions[self.pc]
            self.pc += 1
            return instruction
        else:
            self.running = False
            return None

    def run(self):
        while self.running:
            instruction = self.fetch()
            if instruction is None:
                break
            op, *args = instruction
            getattr(self, f'op_{op}')(*args)

    def op_PUSH(self, value):
        self.push(value)

    def op_POP(self):
        self.pop()

    def op_ADD(self):
        b = self.pop()
        a = self.pop()
        self.push(a + b)

    def op_SUB(self):
        b = self.pop()
        a = self.pop()
        self.push(a - b)

    def op_JMP(self, address):
        self.pc = address

    def op_JZ(self, address):
        if self.pop() == 0:
            self.pc = address

    def op_LOAD(self, index):
        self.push(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.pop()

class PFSCommon(object):
    def __init__(self):
        self.vm = VM()

    def _make_short(self, data, endianness):
        vm_instructions = [
            ('PUSH', binwalk.core.compat.str2bytes(data)),
            ('PUSH', '%sH' % endianness),
            ('PUSH', 0),
            ('LOAD', 2),
            ('LOAD', 1),
            ('STORE', 0),
            ('POP',),
        ]
        self.vm.load_instructions(vm_instructions)
        self.vm.run()
        return struct.unpack(self.vm.stack.pop(), self.vm.stack.pop())[0]

    def _make_int(self, data, endianness):
        vm_instructions = [
            ('PUSH', binwalk.core.compat.str2bytes(data)),
            ('PUSH', '%sI' % endianness),
            ('PUSH', 0),
            ('LOAD', 2),
            ('LOAD', 1),
            ('STORE', 0),
            ('POP',),
        ]
        self.vm.load_instructions(vm_instructions)
        self.vm.run()
        return struct.unpack(self.vm.stack.pop(), self.vm.stack.pop())[0]

class PFS(PFSCommon):
    HEADER_SIZE = 16

    def __init__(self, fname, endianness='<'):
        super().__init__()
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
        for i in range(0, self.num_files):
            yield self._get_node()

    def __enter__(self):
        return self

    def __exit__(self, type, value, traceback):
        self.meta.close()

class PFSNode(PFSCommon):
    def __init__(self, data, endianness):
        super().__init__()
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

        try:
            with PFS(fname) as fs:
                data = binwalk.core.common.BlockFile(fname, 'rb')
                data.seek(fs.get_end_of_meta_data())
                for entry in fs.entries():
                    outfile_path = os.path.abspath(os.path.join(out_dir, entry.fname))
                    if not outfile_path.startswith(out_dir):
                        binwalk.core.common.warning("Unpfs extractor detected directory traversal attempt for file: '%s'. Refusing to extract." % outfile_path)
                    else:
                        self._create_dir_from_fname(outfile_path)
                        outfile = binwalk.core.common.BlockFile(outfile_path, 'wb')
                        outfile.write(data.read(entry.fsize))
                        outfile.close()
                data.close()
        except KeyboardInterrupt as e:
            raise e
        except Exception as e:
            return False

        return True