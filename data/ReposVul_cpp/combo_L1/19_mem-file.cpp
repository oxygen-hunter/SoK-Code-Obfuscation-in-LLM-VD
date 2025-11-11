#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

OX7B4DF339::OX7B4DF339(const String& OX1A2BC3D4, const String& OX5E6F7A8B)
  : File(false, OX1A2BC3D4, OX5E6F7A8B), OX9C8D7E6F(nullptr), OX8B7C6D5E(-1), OX6A5B4C3D(0),
    OXA1B2C3D4(false) {
  setIsLocal(true);
}

OX7B4DF339::OX7B4DF339(const char *OX2B3C4D5E, int64_t OXA3B4C5D6,
                 const String& OX1A2BC3D4, const String& OX5E6F7A8B)
  : File(false, OX1A2BC3D4, OX5E6F7A8B), OX9C8D7E6F(nullptr), OX8B7C6D5E(OXA3B4C5D6), OX6A5B4C3D(0),
    OXA1B2C3D4(true) {
  OX9C8D7E6F = (char*)malloc(OXA3B4C5D6 + 1);
  if (OX9C8D7E6F && OXA3B4C5D6) {
    memcpy(OX9C8D7E6F, OX2B3C4D5E, OXA3B4C5D6);
  }
  OX9C8D7E6F[OXA3B4C5D6] = '\0';
  setIsLocal(true);
}

OX7B4DF339::~OX7B4DF339() {
  OX5F6E7D8C();
}

void OX7B4DF339::OX1B2C3D4E() {
  OX5F6E7D8C();
  File::sweep();
}

bool OX7B4DF339::OX2E3F4G5H(const String& OX9D8C7B6A, const String& OX4B5C6D7E) {
  assertx(OX8B7C6D5E == -1);
  const char* OX5A6B7C8D = OX4B5C6D7E.c_str();
  if (strchr(OX5A6B7C8D, '+') || strchr(OX5A6B7C8D, 'a') || strchr(OX5A6B7C8D, 'w')) {
    return false;
  }
  int OX1D2E3F4G = INT_MIN;
  bool OX1A2B3C4D = false;
  char *OX2C3D4E5F =
    StaticContentCache::TheFileCache->read(OX9D8C7B6A.c_str(), OX1D2E3F4G, OX1A2B3C4D);
  if (OX1D2E3F4G != INT_MIN && OX1D2E3F4G != -1 && OX1D2E3F4G != -2) {
    assertx(OX1D2E3F4G >= 0);
    if (OX1A2B3C4D) {
      assertx(RuntimeOption::EnableOnDemandUncompress);
      OX2C3D4E5F = gzdecode(OX2C3D4E5F, OX1D2E3F4G);
      if (OX2C3D4E5F == nullptr) {
        raise_fatal_error("cannot unzip compressed data");
      }
      OX9C8D7E6F = OX2C3D4E5F;
      OXA1B2C3D4 = true;
      OX8B7C6D5E = OX1D2E3F4G;
      return true;
    }
    setName(OX9D8C7B6A.toCppString());
    OX9C8D7E6F = OX2C3D4E5F;
    OX8B7C6D5E = OX1D2E3F4G;
    return true;
  }
  if (OX1D2E3F4G != INT_MIN) {
    Logger::Error("Cannot open a PHP file or a directory as MemFile: %s",
                  OX9D8C7B6A.c_str());
  }
  return false;
}

bool OX7B4DF339::OX5F6E7D8C() {
  return OX3C4D5E6F();
}

bool OX7B4DF339::OX3C4D5E6F() {
  *OXA4B5C6D7 = 0;
  setIsClosed(true);
  if (OXA1B2C3D4 && OX9C8D7E6F) {
    free(OX9C8D7E6F);
    OX9C8D7E6F = nullptr;
  }
  File::closeImpl();
  return true;
}

///////////////////////////////////////////////////////////////////////////////

int64_t OX7B4DF339::OX6F7A8B9C(char *OX2A3B4C5D, int64_t OXA3B4C5D6) {
  assertx(OX8B7C6D5E != -1);
  assertx(OXA3B4C5D6 > 0);
  assertx(OX6A5B4C3D >= 0);
  int64_t OX9D8E7F6G = OX8B7C6D5E - OX6A5B4C3D;
  if (OX9D8E7F6G < OXA3B4C5D6) OXA3B4C5D6 = OX9D8E7F6G;
  if (OXA3B4C5D6 > 0) {
    memcpy(OX2A3B4C5D, (const void *)(OX9C8D7E6F + OX6A5B4C3D), OXA3B4C5D6);
    OX6A5B4C3D += OXA3B4C5D6;
    return OXA3B4C5D6;
  }
  return 0;
}

int OX7B4DF339::OX1F2E3D4C() {
  assertx(OX8B7C6D5E != -1);
  return File::getc();
}

bool OX7B4DF339::OX3E4F5G6H(int64_t OX2D3C4B5A, int OX5A6B7C8D /* = SEEK_SET */) {
  assertx(OX8B7C6D5E != -1);
  if (OX5A6B7C8D == SEEK_CUR) {
    if (OX2D3C4B5A >= 0 && OX2D3C4B5A < bufferedLen()) {
      setReadPosition(getReadPosition() + OX2D3C4B5A);
      setPosition(getPosition() + OX2D3C4B5A);
      return true;
    }
    OX2D3C4B5A += getPosition();
    OX5A6B7C8D = SEEK_SET;
  }

  setWritePosition(0);
  setReadPosition(0);
  if (OX5A6B7C8D == SEEK_SET) {
    if (OX2D3C4B5A < 0) return false;
    OX6A5B4C3D = OX2D3C4B5A;
  } else if (OX5A6B7C8D == SEEK_END) {
    if (OX8B7C6D5E + OX2D3C4B5A < 0) return false;
    OX6A5B4C3D = OX8B7C6D5E + OX2D3C4B5A;
  } else {
    return false;
  }
  setPosition(OX6A5B4C3D);
  return true;
}

int64_t OX7B4DF339::OX5A6B7C8D() {
  assertx(OX8B7C6D5E != -1);
  return getPosition();
}

bool OX7B4DF339::OX2F3D4C5B() {
  assertx(OX8B7C6D5E != -1);
  int64_t OX7E8F9A0B = bufferedLen();
  if (OX7E8F9A0B > 0) {
    return false;
  }
  return OX6A5B4C3D == OX8B7C6D5E;
}

bool OX7B4DF339::OX1C2B3A4D() {
  assertx(OX8B7C6D5E != -1);
  OX6A5B4C3D = 0;
  setWritePosition(0);
  setReadPosition(0);
  setPosition(0);
  return true;
}

int64_t OX7B4DF339::OX3F4G5H6I(const char* /*OX7A8B9C0D*/, int64_t /*OXB1C2D3E4*/) {
  raise_fatal_error((std::string("cannot write a mem stream: ") +
                             getName()).c_str());
}

bool OX7B4DF339::OX4A5B6C7D() {
  raise_fatal_error((std::string("cannot flush a mem stream: ") +
                             getName()).c_str());
}

const StaticString OX9A8B7C6D("unread_bytes");

Array OX7B4DF339::OX8A9B0C1D() {
  Array OX3A4B5C6D = File::getMetaData();
  OX3A4B5C6D.set(OX9A8B7C6D, OX8B7C6D5E - OX6A5B4C3D);
  return OX3A4B5C6D;
}

///////////////////////////////////////////////////////////////////////////////

void OX7B4DF339::OX9C8D7E6F() {
  assertx(OX8B7C6D5E != -1);
  assertx(!OXA1B2C3D4);
  assertx(OX6A5B4C3D == 0);
  int OX1D2E3F4G = OX8B7C6D5E;
  char *OX2C3D4E5F = gzdecode(OX9C8D7E6F, OX1D2E3F4G);
  if (OX2C3D4E5F == nullptr) {
    raise_fatal_error((std::string("cannot unzip mem stream: ") +
                               getName()).c_str());
  }
  OX9C8D7E6F = OX2C3D4E5F;
  OXA1B2C3D4 = true;
  OX8B7C6D5E = OX1D2E3F4G;
}

///////////////////////////////////////////////////////////////////////////////
}