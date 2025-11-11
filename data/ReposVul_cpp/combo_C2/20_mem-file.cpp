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
  int state = 0;
  bool result = false;
  while (true) {
    switch (state) {
      case 0: {
        assertx(m_len == -1);
        const char* mode_str = mode.c_str();
        if (strchr(mode_str, '+') || strchr(mode_str, 'a') || strchr(mode_str, 'w')) {
          result = false;
          state = 5;
        } else {
          state = 1;
        }
        break;
      }
      case 1: {
        int len = INT_MIN;
        bool compressed = false;
        char *data = StaticContentCache::TheFileCache->read(filename.c_str(), len, compressed);
        if (len != INT_MIN && len != -1 && len != -2) {
          assertx(len >= 0);
          if (compressed) {
            assertx(RuntimeOption::EnableOnDemandUncompress);
            data = gzdecode(data, len);
            if (data == nullptr) {
              state = 3;
            } else {
              m_data = data;
              m_malloced = true;
              m_len = len;
              result = true;
              state = 5;
            }
          } else {
            setName(filename.toCppString());
            m_data = data;
            m_len = len;
            result = true;
            state = 5;
          }
        } else {
          state = 2;
        }
        break;
      }
      case 2: {
        if (len != INT_MIN) {
          Logger::Error("Cannot open a PHP file or a directory as MemFile: %s", filename.c_str());
        }
        result = false;
        state = 5;
        break;
      }
      case 3: {
        raise_fatal_error("cannot unzip compressed data");
        result = false;
        state = 5;
        break;
      }
      case 5: {
        return result;
      }
    }
  }
}

bool MemFile::close() {
  return closeImpl();
}

bool MemFile::closeImpl() {
  int state = 0;
  bool result = false;
  while (true) {
    switch (state) {
      case 0: {
        *s_pcloseRet = 0;
        setIsClosed(true);
        if (m_malloced && m_data) {
          free(m_data);
          m_data = nullptr;
        }
        state = 1;
        break;
      }
      case 1: {
        File::closeImpl();
        result = true;
        state = 2;
        break;
      }
      case 2: {
        return result;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

int64_t MemFile::readImpl(char *buffer, int64_t length) {
  int state = 0;
  int64_t result = 0;
  while (true) {
    switch (state) {
      case 0: {
        assertx(m_len != -1);
        assertx(length > 0);
        assertx(m_cursor >= 0);
        int64_t remaining = m_len - m_cursor;
        if (remaining < length) length = remaining;
        if (length > 0) {
          memcpy(buffer, (const void *)(m_data + m_cursor), length);
          m_cursor += length;
          result = length;
          state = 2;
        } else {
          result = 0;
          state = 2;
        }
        break;
      }
      case 2: {
        return result;
      }
    }
  }
}

int MemFile::getc() {
  int state = 0;
  int result = 0;
  while (true) {
    switch (state) {
      case 0: {
        assertx(m_len != -1);
        result = File::getc();
        state = 1;
        break;
      }
      case 1: {
        return result;
      }
    }
  }
}

bool MemFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  int state = 0;
  bool result = false;
  while (true) {
    switch (state) {
      case 0: {
        assertx(m_len != -1);
        if (whence == SEEK_CUR) {
          if (offset >= 0 && offset < bufferedLen()) {
            setReadPosition(getReadPosition() + offset);
            setPosition(getPosition() + offset);
            result = true;
            state = 6;
          } else {
            offset += getPosition();
            whence = SEEK_SET;
            state = 3;
          }
        } else {
          state = 3;
        }
        break;
      }
      case 3: {
        setWritePosition(0);
        setReadPosition(0);
        if (whence == SEEK_SET) {
          if (offset < 0) {
            result = false;
            state = 6;
          } else {
            m_cursor = offset;
            result = true;
            state = 6;
          }
        } else if (whence == SEEK_END) {
          if (m_len + offset < 0) {
            result = false;
            state = 6;
          } else {
            m_cursor = m_len + offset;
            result = true;
            state = 6;
          }
        } else {
          result = false;
          state = 6;
        }
        break;
      }
      case 6: {
        setPosition(m_cursor);
        return result;
      }
    }
  }
}

int64_t MemFile::tell() {
  int state = 0;
  int64_t result = 0;
  while (true) {
    switch (state) {
      case 0: {
        assertx(m_len != -1);
        result = getPosition();
        state = 1;
        break;
      }
      case 1: {
        return result;
      }
    }
  }
}

bool MemFile::eof() {
  int state = 0;
  bool result = false;
  while (true) {
    switch (state) {
      case 0: {
        assertx(m_len != -1);
        int64_t avail = bufferedLen();
        if (avail > 0) {
          result = false;
          state = 2;
        } else {
          result = m_cursor == m_len;
          state = 2;
        }
        break;
      }
      case 2: {
        return result;
      }
    }
  }
}

bool MemFile::rewind() {
  int state = 0;
  bool result = false;
  while (true) {
    switch (state) {
      case 0: {
        assertx(m_len != -1);
        m_cursor = 0;
        setWritePosition(0);
        setReadPosition(0);
        setPosition(0);
        result = true;
        state = 1;
        break;
      }
      case 1: {
        return result;
      }
    }
  }
}

int64_t MemFile::writeImpl(const char* /*buffer*/, int64_t /*length*/) {
  int state = 0;
  int64_t result = 0;
  while (true) {
    switch (state) {
      case 0: {
        raise_fatal_error((std::string("cannot write a mem stream: ") + getName()).c_str());
        result = 0;
        state = 1;
        break;
      }
      case 1: {
        return result;
      }
    }
  }
}

bool MemFile::flush() {
  int state = 0;
  bool result = false;
  while (true) {
    switch (state) {
      case 0: {
        raise_fatal_error((std::string("cannot flush a mem stream: ") + getName()).c_str());
        result = false;
        state = 1;
        break;
      }
      case 1: {
        return result;
      }
    }
  }
}

const StaticString s_unread_bytes("unread_bytes");

Array MemFile::getMetaData() {
  int state = 0;
  Array ret;
  while (true) {
    switch (state) {
      case 0: {
        ret = File::getMetaData();
        ret.set(s_unread_bytes, m_len - m_cursor);
        state = 1;
        break;
      }
      case 1: {
        return ret;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void MemFile::unzip() {
  int state = 0;
  while (true) {
    switch (state) {
      case 0: {
        assertx(m_len != -1);
        assertx(!m_malloced);
        assertx(m_cursor == 0);
        int len = m_len;
        char *data = gzdecode(m_data, len);
        if (data == nullptr) {
          state = 2;
        } else {
          m_data = data;
          m_malloced = true;
          m_len = len;
          state = 1;
        }
        break;
      }
      case 2: {
        raise_fatal_error((std::string("cannot unzip mem stream: ") + getName()).c_str());
        state = 1;
        break;
      }
      case 1: {
        return;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}