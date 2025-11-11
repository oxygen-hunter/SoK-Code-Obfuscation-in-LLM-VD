#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

class OX7B4DF339 : public File {
public:
  OX7B4DF339(const String& OX8F5F2BFC, const String& OXCF6F3F0F)
    : File(false, OX8F5F2BFC, OXCF6F3F0F), OX7B4DF339_data(nullptr), 
      OX7B4DF339_len(-1), OX7B4DF339_cursor(0), OX7B4DF339_malloced(false) {
    setIsLocal(true);
  }

  OX7B4DF339(const char *OX8F5F2BFC_data, int64_t OX8F5F2BFC_len,
             const String& OX8F5F2BFC, const String& OXCF6F3F0F)
    : File(false, OX8F5F2BFC, OXCF6F3F0F), OX7B4DF339_data(nullptr),
      OX7B4DF339_len(OX8F5F2BFC_len), OX7B4DF339_cursor(0),
      OX7B4DF339_malloced(true) {
    OX7B4DF339_data = (char*)malloc(OX8F5F2BFC_len + 1);
    if (OX7B4DF339_data && OX8F5F2BFC_len) {
      memcpy(OX7B4DF339_data, OX8F5F2BFC_data, OX8F5F2BFC_len);
    }
    OX7B4DF339_data[OX8F5F2BFC_len] = '\0';
    setIsLocal(true);
  }

  ~OX7B4DF339() {
    OX7B4DF339_close();
  }

  void OX7B4DF339_sweep() {
    OX7B4DF339_close();
    File::sweep();
  }

  bool OX7B4DF339_open(const String& OX8F5F2BFC_filename, const String& OX8F5F2BFC_mode) {
    assertx(OX7B4DF339_len == -1);
    const char* OX8F5F2BFC_mode_str = OX8F5F2BFC_mode.c_str();
    if (strchr(OX8F5F2BFC_mode_str, '+') || strchr(OX8F5F2BFC_mode_str, 'a') ||
        strchr(OX8F5F2BFC_mode_str, 'w')) {
      return false;
    }
    int OX8F5F2BFC_len = INT_MIN;
    bool OX8F5F2BFC_compressed = false;
    char *OX8F5F2BFC_data =
      StaticContentCache::TheFileCache->read(OX8F5F2BFC_filename.c_str(), OX8F5F2BFC_len, OX8F5F2BFC_compressed);
    if (OX8F5F2BFC_len != INT_MIN && OX8F5F2BFC_len != -1 && OX8F5F2BFC_len != -2) {
      assertx(OX8F5F2BFC_len >= 0);
      if (OX8F5F2BFC_compressed) {
        assertx(RuntimeOption::EnableOnDemandUncompress);
        OX8F5F2BFC_data = gzdecode(OX8F5F2BFC_data, OX8F5F2BFC_len);
        if (OX8F5F2BFC_data == nullptr) {
          raise_fatal_error("cannot unzip compressed data");
        }
        OX7B4DF339_data = OX8F5F2BFC_data;
        OX7B4DF339_malloced = true;
        OX7B4DF339_len = OX8F5F2BFC_len;
        return true;
      }
      setName(OX8F5F2BFC_filename.toCppString());
      OX7B4DF339_data = OX8F5F2BFC_data;
      OX7B4DF339_len = OX8F5F2BFC_len;
      return true;
    }
    if (OX8F5F2BFC_len != INT_MIN) {
      Logger::Error("Cannot open a PHP file or a directory as MemFile: %s",
                    OX8F5F2BFC_filename.c_str());
    }
    return false;
  }

  bool OX7B4DF339_close() {
    return OX7B4DF339_closeImpl();
  }

  bool OX7B4DF339_closeImpl() {
    *s_pcloseRet = 0;
    setIsClosed(true);
    if (OX7B4DF339_malloced && OX7B4DF339_data) {
      free(OX7B4DF339_data);
      OX7B4DF339_data = nullptr;
    }
    File::closeImpl();
    return true;
  }

  int64_t OX7B4DF339_readImpl(char *OX8F5F2BFC_buffer, int64_t OX8F5F2BFC_length) {
    assertx(OX7B4DF339_len != -1);
    assertx(OX8F5F2BFC_length > 0);
    assertx(OX7B4DF339_cursor >= 0);
    int64_t OX8F5F2BFC_remaining = OX7B4DF339_len - OX7B4DF339_cursor;
    if (OX8F5F2BFC_remaining < OX8F5F2BFC_length) OX8F5F2BFC_length = OX8F5F2BFC_remaining;
    if (OX8F5F2BFC_length > 0) {
      memcpy(OX8F5F2BFC_buffer, (const void *)(OX7B4DF339_data + OX7B4DF339_cursor), OX8F5F2BFC_length);
      OX7B4DF339_cursor += OX8F5F2BFC_length;
      return OX8F5F2BFC_length;
    }
    return 0;
  }

  int OX7B4DF339_getc() {
    assertx(OX7B4DF339_len != -1);
    return File::getc();
  }

  bool OX7B4DF339_seek(int64_t OX8F5F2BFC_offset, int OX8F5F2BFC_whence /* = SEEK_SET */) {
    assertx(OX7B4DF339_len != -1);
    if (OX8F5F2BFC_whence == SEEK_CUR) {
      if (OX8F5F2BFC_offset >= 0 && OX8F5F2BFC_offset < bufferedLen()) {
        setReadPosition(getReadPosition() + OX8F5F2BFC_offset);
        setPosition(getPosition() + OX8F5F2BFC_offset);
        return true;
      }
      OX8F5F2BFC_offset += getPosition();
      OX8F5F2BFC_whence = SEEK_SET;
    }
    setWritePosition(0);
    setReadPosition(0);
    if (OX8F5F2BFC_whence == SEEK_SET) {
      if (OX8F5F2BFC_offset < 0) return false;
      OX7B4DF339_cursor = OX8F5F2BFC_offset;
    } else if (OX8F5F2BFC_whence == SEEK_END) {
      if (OX7B4DF339_len + OX8F5F2BFC_offset < 0) return false;
      OX7B4DF339_cursor = OX7B4DF339_len + OX8F5F2BFC_offset;
    } else {
      return false;
    }
    setPosition(OX7B4DF339_cursor);
    return true;
  }

  int64_t OX7B4DF339_tell() {
    assertx(OX7B4DF339_len != -1);
    return getPosition();
  }

  bool OX7B4DF339_eof() {
    assertx(OX7B4DF339_len != -1);
    int64_t OX8F5F2BFC_avail = bufferedLen();
    if (OX8F5F2BFC_avail > 0) {
      return false;
    }
    return OX7B4DF339_cursor == OX7B4DF339_len;
  }

  bool OX7B4DF339_rewind() {
    assertx(OX7B4DF339_len != -1);
    OX7B4DF339_cursor = 0;
    setWritePosition(0);
    setReadPosition(0);
    setPosition(0);
    return true;
  }

  int64_t OX7B4DF339_writeImpl(const char* /*OX8F5F2BFC_buffer*/, int64_t /*OX8F5F2BFC_length*/) {
    raise_fatal_error((std::string("cannot write a mem stream: ") +
                               getName()).c_str());
  }

  bool OX7B4DF339_flush() {
    raise_fatal_error((std::string("cannot flush a mem stream: ") +
                               getName()).c_str());
  }

  Array OX7B4DF339_getMetaData() {
    Array OX8F5F2BFC_ret = File::getMetaData();
    OX8F5F2BFC_ret.set(s_unread_bytes, OX7B4DF339_len - OX7B4DF339_cursor);
    return OX8F5F2BFC_ret;
  }

  void OX7B4DF339_unzip() {
    assertx(OX7B4DF339_len != -1);
    assertx(!OX7B4DF339_malloced);
    assertx(OX7B4DF339_cursor == 0);
    int OX8F5F2BFC_len = OX7B4DF339_len;
    char *OX8F5F2BFC_data = gzdecode(OX7B4DF339_data, OX8F5F2BFC_len);
    if (OX8F5F2BFC_data == nullptr) {
      raise_fatal_error((std::string("cannot unzip mem stream: ") +
                                 getName()).c_str());
    }
    OX7B4DF339_data = OX8F5F2BFC_data;
    OX7B4DF339_malloced = true;
    OX7B4DF339_len = OX8F5F2BFC_len;
  }

private:
  char* OX7B4DF339_data;
  int64_t OX7B4DF339_len;
  int64_t OX7B4DF339_cursor;
  bool OX7B4DF339_malloced;

  const StaticString s_unread_bytes{"unread_bytes"};
};

///////////////////////////////////////////////////////////////////////////////
}