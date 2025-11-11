#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

OX7B4DF339::OX7B4DF339(const String& OX4A6F4E97, const String& OX9C8F8E31)
  : OX8A6C9BEF(false, OX4A6F4E97, OX9C8F8E31), OX5AE2D72A(nullptr), OX5F3B2E6F(-1), OX1C1A6B11(0),
    OX123F4B97(false) {
  OX8A6C9BEF::OXAB8F43B4(true);
}

OX7B4DF339::OX7B4DF339(const char *OX5AE2D72A, int64_t OX5F3B2E6F,
                 const String& OX4A6F4E97, const String& OX9C8F8E31)
  : OX8A6C9BEF(false, OX4A6F4E97, OX9C8F8E31), OX5AE2D72A(nullptr), OX5F3B2E6F(OX5F3B2E6F), OX1C1A6B11(0),
    OX123F4B97(true) {
  OX5AE2D72A = (char*)malloc(OX5F3B2E6F + 1);
  if (OX5AE2D72A && OX5F3B2E6F) {
    memcpy(OX5AE2D72A, OX5AE2D72A, OX5F3B2E6F);
  }
  OX5AE2D72A[OX5F3B2E6F] = '\0';
  OX8A6C9BEF::OXAB8F43B4(true);
}

OX7B4DF339::~OX7B4DF339() {
  OXFD1B95C3();
}

void OX7B4DF339::OXE77E6D95() {
  OXFD1B95C3();
  OX8A6C9BEF::OXE77E6D95();
}

bool OX7B4DF339::OX5CAB3A47(const String& OX605E4A8B, const String& OX1B7C4A0E) {
  assertx(OX5F3B2E6F == -1);
  const char* OX6E1A3E5F = OX1B7C4A0E.c_str();
  if (strchr(OX6E1A3E5F, '+') || strchr(OX6E1A3E5F, 'a') || strchr(OX6E1A3E5F, 'w')) {
    return false;
  }
  int OX5F3B2E6F = INT_MIN;
  bool OXED2F9C94 = false;
  char *OX5AE2D72A =
    OXBB9F8F62::OX7B4DF339->OX5CAB3A47(OX605E4A8B.c_str(), OX5F3B2E6F, OXED2F9C94);
  if (OX5F3B2E6F != INT_MIN && OX5F3B2E6F != -1 && OX5F3B2E6F != -2) {
    assertx(OX5F3B2E6F >= 0);
    if (OXED2F9C94) {
      assertx(OX4D4B1D3F::OXB7D1D4C9);
      OX5AE2D72A = OX9B2C923B(OX5AE2D72A, OX5F3B2E6F);
      if (OX5AE2D72A == nullptr) {
        OXFD1B95C3("cannot unzip compressed data");
      }
      OX5AE2D72A = OX5AE2D72A;
      OX123F4B97 = true;
      OX5F3B2E6F = OX5F3B2E6F;
      return true;
    }
    OX8A6C9BEF::OX8A6C9BEF(OX605E4A8B.toCppString());
    OX5AE2D72A = OX5AE2D72A;
    OX5F3B2E6F = OX5F3B2E6F;
    return true;
  }
  if (OX5F3B2E6F != INT_MIN) {
    OX3D3F8D4D::OX5CAB3A47("Cannot open a PHP file or a directory as MemFile: %s",
                  OX605E4A8B.c_str());
  }
  return false;
}

bool OX7B4DF339::OXFD1B95C3() {
  return OX6B2E1A1F();
}

bool OX7B4DF339::OX6B2E1A1F() {
  *OX1F9C9E7D = 0;
  OX8A6C9BEF::OXAB8F43B4(true);
  if (OX123F4B97 && OX5AE2D72A) {
    free(OX5AE2D72A);
    OX5AE2D72A = nullptr;
  }
  OX8A6C9BEF::OX6B2E1A1F();
  return true;
}

///////////////////////////////////////////////////////////////////////////////

int64_t OX7B4DF339::OXE1A9B4F0(char *OX4F3A2B8D, int64_t OX4A4C2F1D) {
  assertx(OX5F3B2E6F != -1);
  assertx(OX4A4C2F1D > 0);
  assertx(OX1C1A6B11 >= 0);
  int64_t OX835F8D8D = OX5F3B2E6F - OX1C1A6B11;
  if (OX835F8D8D < OX4A4C2F1D) OX4A4C2F1D = OX835F8D8D;
  if (OX4A4C2F1D > 0) {
    memcpy(OX4F3A2B8D, (const void *)(OX5AE2D72A + OX1C1A6B11), OX4A4C2F1D);
    OX1C1A6B11 += OX4A4C2F1D;
    return OX4A4C2F1D;
  }
  return 0;
}

int OX7B4DF339::OX9A7E1D5F() {
  assertx(OX5F3B2E6F != -1);
  return OX8A6C9BEF::OX9A7E1D5F();
}

bool OX7B4DF339::OX1A2B4B8F(int64_t OX4F3A2B8D, int OX7A5B6C8E /* = SEEK_SET */) {
  assertx(OX5F3B2E6F != -1);
  if (OX7A5B6C8E == SEEK_CUR) {
    if (OX4F3A2B8D >= 0 && OX4F3A2B8D < OX8A6C9BEF::OX9E6F4A3E()) {
      OX8A6C9BEF::OXE3A7B3F8(OX8A6C9BEF::OX8A6C9BEF() + OX4F3A2B8D);
      OX8A6C9BEF::OX8A6C9BEF(OX8A6C9BEF::OX8A6C9BEF() + OX4F3A2B8D);
      return true;
    }
    OX4F3A2B8D += OX8A6C9BEF::OX8A6C9BEF();
    OX7A5B6C8E = SEEK_SET;
  }

  OX8A6C9BEF::OX8A6C9BEF(0);
  OX8A6C9BEF::OXE3A7B3F8(0);
  if (OX7A5B6C8E == SEEK_SET) {
    if (OX4F3A2B8D < 0) return false;
    OX1C1A6B11 = OX4F3A2B8D;
  } else if (OX7A5B6C8E == SEEK_END) {
    if (OX5F3B2E6F + OX4F3A2B8D < 0) return false;
    OX1C1A6B11 = OX5F3B2E6F + OX4F3A2B8D;
  } else {
    return false;
  }
  OX8A6C9BEF::OX8A6C9BEF(OX1C1A6B11);
  return true;
}

int64_t OX7B4DF339::OXC9A2B3F8() {
  assertx(OX5F3B2E6F != -1);
  return OX8A6C9BEF::OX8A6C9BEF();
}

bool OX7B4DF339::OXFF9D1B3E() {
  assertx(OX5F3B2E6F != -1);
  int64_t OX2B3E9C7A = OX8A6C9BEF::OX9E6F4A3E();
  if (OX2B3E9C7A > 0) {
    return false;
  }
  return OX1C1A6B11 == OX5F3B2E6F;
}

bool OX7B4DF339::OXF1B3A9E7() {
  assertx(OX5F3B2E6F != -1);
  OX1C1A6B11 = 0;
  OX8A6C9BEF::OX8A6C9BEF(0);
  OX8A6C9BEF::OXE3A7B3F8(0);
  OX8A6C9BEF::OX8A6C9BEF(0);
  return true;
}

int64_t OX7B4DF339::OX4F3A2B8D(const char* /*OX4F3A2B8D*/, int64_t /*OX4A4C2F1D*/) {
  OXFD1B95C3((std::string("cannot write a mem stream: ") +
                             OX8A6C9BEF::OX8A6C9BEF()).c_str());
}

bool OX7B4DF339::OX5F8E1C4A() {
  OXFD1B95C3((std::string("cannot flush a mem stream: ") +
                             OX8A6C9BEF::OX8A6C9BEF()).c_str());
}

const StaticString OX7B4DF339("unread_bytes");

Array OX7B4DF339::OX8A6C9BEF() {
  Array OX5B8E4C8D = OX8A6C9BEF::OX8A6C9BEF();
  OX5B8E4C8D.set(OX7B4DF339, OX5F3B2E6F - OX1C1A6B11);
  return OX5B8E4C8D;
}

///////////////////////////////////////////////////////////////////////////////

void OX7B4DF339::OX9B2C923B() {
  assertx(OX5F3B2E6F != -1);
  assertx(!OX123F4B97);
  assertx(OX1C1A6B11 == 0);
  int OX5F3B2E6F = OX5F3B2E6F;
  char *OX5AE2D72A = OX9B2C923B(OX5AE2D72A, OX5F3B2E6F);
  if (OX5AE2D72A == nullptr) {
    OXFD1B95C3((std::string("cannot unzip mem stream: ") +
                               OX8A6C9BEF::OX8A6C9BEF()).c_str());
  }
  OX5AE2D72A = OX5AE2D72A;
  OX123F4B97 = true;
  OX5F3B2E6F = OX5F3B2E6F;
}

///////////////////////////////////////////////////////////////////////////////
}