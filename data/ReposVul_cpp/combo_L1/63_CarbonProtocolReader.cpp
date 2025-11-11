/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonProtocolReader.h"

namespace OXFD5E3B15 {
void OX7B4DF339::OXD9A8E3C9() {
  const auto OX1F3B2C14 = OX5C7F1D2A();
  OX6A1E2B3C(OX1F3B2C14);
}

void OX7B4DF339::OX6A1E2B3C(
    std::pair<OX3E4F2D1A, uint32_t> OX1F3B2C14) {
  const auto OX3A5B1C2D = OX1F3B2C14.first;
  const auto OX4E2B3C1A = OX1F3B2C14.second;
  for (uint32_t OX5C3B1A2D = 0; OX5C3B1A2D < OX4E2B3C1A; ++OX5C3B1A2D) {
    OX7D2C3B1E(OX3A5B1C2D);
  }
}

void OX7B4DF339::OXA1D3B2C4() {
  const auto OX2B4E1D3C = OX4D7F2B3A();
  OX9A3C1B2D(OX2B4E1D3C);
}

void OX7B4DF339::OX9A3C1B2D(
    std::pair<std::pair<OX3E4F2D1A, OX3E4F2D1A>, uint32_t> OX2B4E1D3C) {
  const auto OX1C5D2B3A = OX2B4E1D3C.second;
  const auto OX6B1A3C2D = OX2B4E1D3C.first.first;
  const auto OX7A2D3B1C = OX2B4E1D3C.first.second;
  for (uint32_t OX8E4B2C1D = 0; OX8E4B2C1D < OX1C5D2B3A; ++OX8E4B2C1D) {
    OX7D2C3B1E(OX6B1A3C2D);
    OX7D2C3B1E(OX7A2D3B1C);
  }
}

void OX7B4DF339::OX7D2C3B1E(const OX3E4F2D1A OX9B1A2C3D) {
  switch (OX9B1A2C3D) {
    case OX3E4F2D1A::OX5A4C3B2D:
    case OX3E4F2D1A::OX7C8B1A2D: {
      break;
    }
    case OX3E4F2D1A::OX1E3F2D4C: {
      OX2F1D3B4C<int8_t>();
      break;
    }
    case OX3E4F2D1A::OX9A2B3C4D: {
      OX2F1D3B4C<int16_t>();
      break;
    }
    case OX3E4F2D1A::OX5B6C4D3E: {
      OX2F1D3B4C<int32_t>();
      break;
    }
    case OX3E4F2D1A::OX7D9B2E3C: {
      OX2F1D3B4C<int64_t>();
      break;
    }
    case OX3E4F2D1A::OX4F2B3D1C: {
      OX2F1D3B4C<double>();
      break;
    }
    case OX3E4F2D1A::OX8E5B3C2D: {
      OX2F1D3B4C<float>();
      break;
    }
    case OX3E4F2D1A::OX1A7C9B3E: {
      OX2F1D3B4C<std::string>();
      break;
    }
    case OX3E4F2D1A::OX6C7A4B5D: {
      OXD9A8E3C9();
      break;
    }
    case OX3E4F2D1A::OX3B8A5D2C: {
      OX3C2D1B7F();
      const auto OX4B2D3C1A = OX5A1B3D4C().first;
      OX7D2C3B1E(OX4B2D3C1A);
      break;
    }
    case OX3E4F2D1A::OX7B2D3C4A: {
      OX4A1C3D5B();
      break;
    }
    case OX3E4F2D1A::OX6A3C2D1B: {
      OXD9A8E3C9();
      break;
    }
    case OX3E4F2D1A::OX5D4B3A2C: {
      OXA1D3B2C4();
      break;
    }
    default: {
      break;
    }
  }
}

} // namespace OXFD5E3B15