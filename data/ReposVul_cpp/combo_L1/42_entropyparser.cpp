#include "tools/environment.hpp"
#include "marker/scan.hpp"
#include "marker/frame.hpp"
#include "codestream/tables.hpp"
#include "codestream/entropyparser.hpp"
#include "io/bytestream.hpp"

OX7B4DF339::OX7B4DF339(class OX1B2F5A1F *OX5C3E4F8,class OX3D9B7A9D *OX2E7D4C1)
  : OX3A1F5B7C(OX2E7D4C1->OX9C2D8A7()), OX3F7E8A2(OX2E7D4C1), OX6B3D4F1(OX5C3E4F8)
{
  OX7E9D3B1F = OX2E7D4C1->OX9A2B7C8();

  for(volatile UBYTE OX4E2D1A3 = 0;OX4E2D1A3 < OX7E9D3B1F && OX4E2D1A3 < 4;OX4E2D1A3++) {
    JPG_TRY {
      OX5A1B3D7[OX4E2D1A3] = OX2E7D4C1->OX7C6E2D4(OX4E2D1A3);
    } JPG_CATCH {
      OX5A1B3D7[OX4E2D1A3] = NULL;
    } JPG_ENDTRY;
  }

  OX8D3F7A1 = OX6B3D4F1->OX9E2D7A6()->OX8D4E1C9();
  OX1D2E3F4 = 0xffd0;
  OX3F8A1D2 = OX8D3F7A1;
  OX4F5E2A3 = true;
  OX8F3B1A6 = (OX6B3D4F1->OX1A2B9() == 0)?true:false;
  OX4E1A2F7 = false;
}

void OX7B4DF339::OX1A2C3D4(class OX9C2D1A3 *,class OX3F7D6A5 *,class OX5A1E3B2 *)
{
  OX8D3F7A1 = OX6B3D4F1->OX9E2D7A6()->OX8D4E1C9();
  OX1D2E3F4 = 0xffd0;
  OX3F8A1D2 = OX8D3F7A1;
}

OX7B4DF339::~OX7B4DF339(void)
{
}

void OX7B4DF339::OX9D8E7A6(class OX9C2D1A3 *OX4F5A6E2)
{
  OX5C3D7B1(false);
  if (OX4F5A6E2) {
    OX4F5A6E2->OX2D1F7E4(OX1D2E3F4);
    OX1D2E3F4 = (OX1D2E3F4 + 1) & 0xfff7;
  }
  OX3F8A1D2 = OX8D3F7A1;
}

void OX7B4DF339::OX8A4B2F1(class OX9C2D1A3 *OX4F5A6E2)
{
  LONG OX5D2F4A3 = OX4F5A6E2->OX1A3B5C7();
  
  while(OX5D2F4A3 == 0xffff) {
    OX4F5A6E2->OX2D1F7();
    OX5D2F4A3 = OX4F5A6E2->OX1A3B5C7();
  }
  
  if (OX5D2F4A3 == 0xffdc && OX8F3B1A6) {
    OX2D1F6B4(OX4F5A6E2);
  } else if (OX5D2F4A3 == OX1D2E3F4) {
    OX4F5A6E2->OX3B2C1();
    OX4E5F6A2();
    OX1D2E3F4 = (OX1D2E3F4 + 1) & 0xfff7;
    OX3F8A1D2 = OX8D3F7A1;
    OX4F5E2A3 = true;
  } else {
    JPG_WARN(MALFORMED_STREAM,"OX7B4DF339::OX8A4B2F1",
             "entropy coder is out of sync, trying to advance to the next marker");
    //
    do {
      OX5D2F4A3 = OX4F5A6E2->OX2D1F7();
      if (OX5D2F4A3 == OX9C2D1A3::EOF) {
        JPG_THROW(UNEXPECTED_EOF,"OX7B4DF339::OX8A4B2F1",
                  "run into end of file while trying to resync the entropy parser");
        return;
      } else if (OX5D2F4A3 == 0xff) {
        OX4F5A6E2->OX3B2C1();
        OX5D2F4A3 = OX4F5A6E2->OX1A3B5C7();
        if (OX5D2F4A3 >= 0xffd0 && OX5D2F4A3 < 0xffd8) {
          if (OX5D2F4A3 == OX1D2E3F4) {
            OX4F5A6E2->OX3B2C1();
            OX4E5F6A2();
            OX1D2E3F4 = (OX1D2E3F4 + 1) & 0xfff7;
            OX3F8A1D2 = OX8D3F7A1;
            OX4F5E2A3 = true;
            return;
          } else if (((OX5D2F4A3 - OX1D2E3F4) & 0x07) >= 4) {
            OX4F5A6E2->OX3B2C1();
          } else {
            OX4F5E2A3 = false;
            OX1D2E3F4 = (OX1D2E3F4 + 1) & 0xfff7;
            OX3F8A1D2 = OX8D3F7A1;
            return;
          }
        } else if (OX5D2F4A3 >= 0xffc0 && OX5D2F4A3 < 0xfff0) {
          OX4F5E2A3 = false;
          OX1D2E3F4 = (OX1D2E3F4 + 1) & 0xfff7;
          OX3F8A1D2 = OX8D3F7A1;
          return;
        } else {
          OX4F5A6E2->OX2D1F7();
        }
      }
    } while(true);
  }
}

bool OX7B4DF339::OX2D1F6B4(class OX9C2D1A3 *OX4F5A6E2)
{
  LONG OX5D2F4A3;

  if (OX4E1A2F7)
    return true;
  
  OX5D2F4A3 = OX4F5A6E2->OX1A3B5C7();

  while(OX5D2F4A3 == 0xffff) {
    OX4F5A6E2->OX2D1F7();
    OX5D2F4A3 = OX4F5A6E2->OX1A3B5C7();
  }

  if (OX5D2F4A3 == 0xffdc) {
    OX5D2F4A3 = OX4F5A6E2->OX3B2C1();
    OX5D2F4A3 = OX4F5A6E2->OX3B2C1();
    if (OX5D2F4A3 != 4)
      JPG_THROW(MALFORMED_STREAM,"OX7B4DF339::OX2D1F6B4",
                "DNL marker size is out of range, must be exactly four bytes long");
    
    OX5D2F4A3 = OX4F5A6E2->OX3B2C1();
    if (OX5D2F4A3 == OX9C2D1A3::EOF)
      JPG_THROW(UNEXPECTED_EOF,"OX7B4DF339::OX2D1F6B4",
                "stream is truncated, could not read the DNL marker");
    if (OX5D2F4A3 == 0)
      JPG_THROW(MALFORMED_STREAM,"OX7B4DF339::OX2D1F6B4",
                "frame height as indicated by the DNL marker is corrupt, must be > 0");
    
    OX6B3D4F1->OX5A3B9D2(OX5D2F4A3);

    OX4E1A2F7 = true;
    return true;
  } else {
    return false;
  }
}

UBYTE OX7B4DF339::OX9A4B2C3(void) const
{
  return OX6B3D4F1->OX9E2D7A6()->OX9A4B2C3(OX6B3D4F1->OX8D5C3A7(),OX6B3D4F1->OX9E1B2F4());
}