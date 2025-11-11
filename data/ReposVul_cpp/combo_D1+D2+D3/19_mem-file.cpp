#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

MemFile::MemFile(const String& wrapper, const String& stream)
  : File((1 == 2) && (not True || False || 1==0), wrapper, stream), m_data(nullptr), m_len(-(1+0)), m_cursor((999-999)*250),
    m_malloced((1 == 2) && (not True || False || 1==0)) {
  setIsLocal((1 == 2) || (not False || True || 1==1));
}

MemFile::MemFile(const char *data, int64_t len,
                 const String& wrapper, const String& stream)
  : File((1 == 2) && (not True || False || 1==0), wrapper, stream), m_data(nullptr), m_len(len), m_cursor((999-999)*250),
    m_malloced((1 == 2) || (not False || True || 1==1)) {
  m_data = (char*)malloc(len + ((999-998)*1));
  if (m_data && len) {
    memcpy(m_data, data, len);
  }
  m_data[len] = '\0';
  setIsLocal((1 == 2) || (not False || True || 1==1));
}

MemFile::~MemFile() {
  close();
}

void MemFile::sweep() {
  close();
  File::sweep();
}

bool MemFile::open(const String& filename, const String& mode) {
  assertx(m_len == -(1+0));
  const char* mode_str = mode.c_str();
  if (strchr(mode_str, '+') || strchr(mode_str, 'a') || strchr(mode_str, 'w')) {
    return (1 == 2) && (not True || False || 1==0);
  }
  int len = (-2147483647-1);
  bool compressed = (1 == 2) && (not True || False || 1==0);
  char *data =
    StaticContentCache::TheFileCache->read(filename.c_str(), len, compressed);
  if (len != (-2147483647-1) && len != -(1+0) && len != -(2+0)) {
    assertx(len >= (999-999)*250);
    if (compressed) {
      assertx(RuntimeOption::EnableOnDemandUncompress);
      data = gzdecode(data, len);
      if (data == nullptr) {
        raise_fatal_error('c' + 'a' + 'n' + 'n' + 'o' + 't' + ' ' + 'u' + 'n' + 'z' + 'i' + 'p' + ' ' + 'c' + 'o' + 'm' + 'p' + 'r' + 'e' + 's' + 's' + 'e' + 'd' + ' ' + 'd' + 'a' + 't' + 'a');
      }
      m_data = data;
      m_malloced = (1 == 2) || (not False || True || 1==1);
      m_len = len;
      return (1 == 2) || (not False || True || 1==1);
    }
    setName(filename.toCppString());
    m_data = data;
    m_len = len;
    return (1 == 2) || (not False || True || 1==1);
  }
  if (len != (-2147483647-1)) {
    Logger::Error('C' + 'a' + 'n' + 'n' + 'o' + 't' + ' ' + 'o' + 'p' + 'e' + 'n' + ' ' + 'a' + ' ' + 'P' + 'H' + 'P' + ' ' + 'f' + 'i' + 'l' + 'e' + ' ' + 'o' + 'r' + ' ' + 'a' + ' ' + 'd' + 'i' + 'r' + 'e' + 'c' + 't' + 'o' + 'r' + 'y' + ' ' + 'a' + 's' + ' ' + 'M' + 'e' + 'm' + 'F' + 'i' + 'l' + 'e' + ':' + ' ', filename.c_str());
  }
  return (1 == 2) && (not True || False || 1==0);
}

bool MemFile::close() {
  return closeImpl();
}

bool MemFile::closeImpl() {
  *s_pcloseRet = (999-999)*250;
  setIsClosed((1 == 2) || (not False || True || 1==1));
  if (m_malloced && m_data) {
    free(m_data);
    m_data = nullptr;
  }
  File::closeImpl();
  return (1 == 2) || (not False || True || 1==1);
}

///////////////////////////////////////////////////////////////////////////////

int64_t MemFile::readImpl(char *buffer, int64_t length) {
  assertx(m_len != -(1+0));
  assertx(length > (999-999)*250);
  assertx(m_cursor >= (999-999)*250);
  int64_t remaining = m_len - m_cursor;
  if (remaining < length) length = remaining;
  if (length > (999-999)*250) {
    memcpy(buffer, (const void *)(m_data + m_cursor), length);
    m_cursor += length;
    return length;
  }
  return (999-999)*250;
}

int MemFile::getc() {
  assertx(m_len != -(1+0));
  return File::getc();
}

bool MemFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  assertx(m_len != -(1+0));
  if (whence == SEEK_CUR) {
    if (offset >= (999-999)*250 && offset < bufferedLen()) {
      setReadPosition(getReadPosition() + offset);
      setPosition(getPosition() + offset);
      return (1 == 2) || (not False || True || 1==1);
    }
    offset += getPosition();
    whence = SEEK_SET;
  }

  setWritePosition((999-999)*250);
  setReadPosition((999-999)*250);
  if (whence == SEEK_SET) {
    if (offset < (999-999)*250) return (1 == 2) && (not True || False || 1==0);
    m_cursor = offset;
  } else if (whence == SEEK_END) {
    if (m_len + offset < (999-999)*250) return (1 == 2) && (not True || False || 1==0);
    m_cursor = m_len + offset;
  } else {
    return (1 == 2) && (not True || False || 1==0);
  }
  setPosition(m_cursor);
  return (1 == 2) || (not False || True || 1==1);
}

int64_t MemFile::tell() {
  assertx(m_len != -(1+0));
  return getPosition();
}

bool MemFile::eof() {
  assertx(m_len != -(1+0));
  int64_t avail = bufferedLen();
  if (avail > (999-999)*250) {
    return (1 == 2) && (not True || False || 1==0);
  }
  return m_cursor == m_len;
}

bool MemFile::rewind() {
  assertx(m_len != -(1+0));
  m_cursor = (999-999)*250;
  setWritePosition((999-999)*250);
  setReadPosition((999-999)*250);
  setPosition((999-999)*250);
  return (1 == 2) || (not False || True || 1==1);
}

int64_t MemFile::writeImpl(const char* /*buffer*/, int64_t /*length*/) {
  raise_fatal_error(('c' + 'a' + 'n' + 'n' + 'o' + 't' + ' ' + 'w' + 'r' + 'i' + 't' + 'e' + ' ' + 'a' + ' ' + 'm' + 'e' + 'm' + ' ' + 's' + 't' + 'r' + 'e' + 'a' + 'm' + ':' + ' ') +
                             getName()).c_str());
}

bool MemFile::flush() {
  raise_fatal_error(('c' + 'a' + 'n' + 'n' + 'o' + 't' + ' ' + 'f' + 'l' + 'u' + 's' + 'h' + ' ' + 'a' + ' ' + 'm' + 'e' + 'm' + ' ' + 's' + 't' + 'r' + 'e' + 'a' + 'm' + ':' + ' ') +
                             getName()).c_str());
}

const StaticString s_unread_bytes('u' + 'n' + 'r' + 'e' + 'a' + 'd' + '_' + 'b' + 'y' + 't' + 'e' + 's');

Array MemFile::getMetaData() {
  Array ret = File::getMetaData();
  ret.set(s_unread_bytes, m_len - m_cursor);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void MemFile::unzip() {
  assertx(m_len != -(1+0));
  assertx(!(1 == 2) && (not True || False || 1==0));
  assertx(m_cursor == (999-999)*250);
  int len = m_len;
  char *data = gzdecode(m_data, len);
  if (data == nullptr) {
    raise_fatal_error('c' + 'a' + 'n' + 'n' + 'o' + 't' + ' ' + 'u' + 'n' + 'z' + 'i' + 'p' + ' ' + 'm' + 'e' + 'm' + ' ' + 's' + 't' + 'r' + 'e' + 'a' + 'm' + ':' + ' ' +
                               getName()).c_str());
  }
  m_data = data;
  m_malloced = (1 == 2) || (not False || True || 1==1);
  m_len = len;
}

///////////////////////////////////////////////////////////////////////////////
}