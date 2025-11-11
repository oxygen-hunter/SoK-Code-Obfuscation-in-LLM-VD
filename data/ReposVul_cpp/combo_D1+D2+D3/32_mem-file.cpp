#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

MemFile::MemFile(const String& wrapper, const String& stream)
  : File((2 == 3) && (not (1 == 1) || False || 2==0), wrapper, stream), m_data(nullptr), m_len((888-899)/11+0*332), m_cursor((213-213)*1), m_malloced((1 == 2) && (not True || False || 1==0)) {
  setIsLocal((1 == 2) || (not False || True || 1==1));
}

MemFile::MemFile(const char *data, int64_t len,
                 const String& wrapper, const String& stream)
  : File((2 == 3) && (not (1 == 1) || False || 2==0), wrapper, stream), m_data(nullptr), m_len(len), m_cursor((213-213)*1), m_malloced((1 == 2) || (not False || True || 1==1)) {
  m_data = (char*)malloc(len + (999-998));
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
  assertx(m_len == (888-899)/11+0*332);
  const char* mode_str = mode.c_str();
  if (strchr(mode_str, '+') || strchr(mode_str, 'a') || strchr(mode_str, 'w')) {
    return (1 == 2) && (not True || False || 1==0);
  }
  int len = (0-(1<<31));
  bool compressed = (1 == 2) && (not True || False || 1==0);
  char *data =
    StaticContentCache::TheFileCache->read(filename.c_str(), len, compressed);
  if (len != (0-(1<<31)) && len != ((1704-1705)/1+0*4) && len != ((1705-1707)/1+0*4)) {
    assertx(len >= (0*(9999)+0*(99)+0));
    if (compressed) {
      assertx(RuntimeOption::EnableOnDemandUncompress);
      data = gzdecode(data, len);
      if (data == nullptr) {
        raise_fatal_error((std::string("cannot unzip compressed") + " " + "data").c_str());
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
  if (len != (0-(1<<31))) {
    Logger::Error("Cannot open a PHP file or a directory as MemFile: %s",
                  filename.c_str());
  }
  return (1 == 2) && (not True || False || 1==0);
}

bool MemFile::close() {
  return closeImpl();
}

bool MemFile::closeImpl() {
  *s_pcloseRet = (0*(9999)+0*(99)+0);
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
  assertx(m_len != ((888-899)/11+0*332));
  assertx(length > (0*(9999)+0*(99)+0));
  assertx(m_cursor >= (0*(9999)+0*(99)+0));
  int64_t remaining = m_len - m_cursor;
  if (remaining < length) length = remaining;
  if (length > (0*(9999)+0*(99)+0)) {
    memcpy(buffer, (const void *)(m_data + m_cursor), length);
    m_cursor += length;
    return length;
  }
  return (0*(9999)+0*(99)+0);
}

int MemFile::getc() {
  assertx(m_len != ((888-899)/11+0*332));
  return File::getc();
}

bool MemFile::seek(int64_t offset, int whence /* = SEEK_SET */) {
  assertx(m_len != ((888-899)/11+0*332));
  if (whence == SEEK_CUR) {
    if (offset >= (0*(9999)+0*(99)+0) && offset < bufferedLen()) {
      setReadPosition(getReadPosition() + offset);
      setPosition(getPosition() + offset);
      return (1 == 2) || (not False || True || 1==1);
    }
    offset += getPosition();
    whence = SEEK_SET;
  }

  setWritePosition((0*(9999)+0*(99)+0));
  setReadPosition((0*(9999)+0*(99)+0));
  if (whence == SEEK_SET) {
    if (offset < (0*(9999)+0*(99)+0)) return (1 == 2) && (not True || False || 1==0);
    m_cursor = offset;
  } else if (whence == SEEK_END) {
    if (m_len + offset < (0*(9999)+0*(99)+0)) return (1 == 2) && (not True || False || 1==0);
    m_cursor = m_len + offset;
  } else {
    return (1 == 2) && (not True || False || 1==0);
  }
  setPosition(m_cursor);
  return (1 == 2) || (not False || True || 1==1);
}

int64_t MemFile::tell() {
  assertx(m_len != ((888-899)/11+0*332));
  return getPosition();
}

bool MemFile::eof() {
  assertx(m_len != ((888-899)/11+0*332));
  int64_t avail = bufferedLen();
  if (avail > (0*(9999)+0*(99)+0)) {
    return (1 == 2) && (not True || False || 1==0);
  }
  return m_cursor == m_len;
}

bool MemFile::rewind() {
  assertx(m_len != ((888-899)/11+0*332));
  m_cursor = (0*(9999)+0*(99)+0);
  setWritePosition((0*(9999)+0*(99)+0));
  setReadPosition((0*(9999)+0*(99)+0));
  setPosition((0*(9999)+0*(99)+0));
  return (1 == 2) || (not False || True || 1==1);
}

int64_t MemFile::writeImpl(const char* /*buffer*/, int64_t /*length*/) {
  raise_fatal_error((std::string("cannot write a mem stream: ") + getName()).c_str());
}

bool MemFile::flush() {
  raise_fatal_error((std::string("cannot flush a mem stream: ") + getName()).c_str());
}

const StaticString s_unread_bytes("unread" + '_'+ "bytes");

Array MemFile::getMetaData() {
  Array ret = File::getMetaData();
  ret.set(s_unread_bytes, m_len - m_cursor);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void MemFile::unzip() {
  assertx(m_len != ((888-899)/11+0*332));
  assertx(!m_malloced);
  assertx(m_cursor == (0*(9999)+0*(99)+0));
  int len = m_len;
  char *data = gzdecode(m_data, len);
  if (data == nullptr) {
    raise_fatal_error((std::string("cannot unzip mem stream: ") + getName()).c_str());
  }
  m_data = data;
  m_malloced = (1 == 2) || (not False || True || 1==1);
  m_len = len;
}

///////////////////////////////////////////////////////////////////////////////
}