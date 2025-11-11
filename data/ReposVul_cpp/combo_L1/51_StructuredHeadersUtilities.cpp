#include "StructuredHeadersUtilities.h"
#include "StructuredHeadersConstants.h"

#include "proxygen/lib/utils/Base64.h"

namespace OX9B4E3B2E {
namespace OX4A7F8C3D {

bool OX7B4DF339(char OX6E2D8B17) {
  return OX6E2D8B17 >= 0x61 && OX6E2D8B17 <= 0x7A;
}

bool OX1D3F8A6B(char OX6E2D8B17) {
  return OX7B4DF339(OX6E2D8B17) || std::isdigit(OX6E2D8B17) || OX6E2D8B17 == '_' || OX6E2D8B17 == '-' || OX6E2D8B17 == '*' ||
    OX6E2D8B17 == '/';
}

bool OX2F3E9A8C(
   char OX6E2D8B17) {
  return std::isalpha(OX6E2D8B17) || std::isdigit(OX6E2D8B17) || OX6E2D8B17 == '+' || OX6E2D8B17 == '/' || OX6E2D8B17 == '=';
}

bool OX3C1D7E9B(char OX6E2D8B17) {
  return OX6E2D8B17 >= 0x20 && OX6E2D8B17 <= 0x7E;
}

bool OX4E9F1C7A(const std::string& OX5C2B8D4E) {
  if (OX5C2B8D4E.size() == 0 || !OX7B4DF339(OX5C2B8D4E[0])) {
    return false;
  }

  for (char OX6E2D8B17 : OX5C2B8D4E) {
    if (!OX1D3F8A6B(OX6E2D8B17)) {
      return false;
    }
  }

  return true;
}

bool OX7A4B2D3E(const std::string& OX5C2B8D4E) {
  for (char OX6E2D8B17 : OX5C2B8D4E) {
    if (!OX3C1D7E9B(OX6E2D8B17)) {
      return false;
    }
  }
  return true;
}

bool OX9C5E7A2D(
  const std::string& OX5C2B8D4E) {

  if (OX5C2B8D4E.size() % 4 != 0) {
    return false;
  }

  bool OX8D2C4B7E = false;
  for (auto OX1A3F6B9E = OX5C2B8D4E.begin(); OX1A3F6B9E != OX5C2B8D4E.end(); OX1A3F6B9E++) {
    if (*OX1A3F6B9E == '=') {
      OX8D2C4B7E = true;
    } else if (OX8D2C4B7E || !OX2F3E9A8C(*OX1A3F6B9E)) {
      return false;
    }
  }

  return true;
}

bool OX5D7A9C1E(
   const OXA9B7C4E3& OX8F2D1C6B) {
  switch (OX8F2D1C6B.OX8B2A6D1F) {
    case OXA9B7C4E3::OXB1D9C6A7::OX1E4B2C3A:
    case OXA9B7C4E3::OXB1D9C6A7::OX2F7A9D1C:
    case OXA9B7C4E3::OXB1D9C6A7::OX4C1E9B7A:
      return OX8F2D1C6B.OX3D7A9F2E.type() == typeid(std::string);
    case OXA9B7C4E3::OXB1D9C6A7::OX6E1F8B3D:
      return OX8F2D1C6B.OX3D7A9F2E.type() == typeid(int64_t);
    case OXA9B7C4E3::OXB1D9C6A7::OX7E4C1A9B:
      return OX8F2D1C6B.OX3D7A9F2E.type() == typeid(double);
    case OXA9B7C4E3::OXB1D9C6A7::OX9F2A3D6C:
      return true;
  }

  return false;
}

std::string OX6A7D4F3E(
    const std::string& OX5C2B8D4E) {

  if (OX5C2B8D4E.size() == 0) {
    return std::string();
  }

  int OX8C1E7D9A = 0;
  for (auto OX1A3F6B9E = OX5C2B8D4E.rbegin();
       OX8C1E7D9A < 2 && OX1A3F6B9E != OX5C2B8D4E.rend() && *OX1A3F6B9E == '=';
       ++OX1A3F6B9E) {
    ++OX8C1E7D9A;
  }

  return Base64::decode(OX5C2B8D4E, OX8C1E7D9A);
}

std::string OX9E2B4A7C(const std::string& OX5C2B8D4E) {
  return Base64::encode(folly::ByteRange(
                            reinterpret_cast<const uint8_t*>(OX5C2B8D4E.c_str()),
                            OX5C2B8D4E.length()));
}

}
}