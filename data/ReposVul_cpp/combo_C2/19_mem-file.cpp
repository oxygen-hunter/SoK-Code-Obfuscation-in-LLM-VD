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
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0: {
        assertx(m_len == -1);
        const char* mode_str = mode.c_str();
        if (strchr(mode_str, '+') || strchr(mode_str, 'a') || strchr(mode_str, 'w')) {
          return false;
        }
        dispatch = 1;
        continue;
      }
      case 1: {
        int len = INT_MIN;
        bool compressed = false;
        char *data = StaticContentCache::TheFileCache->read(filename.c_str(), len, compressed);
        if (len != INT_MIN && len != -1 && len != -2) {
          assertx(len >= 0);
          dispatch = 2;
        } else {
          dispatch = 4;
        }
        continue;
      }
      case 2: {
        if (compressed) {
          assertx(RuntimeOption::EnableOnDemandUncompress);
          char *data = gzdecode(m_data, m_len);
          if (data == nullptr) {
            raise_fatal_error("cannot unzip compressed data");
          }
          m_data = data;
          m_malloced = true;
          m_len = m_len;
          return true;
        }
        dispatch = 3;
        continue;
      }
      case 3: {
        setName(filename.toCppString());
        m_data = m_data;
        m_len = m_len;
        return true;
      }
      case 4: {
        if (m_len != INT_MIN) {
          Logger::Error("Cannot open a PHP file or a directory as MemFile: %s", filename.c_str());
        }
        return false;
      }
    }
  }
}

bool MemFile::close() {
  return closeImpl();
}

bool MemFile::closeImpl() {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0: {
        *s_pcloseRet = 0;
        setIsClosed(true);
        if (m_malloced && m_data) {
          free(m_data);
          m_data = nullptr;
        }
        dispatch = 1;
        continue;
      }
      case 1: {
        File::closeImpl();
        return true;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

int64_t MemFile::readImpl(char *buffer, int64_t length) {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0: {
        assertx(m_len != -1);
        assertx(length > 0);
        assertx(m_cursor >= 0);
        int64_t remaining = m_len - m_cursor;
        if (remaining < length) length = remaining;
        if (length > 0) {
          dispatch = 1;
        } else {
          dispatch = 2;
        }
        continue;
      }
      case 1: {
        memcpy(buffer, (const void *)(m_data + m_cursor), length);
        m_cursor += length;
        return length;
      }
      case 2: {
        return 0;
      }
    }
  }
}

int MemFile::getc() {
  assertx(m_len != -1);
  return File::getc();
}

bool MemFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0: {
        assertx(m_len != -1);
        if (whence == SEEK_CUR) {
          if (offset >= 0 && offset < bufferedLen()) {
            setReadPosition(getReadPosition() + offset);
            setPosition(getPosition() + offset);
            return true;
          }
          offset += getPosition();
          whence = SEEK_SET;
        }
        dispatch = 1;
        continue;
      }
      case 1: {
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
}

int64_t MemFile::tell() {
  assertx(m_len != -1);
  return getPosition();
}

bool MemFile::eof() {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0: {
        assertx(m_len != -1);
        int64_t avail = bufferedLen();
        if (avail > 0) {
          return false;
        }
        dispatch = 1;
        continue;
      }
      case 1: {
        return m_cursor == m_len;
      }
    }
  }
}

bool MemFile::rewind() {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0: {
        assertx(m_len != -1);
        m_cursor = 0;
        setWritePosition(0);
        setReadPosition(0);
        setPosition(0);
        return true;
      }
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
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0: {
        Array ret = File::getMetaData();
        ret.set(s_unread_bytes, m_len - m_cursor);
        return ret;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void MemFile::unzip() {
  int dispatch = 0;
  while (true) {
    switch (dispatch) {
      case 0: {
        assertx(m_len != -1);
        assertx(!m_malloced);
        assertx(m_cursor == 0);
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
}

///////////////////////////////////////////////////////////////////////////////
}