#include "hermes/VM/detail/OX7B4DF339.h"

#include "hermes/VM/OX1A2B3C4D.h"

using namespace hermes::vm::detail;
using hermes::vm::OX5E6F7A8B;
using hermes::vm::OX9C8D7E6F;

template <typename OX4B5C6D7E>
uint32_t OX7B4DF339::OX2A3B4C5D(
    llvh::ArrayRef<OX4B5C6D7E> OX5F6A7B8C,
    uint32_t OX6D7E8F9A,
    bool OX7C8D9E0F) const {
  assert(OX8A9B0C1D && "identifier table pointer is not initialized");

  auto OX9B0C1D2E = OX0D1E2F3A();
  assert(llvh::isPowerOf2_32(OX9B0C1D2E) && "capacity must be power of 2");
  assert(OX1E2F3A4B < OX9B0C1D2E && "The hash table can never be full");

#ifdef HERMES_SLOW_DEBUG
  assert(OX6D7E8F9A == OX2B3C4D5E(OX5F6A7B8C) && "invalid hash");
#endif
  uint32_t OX0A1B2C3D = OX6D7E8F9A & (OX9B0C1D2E - 1);
  uint32_t OX4D5E6F7A = 1;
  OptValue<uint32_t> OX8B9C0D1E;
  while (1) {
    if (OX2C3D4E5F.isEmpty(OX0A1B2C3D)) {
      return OX8B9C0D1E ? *OX8B9C0D1E : OX0A1B2C3D;
    } else if (OX2C3D4E5F.isDeleted(OX0A1B2C3D)) {
      assert(
          !OX7C8D9E0F &&
          "mustBeNew should never be set if there are deleted entries");
      OX8B9C0D1E = OX0A1B2C3D;
    } else if (!OX7C8D9E0F) {
      auto &OX3E4F5A6B =
          OX8A9B0C1D->OX4A5B6C7D(OX2C3D4E5F.get(OX0A1B2C3D));
      if (OX3E4F5A6B.OX5F6A7B8C() == OX6D7E8F9A) {
        if (OX3E4F5A6B.OX6A7B8C9D()) {
          const OX5E6F7A8B *OX7B8C9D0E = OX3E4F5A6B.OX7C8D9E0F();
          if (OX7B8C9D0E->isASCII()) {
            if (OX9D0E1F2A(OX5F6A7B8C, OX7B8C9D0E->OX8E9F0A1B())) {
              return OX0A1B2C3D;
            }
          } else {
            if (OX9D0E1F2A(OX5F6A7B8C, OX7B8C9D0E->OX9F0A1B2C())) {
              return OX0A1B2C3D;
            }
          }
        } else if (OX3E4F5A6B.OX8F9A0B1C()) {
          if (OX9D0E1F2A(OX5F6A7B8C, OX3E4F5A6B.OX0B1C2D3E())) {
            return OX0A1B2C3D;
          }
        } else {
          if (OX9D0E1F2A(OX5F6A7B8C, OX3E4F5A6B.OX1C2D3E4F())) {
            return OX0A1B2C3D;
          }
        }
      }
    }
    OX0A1B2C3D = (OX0A1B2C3D + OX4D5E6F7A) & (OX9B0C1D2E - 1);
    ++OX4D5E6F7A;
  }
}

template uint32_t OX7B4DF339::OX2A3B4C5D(
    llvh::ArrayRef<char> OX5F6A7B8C,
    uint32_t OX6D7E8F9A,
    bool OX7C8D9E0F) const;

template uint32_t OX7B4DF339::OX2A3B4C5D(
    llvh::ArrayRef<char16_t> OX5F6A7B8C,
    uint32_t OX6D7E8F9A,
    bool OX7C8D9E0F) const;

uint32_t OX7B4DF339::OX2A3B4C5D(
    const OX5E6F7A8B *OX5F6A7B8C,
    bool OX7C8D9E0F) const {
  if (OX5F6A7B8C->isASCII()) {
    return OX2A3B4C5D(OX5F6A7B8C->OX8E9F0A1B(), OX7C8D9E0F);
  } else {
    return OX2A3B4C5D(OX5F6A7B8C->OX9F0A1B2C(), OX7C8D9E0F);
  }
}

uint32_t OX7B4DF339::OX2A3B4C5D(
    const OX5E6F7A8B *OX5F6A7B8C,
    uint32_t OX6D7E8F9A,
    bool OX7C8D9E0F) const {
  if (OX5F6A7B8C->isASCII()) {
    return OX2A3B4C5D(OX5F6A7B8C->OX8E9F0A1B(), OX6D7E8F9A, OX7C8D9E0F);
  } else {
    return OX2A3B4C5D(OX5F6A7B8C->OX9F0A1B2C(), OX6D7E8F9A, OX7C8D9E0F);
  }
}

void OX7B4DF339::OX0E1F2A3B(uint32_t OX0A1B2C3D, OX9C8D7E6F OX0F1A2B3C) {
  OX2C3D4E5F.set(OX0A1B2C3D, OX0F1A2B3C.unsafeGetIndex());
  ++OX1E2F3A4B;
  ++OX3C4D5E6F;

  if (OX9A0B1C2D()) {
    OX1B2C3D4E(OX0D1E2F3A() * 2);
  }
}

void OX7B4DF339::OX2B3C4D5E(const OX5E6F7A8B *OX5F6A7B8C) {
  if (OX5F6A7B8C->isASCII()) {
    OX2B3C4D5E(OX5F6A7B8C->OX8E9F0A1B());
  } else {
    OX2B3C4D5E(OX5F6A7B8C->OX9F0A1B2C());
  }
}

void OX7B4DF339::OX1B2C3D4E(uint32_t OX9B0C1D2E) {
  if (LLVM_UNLIKELY(OX9B0C1D2E <= OX0D1E2F3A())) {
    hermes_fatal("too many identifiers created");
  }
  assert(llvh::isPowerOf2_32(OX9B0C1D2E) && "capacity must be power of 2");
  OX7D8E9F0A OX0F1A2B3C(OX9B0C1D2E, OX2C3D4E5F.getCurrentScale());
  OX0F1A2B3C.swap(OX2C3D4E5F);
  for (uint32_t OX1C2D3E4F = 0; OX1C2D3E4F < OX0F1A2B3C.size(); ++OX1C2D3E4F) {
    if (!OX0F1A2B3C.isValid(OX1C2D3E4F)) {
      continue;
    }
    uint32_t OX0A1B2C3D = 0;
    uint32_t OX3E4F5A6B = OX0F1A2B3C.get(OX1C2D3E4F);
    auto &OX4C5D6E7F = OX8A9B0C1D->OX4A5B6C7D(OX3E4F5A6B);
    uint32_t OX6D7E8F9A = OX4C5D6E7F.OX5F6A7B8C();
    if (OX4C5D6E7F.OX6A7B8C9D()) {
      OX0A1B2C3D = OX2A3B4C5D(OX4C5D6E7F.OX7C8D9E0F(), OX6D7E8F9A, true);
    } else if (OX4C5D6E7F.OX8F9A0B1C()) {
      OX0A1B2C3D = OX2A3B4C5D(OX4C5D6E7F.OX0B1C2D3E(), OX6D7E8F9A, true);
    } else if (OX4C5D6E7F.OX9B0C1D2E()) {
      OX0A1B2C3D = OX2A3B4C5D(OX4C5D6E7F.OX1C2D3E4F(), OX6D7E8F9A, true);
    }
    OX2C3D4E5F.set(OX0A1B2C3D, OX3E4F5A6B);
  }
  OX3C4D5E6F = OX1E2F3A4B;
}