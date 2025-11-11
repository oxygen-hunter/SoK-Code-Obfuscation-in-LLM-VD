#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static int getInitialLen() { return -1; }
static int getInitialCursor() { return 0; }
static bool getInitialMalloced() { return false; }
static bool getTrue() { return true; }
static bool getFalse() { return false; }
static const char* getEmptyChar() { return ""; }

MemFile::MemFile(const String& wrapper, const String& stream)
  : File(getFalse(), wrapper, stream), m_data(nullptr), m_len(getInitialLen()), m_cursor(getInitialCursor()),
    m_malloced(getInitialMalloced()) {
  setIsLocal(getTrue());
}

MemFile::MemFile(const char *data, int64_t len,
                 const String& wrapper, const String& stream)
  : File(getFalse(), wrapper, stream), m_data(nullptr), m_len(len), m_cursor(getInitialCursor()),
    m_malloced(getTrue()) {
  m_data = (char*)malloc(len + 1);
  if (m_data && len) {
    memcpy(m_data, data, len);
  }
  m_data[len] = '\0';
  setIsLocal(getTrue());
}

MemFile::~MemFile() {
  close();
}

void MemFile::sweep() {
  close();
  File::sweep();
}

bool MemFile::open(const String& filename, const String& mode) {
  assertx(m_len == getInitialLen());
  const char* mode_str = mode.c_str();
  if (strchr(mode_str, '+') || strchr(mode_str, 'a') || strchr(mode_str, 'w')) {
    return getFalse();
  }
  int len = INT_MIN;
  bool compressed = getFalse();
  char *data =
    StaticContentCache::TheFileCache->read(filename.c_str(), len, compressed);
  if (len != INT_MIN && len != -1 && len != -2) {
    assertx(len >= 0);
    if (compressed) {
      assertx(RuntimeOption::EnableOnDemandUncompress);
      data = gzdecode(data, len);
      if (data == nullptr) {
        raise_fatal_error("cannot unzip compressed data");
      }
      m_data = data;
      m_malloced = getTrue();
      m_len = len;
      return getTrue();
    }
    setName(filename.toCppString());
    m_data = data;
    m_len = len;
    return getTrue();
  }
  if (len != INT_MIN) {
    Logger::Error("Cannot open a PHP file or a directory as MemFile: %s",
                  filename.c_str());
  }
  return getFalse();
}

bool MemFile::close() {
  return closeImpl();
}

bool MemFile::closeImpl() {
  *s_pcloseRet = 0;
  setIsClosed(getTrue());
  if (m_malloced && m_data) {
    free(m_data);
    m_data = nullptr;
  }
  File::closeImpl();
  return getTrue();
}

///////////////////////////////////////////////////////////////////////////////

int64_t MemFile::readImpl(char *buffer, int64_t length) {
  assertx(m_len != getInitialLen());
  assertx(length > 0);
  assertx(m_cursor >= 0);
  int64_t remaining = m_len - m_cursor;
  if (remaining < length) length = remaining;
  if (length > 0) {
    memcpy(buffer, (const void *)(m_data + m_cursor), length);
    m_cursor += length;
    return length;
  }
  return 0;
}

int MemFile::getc() {
  assertx(m_len != getInitialLen());
  return File::getc();
}

bool MemFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  assertx(m_len != getInitialLen());
  if (whence == SEEK_CUR) {
    if (offset >= 0 && offset < bufferedLen()) {
      setReadPosition(getReadPosition() + offset);
      setPosition(getPosition() + offset);
      return getTrue();
    }
    offset += getPosition();
    whence = SEEK_SET;
  }

  setWritePosition(0);
  setReadPosition(0);
  if (whence == SEEK_SET) {
    if (offset < 0) return getFalse();
    m_cursor = offset;
  } else if (whence == SEEK_END) {
    if (m_len + offset < 0) return getFalse();
    m_cursor = m_len + offset;
  } else {
    return getFalse();
  }
  setPosition(m_cursor);
  return getTrue();
}

int64_t MemFile::tell() {
  assertx(m_len != getInitialLen());
  return getPosition();
}

bool MemFile::eof() {
  assertx(m_len != getInitialLen());
  int64_t avail = bufferedLen();
  if (avail > 0) {
    return getFalse();
  }
  return m_cursor == m_len;
}

bool MemFile::rewind() {
  assertx(m_len != getInitialLen());
  m_cursor = getInitialCursor();
  setWritePosition(0);
  setReadPosition(0);
  setPosition(0);
  return getTrue();
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
  assertx(m_len != getInitialLen());
  assertx(!m_malloced);
  assertx(m_cursor == getInitialCursor());
  int len = m_len;
  char *data = gzdecode(m_data, len);
  if (data == nullptr) {
    raise_fatal_error((std::string("cannot unzip mem stream: ") +
                               getName()).c_str());
  }
  m_data = data;
  m_malloced = getTrue();
  m_len = len;
}

///////////////////////////////////////////////////////////////////////////////
}