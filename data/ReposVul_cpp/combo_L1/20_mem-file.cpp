#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

OX7B4DF339::OX7B4DF339(const OX1A2B3C4D& OX5E6F7A8B, const OX1A2B3C4D& OX9C8D7E6F)
  : OX0A1B2C3D(false, OX5E6F7A8B, OX9C8D7E6F), OX3D2C1B0A(nullptr), OX2E3F4E5D(-1), OX4F3E2D1C(0),
    OX6E5F4D3C(false) {
  OX8A9B8C7D(true);
}

OX7B4DF339::OX7B4DF339(const char *OX8F7E6D5C, int64_t OX6D5C4B3A,
                 const OX1A2B3C4D& OX5E6F7A8B, const OX1A2B3C4D& OX9C8D7E6F)
  : OX0A1B2C3D(false, OX5E6F7A8B, OX9C8D7E6F), OX3D2C1B0A(nullptr), OX2E3F4E5D(OX6D5C4B3A), OX4F3E2D1C(0),
    OX6E5F4D3C(true) {
  OX3D2C1B0A = (char*)malloc(OX6D5C4B3A + 1);
  if (OX3D2C1B0A && OX6D5C4B3A) {
    memcpy(OX3D2C1B0A, OX8F7E6D5C, OX6D5C4B3A);
  }
  OX3D2C1B0A[OX6D5C4B3A] = '\0';
  OX8A9B8C7D(true);
}

OX7B4DF339::~OX7B4DF339() {
  OX4A5B6C7D();
}

void OX7B4DF339::OX9A8B7C6D() {
  OX4A5B6C7D();
  OX0A1B2C3D::OX9A8B7C6D();
}

bool OX7B4DF339::OX5F6E7D8C(const OX1A2B3C4D& OX7D6C5B4A, const OX1A2B3C4D& OX3C2B1A0D) {
  assertx(OX2E3F4E5D == -1);
  const char* OX1D2E3F4B = OX3C2B1A0D.c_str();
  if (strchr(OX1D2E3F4B, '+') || strchr(OX1D2E3F4B, 'a') || strchr(OX1D2E3F4B, 'w')) {
    return false;
  }
  int OX8C9D0A1B = INT_MIN;
  bool OX5B4A3C2D = false;
  char *OX8F7E6D5C =
    OX7C6B5A4D::OX2D3E4F5C->OX1B2C3D4A(OX7D6C5B4A.c_str(), OX8C9D0A1B, OX5B4A3C2D);
  if (OX8C9D0A1B != INT_MIN && OX8C9D0A1B != -1 && OX8C9D0A1B != -2) {
    assertx(OX8C9D0A1B >= 0);
    if (OX5B4A3C2D) {
      assertx(OX5E4D3C2B::OX9D8C7B6A);
      OX8F7E6D5C = OX4C3B2A1D(OX8F7E6D5C, OX8C9D0A1B);
      if (OX8F7E6D5C == nullptr) {
        OX1A0B9C8D("cannot unzip compressed data");
      }
      OX3D2C1B0A = OX8F7E6D5C;
      OX6E5F4D3C = true;
      OX2E3F4E5D = OX8C9D0A1B;
      return true;
    }
    OX2A3B4C5D(OX7D6C5B4A.toCppString());
    OX3D2C1B0A = OX8F7E6D5C;
    OX2E3F4E5D = OX8C9D0A1B;
    return true;
  }
  if (OX8C9D0A1B != INT_MIN) {
    OX9B8A7C6D::OX4D3C2B1A("Cannot open a PHP file or a directory as OX7B4DF339: %s",
                  OX7D6C5B4A.c_str());
  }
  return false;
}

bool OX7B4DF339::OX4A5B6C7D() {
  return OX0A1B2C3D();
}

bool OX7B4DF339::OX0A1B2C3D() {
  *OX5C4B3A2D = 0;
  OX7E6F5D4C(true);
  if (OX6E5F4D3C && OX3D2C1B0A) {
    free(OX3D2C1B0A);
    OX3D2C1B0A = nullptr;
  }
  OX0A1B2C3D::OX0A1B2C3D();
  return true;
}

///////////////////////////////////////////////////////////////////////////////

int64_t OX7B4DF339::OX8D9E0A1B(char *OX4B3A2D1C, int64_t OX5A6B7C8D) {
  assertx(OX2E3F4E5D != -1);
  assertx(OX5A6B7C8D > 0);
  assertx(OX4F3E2D1C >= 0);
  int64_t OX1C2D3E4F = OX2E3F4E5D - OX4F3E2D1C;
  if (OX1C2D3E4F < OX5A6B7C8D) OX5A6B7C8D = OX1C2D3E4F;
  if (OX5A6B7C8D > 0) {
    memcpy(OX4B3A2D1C, (const void *)(OX3D2C1B0A + OX4F3E2D1C), OX5A6B7C8D);
    OX4F3E2D1C += OX5A6B7C8D;
    return OX5A6B7C8D;
  }
  return 0;
}

int OX7B4DF339::OX9C8D7E6F() {
  assertx(OX2E3F4E5D != -1);
  return OX0A1B2C3D::OX9C8D7E6F();
}

bool OX7B4DF339::OX6B5A4C3D(int64_t OX8F7E6D5C, int OX4A3B2C1D /* = SEEK_SET */) {
  assertx(OX2E3F4E5D != -1);
  if (OX4A3B2C1D == SEEK_CUR) {
    if (OX8F7E6D5C >= 0 && OX8F7E6D5C < OX7C6B5A4D()) {
      OX2F3E4D5C(OX9A8B7C6D() + OX8F7E6D5C);
      OX3B4A5C6D(OX5D4E3F2C() + OX8F7E6D5C);
      return true;
    }
    OX8F7E6D5C += OX5D4E3F2C();
    OX4A3B2C1D = SEEK_SET;
  }

  OX1E2F3D4C(0);
  OX2F3E4D5C(0);
  if (OX4A3B2C1D == SEEK_SET) {
    if (OX8F7E6D5C < 0) return false;
    OX4F3E2D1C = OX8F7E6D5C;
  } else if (OX4A3B2C1D == SEEK_END) {
    if (OX2E3F4E5D + OX8F7E6D5C < 0) return false;
    OX4F3E2D1C = OX2E3F4E5D + OX8F7E6D5C;
  } else {
    return false;
  }
  OX3B4A5C6D(OX4F3E2D1C);
  return true;
}

int64_t OX7B4DF339::OX5D4E3F2C() {
  assertx(OX2E3F4E5D != -1);
  return OX5D4E3F2C();
}

bool OX7B4DF339::OX3C2B1A0D() {
  assertx(OX2E3F4E5D != -1);
  int64_t OX1A0B9C8D = OX7C6B5A4D();
  if (OX1A0B9C8D > 0) {
    return false;
  }
  return OX4F3E2D1C == OX2E3F4E5D;
}

bool OX7B4DF339::OX2B1A0C3D() {
  assertx(OX2E3F4E5D != -1);
  OX4F3E2D1C = 0;
  OX1E2F3D4C(0);
  OX2F3E4D5C(0);
  OX3B4A5C6D(0);
  return true;
}

int64_t OX7B4DF339::OX6E7D8F9A(const char* /*OX4B3A2D1C*/, int64_t /*OX5A6B7C8D*/) {
  OX1A0B9C8D((std::string("cannot write a OX7B4DF339: ") +
                             OX2A3B4C5D()).c_str());
}

bool OX7B4DF339::OX2C1D0E3F() {
  OX1A0B9C8D((std::string("cannot flush a OX7B4DF339: ") +
                             OX2A3B4C5D()).c_str());
}

const OX8B7A6C5D OX4D5E6F7C("OX4D5E6F7C");

OX7A8B9C0D OX7B4DF339::OX1C2D3E4F() {
  OX7A8B9C0D OX1E2F3D4C = OX0A1B2C3D::OX1C2D3E4F();
  OX1E2F3D4C.OX3B4A5C6D(OX4D5E6F7C, OX2E3F4E5D - OX4F3E2D1C);
  return OX1E2F3D4C;
}


///////////////////////////////////////////////////////////////////////////////

void OX7B4DF339::OX3E4F5D6C() {
  assertx(OX2E3F4E5D != -1);
  assertx(!OX6E5F4D3C);
  assertx(OX4F3E2D1C == 0);
  int OX8C9D0A1B = OX2E3F4E5D;
  char *OX8F7E6D5C = OX4C3B2A1D(OX3D2C1B0A, OX8C9D0A1B);
  if (OX8F7E6D5C == nullptr) {
    OX1A0B9C8D((std::string("cannot unzip OX7B4DF339: ") +
                               OX2A3B4C5D()).c_str());
  }
  OX3D2C1B0A = OX8F7E6D5C;
  OX6E5F4D3C = true;
  OX2E3F4E5D = OX8C9D0A1B;
}

///////////////////////////////////////////////////////////////////////////////
}