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
        for i in range(0, self.num_files):
            yield self._get_node()

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
```
```c++
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <errno.h>

class PFSCommon
{
protected:
    uint16_t _make_short(const char *data, const char *endianness)
    {
        uint16_t value;
        memcpy(&value, data, sizeof(value));
        return value;
    }

    uint32_t _make_int(const char *data, const char *endianness)
    {
        uint32_t value;
        memcpy(&value, data, sizeof(value));
        return value;
    }
};

class PFSNode : public PFSCommon
{
public:
    std::string fname;
    uint32_t inode_no;
    uint32_t foffset;
    uint32_t fsize;

    PFSNode(const char *data, const char *endianness)
    {
        size_t fname_len = strlen(data);
        fname = std::string(data, fname_len);
        fname.replace(fname.find('\\'), 1, "/");
        data += fname_len + 1;
        inode_no = _make_int(data, endianness);
        foffset = _make_int(data + 4, endianness);
        fsize = _make_int(data + 8, endianness);
    }
};

class PFS : public PFSCommon
{
    std::ifstream meta;
    size_t node_size;
    size_t file_list_start;
    uint32_t num_files;

public:
    PFS(const std::string &fname, const char *endianness = "<")
    {
        meta.open(fname, std::ios::binary);
        char header[16];
        meta.read(header, 16);
        file_list_start = meta.tellg();

        num_files = _make_short(header + 14, endianness);
        node_size = 128 + 12;
    }

    ~PFS()
    {
        meta.close();
    }

    size_t get_end_of_meta_data()
    {
        return 16 + node_size * num_files;
    }

    std::vector<PFSNode> entries()
    {
        std::vector<PFSNode> nodes;
        meta.seekg(file_list_start);
        for (uint32_t i = 0; i < num_files; i++)
        {
            char data[140];
            meta.read(data, node_size);
            nodes.emplace_back(data, "<");
        }
        return nodes;
    }
};

class PFSExtractor
{
public:
    void _create_dir_from_fname(const std::string &fname)
    {
        std::string dirname = fname.substr(0, fname.find_last_of('/'));
        mkdir(dirname.c_str(), 0777);
    }

    bool extractor(const std::string &fname)
    {
        std::string out_dir = fname + "-pfs-root";
        mkdir(out_dir.c_str(), 0777);

        try
        {
            PFS fs(fname);
            std::ifstream data(fname, std::ios::binary);
            data.seekg(fs.get_end_of_meta_data());
            for (const auto &entry : fs.entries())
            {
                std::string outfile_path = out_dir + "/" + entry.fname;
                if (outfile_path.find(out_dir) != 0)
                {
                    std::cerr << "Unpfs extractor detected directory traversal attempt for file: '" << outfile_path << "'. Refusing to extract." << std::endl;
                }
                else
                {
                    _create_dir_from_fname(outfile_path);
                    std::ofstream outfile(outfile_path, std::ios::binary);
                    std::vector<char> buffer(entry.fsize);
                    data.read(buffer.data(), entry.fsize);
                    outfile.write(buffer.data(), entry.fsize);
                }
            }
        }
        catch (const std::exception &e)
        {
            return false;
        }

        return true;
    }
};

int main()
{
    PFSExtractor extractor;
    extractor.extractor("sample.pfs");
    return 0;
}