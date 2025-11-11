#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

MemFile::MemFile(const String& wrapper, const String& stream)
  : File(false, wrapper, stream), m_data(nullptr), m_len(-1), m_cursor(0),
    m_malloced(false) {
  setIsLocal(true);
}

MemFile::MemFile(const char *data, int64_t len,
                 const String& wrapper, const String& stream)
  : File(false, wrapper, stream), m_data(nullptr), m_len(len), m_cursor(0),
    m_malloced(true) {
  m_data = (char*)malloc(len + 1);
  if (m_data && len) {
    memcpy(m_data, data, len);
  }
  m_data[len] = '\0';
  setIsLocal(true);
}

MemFile::~MemFile() {
  close();
}

void MemFile::sweep() {
  close();
  File::sweep();
}

bool MemFile::open(const String& filename, const String& mode) {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        assertx(m_len == -1);
        dispatcher = 1;
        break;
      case 1:
        const char* mode_str = mode.c_str();
        if (strchr(mode_str, '+') || strchr(mode_str, 'a') || strchr(mode_str, 'w')) {
          return false;
        }
        dispatcher = 2;
        break;
      case 2:
        int len = INT_MIN;
        bool compressed = false;
        char *data = StaticContentCache::TheFileCache->read(filename.c_str(), len, compressed);
        if (len != INT_MIN && len != -1 && len != -2) {
          dispatcher = 3;
        } else {
          dispatcher = 6;
        }
        break;
      case 3:
        assertx(len >= 0);
        if (compressed) {
          dispatcher = 4;
        } else {
          dispatcher = 5;
        }
        break;
      case 4:
        assertx(RuntimeOption::EnableOnDemandUncompress);
        data = gzdecode(data, len);
        if (data == nullptr) {
          raise_fatal_error("cannot unzip compressed data");
        }
        m_data = data;
        m_malloced = true;
        m_len = len;
        return true;
      case 5:
        setName(filename.toCppString());
        m_data = data;
        m_len = len;
        return true;
      case 6:
        if (len != INT_MIN) {
          Logger::Error("Cannot open a PHP file or a directory as MemFile: %s", filename.c_str());
        }
        return false;
    }
  }
}

bool MemFile::close() {
  return closeImpl();
}

bool MemFile::closeImpl() {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        *s_pcloseRet = 0;
        setIsClosed(true);
        dispatcher = 1;
        break;
      case 1:
        if (m_malloced && m_data) {
          free(m_data);
          m_data = nullptr;
        }
        File::closeImpl();
        return true;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

int64_t MemFile::readImpl(char *buffer, int64_t length) {
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        assertx(m_len != -1);
        assertx(length > 0);
        assertx(m_cursor >= 0);
        dispatcher = 1;
        break;
      case 1:
        int64_t remaining = m_len - m_cursor;
        if (remaining < length) length = remaining;
        if (length > 0) {
          memcpy(buffer, (const void *)(m_data + m_cursor), length);
          m_cursor += length;
          return length;
        }
        return 0;
    }
  }
}

int MemFile::getc() {
  assertx(m_len != -1);
  return File::getc();
}

bool MemFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  assertx(m_len != -1);
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (whence == SEEK_CUR) {
          if (offset >= 0 && offset < bufferedLen()) {
            setReadPosition(getReadPosition() + offset);
            setPosition(getPosition() + offset);
            return true;
          }
          offset += getPosition();
          whence = SEEK_SET;
        }
        dispatcher = 1;
        break;
      case 1:
        setWritePosition(0);
        setReadPosition(0);
        if (whence == SEEK_SET) {
          if (offset < 0) return false;
          m_cursor = offset;
        } else if (whence == SEEK_END) {
          if (m_len + offset < 0) return false;
          m_cursor = m_len + offset;
        } else {
          return false;
        }
        setPosition(m_cursor);
        return true;
    }
  }
}

int64_t MemFile::tell() {
  assertx(m_len != -1);
  return getPosition();
}

bool MemFile::eof() {
  assertx(m_len != -1);
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        int64_t avail = bufferedLen();
        if (avail > 0) {
          return false;
        }
        return m_cursor == m_len;
    }
  }
}

bool MemFile::rewind() {
  assertx(m_len != -1);
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        m_cursor = 0;
        setWritePosition(0);
        setReadPosition(0);
        setPosition(0);
        return true;
    }
  }
}

int64_t MemFile::writeImpl(const char* /*buffer*/, int64_t /*length*/) {
  raise_fatal_error((std::string("cannot write a mem stream: ") +
                             getName()).c_str());
}

bool MemFile::flush() {
  raise_fatal_error((std::string("cannot flush a mem stream: ") +
                             getName()).c_str());
}

const StaticString s_unread_bytes("unread_bytes");

Array MemFile::getMetaData() {
  Array ret = File::getMetaData();
  ret.set(s_unread_bytes, m_len - m_cursor);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void MemFile::unzip() {
  assertx(m_len != -1);
  assertx(!m_malloced);
  assertx(m_cursor == 0);
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        int len = m_len;
        char *data = gzdecode(m_data, len);
        if (data == nullptr) {
          raise_fatal_error((std::string("cannot unzip mem stream: ") +
                                     getName()).c_str());
        }
        m_data = data;
        m_malloced = true;
        m_len = len;
        return;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}