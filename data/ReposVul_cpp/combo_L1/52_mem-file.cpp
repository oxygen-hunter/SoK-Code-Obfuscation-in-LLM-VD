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
  OX7B4DF339(const String& OX2BCF50E1, const String& OX4F2B6F06);
  OX7B4DF339(const char* OX7F2E3A39, int64_t OX2B1F7E2F, const String& OX2BCF50E1, const String& OX4F2B6F06);
  ~OX7B4DF339();

  void OX4A464B0D();
  bool OX626E8C1E(const String& OX7C4B6B0B, const String& OX2F9C3B65);
  bool OX5F8A9BDF();
  bool OX59B1E6B8();

  int64_t OX5C5E6B1C(char* OX2A9B7D5F, int64_t OX1A5E3F6C);
  int OX2E1A3B4D();
  bool OX4E5F9B0D(int64_t OX3C4B6F0A, int OX4F7E2E1B = SEEK_SET);
  int64_t OX1A3F4B5D();
  bool OX5D1F6E4A();
  bool OX4E6B2A1F();
  int64_t OX7C4B2E0A(const char* /*buffer*/, int64_t /*length*/);
  bool OX2D1F4B5C();
  Array OX3F6B2E1A();

  void OX2B4F6E1C();

private:
  char* OX7F2E3A39;
  int64_t OX2B1F7E2F;
  int64_t OX3C1F6E2B;
  bool OX2B7E6F3A;
};

///////////////////////////////////////////////////////////////////////////////

OX7B4DF339::OX7B4DF339(const String& OX2BCF50E1, const String& OX4F2B6F06)
  : File(false, OX2BCF50E1, OX4F2B6F06), OX7F2E3A39(nullptr), OX2B1F7E2F(-1), OX3C1F6E2B(0),
    OX2B7E6F3A(false) {
  setIsLocal(true);
}

OX7B4DF339::OX7B4DF339(const char* OX7F2E3A39, int64_t OX2B1F7E2F, const String& OX2BCF50E1, const String& OX4F2B6F06)
  : File(false, OX2BCF50E1, OX4F2B6F06), OX7F2E3A39(nullptr), OX2B1F7E2F(OX2B1F7E2F), OX3C1F6E2B(0),
    OX2B7E6F3A(true) {
  OX7F2E3A39 = (char*)malloc(OX2B1F7E2F + 1);
  if (OX7F2E3A39 && OX2B1F7E2F) {
    memcpy(OX7F2E3A39, OX7F2E3A39, OX2B1F7E2F);
  }
  OX7F2E3A39[OX2B1F7E2F] = '\0';
  setIsLocal(true);
}

OX7B4DF339::~OX7B4DF339() {
  OX5F8A9BDF();
}

void OX7B4DF339::OX4A464B0D() {
  OX5F8A9BDF();
  File::sweep();
}

bool OX7B4DF339::OX626E8C1E(const String& OX7C4B6B0B, const String& OX2F9C3B65) {
  assertx(OX2B1F7E2F == -1);
  const char* OX7E2B6A39 = OX2F9C3B65.c_str();
  if (strchr(OX7E2B6A39, '+') || strchr(OX7E2B6A39, 'a') || strchr(OX7E2B6A39, 'w')) {
    return false;
  }
  int OX5D1F6A3B = INT_MIN;
  bool OX3F4B6A2E = false;
  char* OX7F2E3A39 =
    StaticContentCache::TheFileCache->read(OX7C4B6B0B.c_str(), OX5D1F6A3B, OX3F4B6A2E);
  if (OX5D1F6A3B != INT_MIN && OX5D1F6A3B != -1 && OX5D1F6A3B != -2) {
    assertx(OX5D1F6A3B >= 0);
    if (OX3F4B6A2E) {
      assertx(RuntimeOption::EnableOnDemandUncompress);
      OX7F2E3A39 = gzdecode(OX7F2E3A39, OX5D1F6A3B);
      if (OX7F2E3A39 == nullptr) {
        raise_fatal_error("cannot unzip compressed data");
      }
      OX7F2E3A39 = OX7F2E3A39;
      OX2B7E6F3A = true;
      OX2B1F7E2F = OX5D1F6A3B;
      return true;
    }
    setName(OX7C4B6B0B.toCppString());
    OX7F2E3A39 = OX7F2E3A39;
    OX2B1F7E2F = OX5D1F6A3B;
    return true;
  }
  if (OX5D1F6A3B != INT_MIN) {
    Logger::Error("Cannot open a PHP file or a directory as MemFile: %s",
                  OX7C4B6B0B.c_str());
  }
  return false;
}

bool OX7B4DF339::OX5F8A9BDF() {
  return OX59B1E6B8();
}

bool OX7B4DF339::OX59B1E6B8() {
  *s_pcloseRet = 0;
  setIsClosed(true);
  if (OX2B7E6F3A && OX7F2E3A39) {
    free(OX7F2E3A39);
    OX7F2E3A39 = nullptr;
  }
  File::closeImpl();
  return true;
}

///////////////////////////////////////////////////////////////////////////////

int64_t OX7B4DF339::OX5C5E6B1C(char* OX2A9B7D5F, int64_t OX1A5E3F6C) {
  assertx(OX2B1F7E2F != -1);
  assertx(OX1A5E3F6C > 0);
  assertx(OX3C1F6E2B >= 0);
  int64_t OX4F6E3B2D = OX2B1F7E2F - OX3C1F6E2B;
  if (OX4F6E3B2D < OX1A5E3F6C) OX1A5E3F6C = OX4F6E3B2D;
  if (OX1A5E3F6C > 0) {
    memcpy(OX2A9B7D5F, (const void*)(OX7F2E3A39 + OX3C1F6E2B), OX1A5E3F6C);
    OX3C1F6E2B += OX1A5E3F6C;
    return OX1A5E3F6C;
  }
  return 0;
}

int OX7B4DF339::OX2E1A3B4D() {
  assertx(OX2B1F7E2F != -1);
  return File::getc();
}

bool OX7B4DF339::OX4E5F9B0D(int64_t OX3C4B6F0A, int OX4F7E2E1B /* = SEEK_SET */) {
  assertx(OX2B1F7E2F != -1);
  if (OX4F7E2E1B == SEEK_CUR) {
    if (OX3C4B6F0A >= 0 && OX3C4B6F0A < bufferedLen()) {
      setReadPosition(getReadPosition() + OX3C4B6F0A);
      setPosition(getPosition() + OX3C4B6F0A);
      return true;
    }
    OX3C4B6F0A += getPosition();
    OX4F7E2E1B = SEEK_SET;
  }
  setWritePosition(0);
  setReadPosition(0);
  if (OX4F7E2E1B == SEEK_SET) {
    if (OX3C4B6F0A < 0) return false;
    OX3C1F6E2B = OX3C4B6F0A;
  } else if (OX4F7E2E1B == SEEK_END) {
    if (OX2B1F7E2F + OX3C4B6F0A < 0) return false;
    OX3C1F6E2B = OX2B1F7E2F + OX3C4B6F0A;
  } else {
    return false;
  }
  setPosition(OX3C1F6E2B);
  return true;
}

int64_t OX7B4DF339::OX1A3F4B5D() {
  assertx(OX2B1F7E2F != -1);
  return getPosition();
}

bool OX7B4DF339::OX5D1F6E4A() {
  assertx(OX2B1F7E2F != -1);
  int64_t OX4F3E6B2A = bufferedLen();
  if (OX4F3E6B2A > 0) {
    return false;
  }
  return OX3C1F6E2B == OX2B1F7E2F;
}

bool OX7B4DF339::OX4E6B2A1F() {
  assertx(OX2B1F7E2F != -1);
  OX3C1F6E2B = 0;
  setWritePosition(0);
  setReadPosition(0);
  setPosition(0);
  return true;
}

int64_t OX7B4DF339::OX7C4B2E0A(const char* /*buffer*/, int64_t /*length*/) {
  raise_fatal_error((std::string("cannot write a mem stream: ") +
                             getName()).c_str());
}

bool OX7B4DF339::OX2D1F4B5C() {
  raise_fatal_error((std::string("cannot flush a mem stream: ") +
                             getName()).c_str());
}

const StaticString OX4A7F3B6D("unread_bytes");

Array OX7B4DF339::OX3F6B2E1A() {
  Array OX2C3E5A1D = File::getMetaData();
  OX2C3E5A1D.set(OX4A7F3B6D, OX2B1F7E2F - OX3C1F6E2B);
  return OX2C3E5A1D;
}

///////////////////////////////////////////////////////////////////////////////

void OX7B4DF339::OX2B4F6E1C() {
  assertx(OX2B1F7E2F != -1);
  assertx(!OX2B7E6F3A);
  assertx(OX3C1F6E2B == 0);
  int OX4F5E2A1C = OX2B1F7E2F;
  char* OX7F2E3A39 = gzdecode(OX7F2E3A39, OX4F5E2A1C);
  if (OX7F2E3A39 == nullptr) {
    raise_fatal_error((std::string("cannot unzip mem stream: ") +
                               getName()).c_str());
  }
  OX7F2E3A39 = OX7F2E3A39;
  OX2B7E6F3A = true;
  OX2B1F7E2F = OX4F5E2A1C;
}

///////////////////////////////////////////////////////////////////////////////
}