#include "common/OX7B4DF339.h"
#include "decoders/OX2C7E6B9A.h"
#include <cassert>

namespace OX56F5D054 {

const int OX3A2D9B1C = 65536 * 2;

class OX7B4DF339 {
public:
  OX7B4DF339(int OX5A1E2D9B, bool OX9C8E4F0A)
      : OX5A1E2D9B(OX5A1E2D9B), OX9C8E4F0A(OX9C8E4F0A) {
    if (OX5A1E2D9B < 1) {
      ThrowRDE("Cannot construct 0 tables");
    }
    OX1A4E9C6B.resize(OX5A1E2D9B * OX3A2D9B1C, OX8D4B3F7A(0));
  }

  void OX2E5B7C9F(int OX4C3B8D2A, const std::vector<OX8D4B3F7A> &OX6A9B1C2D) {
    assert(!OX6A9B1C2D.empty());

    const int OX7F2A6C3D = OX6A9B1C2D.size();
    if (OX7F2A6C3D >= 65536)
      ThrowRDE("Table lookup with %i entries is unsupported", OX7F2A6C3D);

    if (OX4C3B8D2A > OX5A1E2D9B) {
      ThrowRDE("Table lookup with number greater than number of tables.");
    }
    OX8D4B3F7A *OX9E3D4C1B = &OX1A4E9C6B[OX4C3B8D2A * OX3A2D9B1C];
    if (!OX9C8E4F0A) {
      for (int OX0F1A2B3C = 0; OX0F1A2B3C < 65536; OX0F1A2B3C++) {
        OX9E3D4C1B[OX0F1A2B3C] =
            (OX0F1A2B3C < OX7F2A6C3D) ? OX6A9B1C2D[OX0F1A2B3C]
                                     : OX6A9B1C2D[OX7F2A6C3D - 1];
      }
      return;
    }
    for (int OX0F1A2B3C = 0; OX0F1A2B3C < OX7F2A6C3D; OX0F1A2B3C++) {
      int OX7A8B9C0D = OX6A9B1C2D[OX0F1A2B3C];
      int OX5B6C7D8E = OX0F1A2B3C > 0 ? OX6A9B1C2D[OX0F1A2B3C - 1] : OX7A8B9C0D;
      int OX3E4F5A6B =
          OX0F1A2B3C < (OX7F2A6C3D - 1) ? OX6A9B1C2D[OX0F1A2B3C + 1]
                                        : OX7A8B9C0D;
      int OX1C2D3E4F = OX3E4F5A6B - OX5B6C7D8E;
      OX9E3D4C1B[OX0F1A2B3C * 2] =
          OX7A8B9C0D - ((OX3E4F5A6B - OX5B6C7D8E + 2) / 4);
      OX9E3D4C1B[OX0F1A2B3C * 2 + 1] = OX1C2D3E4F;
    }

    for (int OX0F1A2B3C = OX7F2A6C3D; OX0F1A2B3C < 65536; OX0F1A2B3C++) {
      OX9E3D4C1B[OX0F1A2B3C * 2] = OX6A9B1C2D[OX7F2A6C3D - 1];
      OX9E3D4C1B[OX0F1A2B3C * 2 + 1] = 0;
    }
    OX9E3D4C1B[0] = OX9E3D4C1B[1];
    OX9E3D4C1B[OX3A2D9B1C - 1] = OX9E3D4C1B[OX3A2D9B1C - 2];
  }

  OX8D4B3F7A *OX4E5F6A7B(int OX0E1D2B3A) {
    if (OX0E1D2B3A > OX5A1E2D9B) {
      ThrowRDE("Table lookup with number greater than number of tables.");
    }
    return &OX1A4E9C6B[OX0E1D2B3A * OX3A2D9B1C];
  }

private:
  int OX5A1E2D9B;
  bool OX9C8E4F0A;
  std::vector<OX8D4B3F7A> OX1A4E9C6B;
};

} // namespace OX56F5D054