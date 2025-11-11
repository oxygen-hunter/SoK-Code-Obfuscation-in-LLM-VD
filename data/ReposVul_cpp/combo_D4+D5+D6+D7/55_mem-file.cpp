#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct MemFileParams {
  bool malloced;
  int64_t len;
  int64_t cursor;
  char* data;
};

MemFileParams globalParams = {false, -1, 0, nullptr};

MemFile::MemFile(const String& wrapper, const String& stream)
  : File(false, wrapper, stream) {
  globalParams.data = nullptr;
  globalParams.len = -1;
  globalParams.cursor = 0;
  globalParams.malloced = false;
  setIsLocal(true);
}

MemFile::MemFile(const char *data, int64_t len,
                 const String& wrapper, const String& stream)
  : File(false, wrapper, stream) {
  globalParams.data = nullptr;
  globalParams.len = len;
  globalParams.cursor = 0;
  globalParams.malloced = true;
  globalParams.data = (char*)malloc(len + 1);
  if (globalParams.data && len) {
    memcpy(globalParams.data, data, len);
  }
  globalParams.data[len] = '\0';
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
  assertx(globalParams.len == -1);
  const char* mode_str = mode.c_str();
  if (strchr(mode_str, '+') || strchr(mode_str, 'a') || strchr(mode_str, 'w')) {
    return false;
  }
  int len = INT_MIN;
  bool compressed = false;
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
      globalParams.data = data;
      globalParams.malloced = true;
      globalParams.len = len;
      return true;
    }
    setName(filename.toCppString());
    globalParams.data = data;
    globalParams.len = len;
    return true;
  }
  if (len != INT_MIN) {
    Logger::Error("Cannot open a PHP file or a directory as MemFile: %s",
                  filename.c_str());
  }
  return false;
}

bool MemFile::close() {
  return closeImpl();
}

bool MemFile::closeImpl() {
  *s_pcloseRet = 0;
  setIsClosed(true);
  if (globalParams.malloced && globalParams.data) {
    free(globalParams.data);
    globalParams.data = nullptr;
  }
  File::closeImpl();
  return true;
}

///////////////////////////////////////////////////////////////////////////////

int64_t MemFile::readImpl(char *buffer, int64_t length) {
  assertx(globalParams.len != -1);
  assertx(length > 0);
  assertx(globalParams.cursor >= 0);
  int64_t remaining = globalParams.len - globalParams.cursor;
  if (remaining < length) length = remaining;
  if (length > 0) {
    memcpy(buffer, (const void *)(globalParams.data + globalParams.cursor), length);
    globalParams.cursor += length;
    return length;
  }
  return 0;
}

int MemFile::getc() {
  assertx(globalParams.len != -1);
  return File::getc();
}

bool MemFile::seek(int64_t offset, int whence) {
  assertx(globalParams.len != -1);
  if (whence == SEEK_CUR) {
    if (offset >= 0 && offset < bufferedLen()) {
      setReadPosition(getReadPosition() + offset);
      setPosition(getPosition() + offset);
      return true;
    }
    offset += getPosition();
    whence = SEEK_SET;
  }

  setWritePosition(0);
  setReadPosition(0);
  if (whence == SEEK_SET) {
    if (offset < 0) return false;
    globalParams.cursor = offset;
  } else if (whence == SEEK_END) {
    if (globalParams.len + offset < 0) return false;
    globalParams.cursor = globalParams.len + offset;
  } else {
    return false;
  }
  setPosition(globalParams.cursor);
  return true;
}

int64_t MemFile::tell() {
  assertx(globalParams.len != -1);
  return getPosition();
}

bool MemFile::eof() {
  assertx(globalParams.len != -1);
  int64_t avail = bufferedLen();
  if (avail > 0) {
    return false;
  }
  return globalParams.cursor == globalParams.len;
}

bool MemFile::rewind() {
  assertx(globalParams.len != -1);
  globalParams.cursor = 0;
  setWritePosition(0);
  setReadPosition(0);
  setPosition(0);
  return true;
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
  ret.set(s_unread_bytes, globalParams.len - globalParams.cursor);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

void MemFile::unzip() {
  assertx(globalParams.len != -1);
  assertx(!globalParams.malloced);
  assertx(globalParams.cursor == 0);
  int len = globalParams.len;
  char *data = gzdecode(globalParams.data, len);
  if (data == nullptr) {
    raise_fatal_error((std::string("cannot unzip mem stream: ") +
                               getName()).c_str());
  }
  globalParams.data = data;
  globalParams.malloced = true;
  globalParams.len = len;
}

///////////////////////////////////////////////////////////////////////////////
}