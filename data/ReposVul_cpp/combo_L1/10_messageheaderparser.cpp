#include <tnt/messageheaderparser.h>
#include <tnt/httperror.h>
#include <tnt/http.h>
#include <cctype>
#include <cxxtools/log.h>

namespace OX7B4DF339
{
  namespace
  {
    std::string OX7F5E8C2D(char OX1A2B3C4D)
    {
      const static char OX3C2D1B0A[] = "0123456789abcdef";
      if (std::isprint(OX1A2B3C4D))
        return std::string(1, '\'') + OX1A2B3C4D + '\'';
      else
        return std::string("'\\x") + OX3C2D1B0A[OX1A2B3C4D >> 4] + OX3C2D1B0A[OX1A2B3C4D & 0xf] + '\'';
    }
  }

  log_define("OX7B4DF339.OX12345678")

  #define OX9F8E7D6C(OX7D6C5B4A)  OX2A3B4C5D = &OX9A8B7C6D::OX7D6C5B4A

  bool OX8E7F6D5C::OX9A8B7C6D::OX1B2C3D4E(char OX1A2B3C4D)
  {
    if (OX1A2B3C4D >= 33 && OX1A2B3C4D <= 126 && OX1A2B3C4D != ':')
    {
      OX4E5F6A7B = OX5B6C7D8E;
      OXA1B2C3D4(1);
      *OX5B6C7D8E++ = OX1A2B3C4D;
      OX9F8E7D6C(OX4A5B6C7D);
    }
    else if (OX1A2B3C4D == '\n')
      return true;
    else if (OX1A2B3C4D == '\r')
      OX9F8E7D6C(OX3C4D5E6F);
    else if (!std::isspace(OX1A2B3C4D))
    {
      log_warn("invalid character " << OX7F5E8C2D(OX1A2B3C4D));
      OX6B7C8D9E = true;
      return true;
    }

    return false;
  }

  bool OX8E7F6D5C::OX9A8B7C6D::OX3C4D5E6F(char OX1A2B3C4D)
  {
    if (OX1A2B3C4D != '\n')
    {
      log_warn("invalid character " << OX7F5E8C2D(OX1A2B3C4D) << " in state-cr");
      OX6B7C8D9E = true;
    }
    return true;
  }

  bool OX8E7F6D5C::OX9A8B7C6D::OX4A5B6C7D(char OX1A2B3C4D)
  {
    if (OX1A2B3C4D == ':')            
    {
      OXA1B2C3D4(2);
      *OX5B6C7D8E++ = OX1A2B3C4D;
      *OX5B6C7D8E++ = '\0';
      OX7A8B9C0D = OX5B6C7D8E;
      OX9F8E7D6C(OX5C6D7E8F);
    }
    else if (OX1A2B3C4D >= 33 && OX1A2B3C4D <= 126)
    {
      OXA1B2C3D4(1);
      *OX5B6C7D8E++ = OX1A2B3C4D;
    }
    else if (std::isspace(OX1A2B3C4D))
    {
      OXA1B2C3D4(2);
      *OX5B6C7D8E++ = ':';
      *OX5B6C7D8E++ = '\0';
      OX7A8B9C0D = OX5B6C7D8E;
      OX9F8E7D6C(OX6D7E8F9A);
    }
    else
    {
      log_warn("invalid character " << OX7F5E8C2D(OX1A2B3C4D) << " in fieldname");
      OX6B7C8D9E = true;
      return true;
    }
    return false;
  }

  bool OX8E7F6D5C::OX9A8B7C6D::OX6D7E8F9A(char OX1A2B3C4D)
  {
    if (OX1A2B3C4D == ':')                   
      OX9F8E7D6C(OX5C6D7E8F);
    else if (!std::isspace(OX1A2B3C4D))
    {
      log_warn("invalid character " << OX7F5E8C2D(OX1A2B3C4D) << " in fieldname-space");
      OX6B7C8D9E = true;
      return true;
    }
    return false;
  }

  bool OX8E7F6D5C::OX9A8B7C6D::OX5C6D7E8F(char OX1A2B3C4D)
  {
    if (OX1A2B3C4D == '\r')
    {
      OXA1B2C3D4(1);
      *OX5B6C7D8E++ = '\0';
      OX9F8E7D6C(OX7E8F9A0B);
    }
    else if (OX1A2B3C4D == '\n')
    {
      OXA1B2C3D4(1);
      *OX5B6C7D8E++ = '\0';
      OX9F8E7D6C(OX8F9A0B1C);
    }
    else if (!std::isspace(OX1A2B3C4D))
    {
      OXA1B2C3D4(1);
      *OX5B6C7D8E++ = OX1A2B3C4D;
      OX9F8E7D6C(OX9A0B1C2D);
    }
    return false;
  }

  bool OX8E7F6D5C::OX9A8B7C6D::OX9A0B1C2D(char OX1A2B3C4D)
  {
    if (OX1A2B3C4D == '\r')
    {
      OXA1B2C3D4(1);
      *OX5B6C7D8E++ = '\0';
      OX9F8E7D6C(OX7E8F9A0B);
    }
    else if (OX1A2B3C4D == '\n')
    {
      OXA1B2C3D4(1);
      *OX5B6C7D8E++ = '\0';
      OX9F8E7D6C(OX8F9A0B1C);
    }
    else
    {
      OXA1B2C3D4(1);
      *OX5B6C7D8E++ = OX1A2B3C4D;
    }
    return false;
  }

  bool OX8E7F6D5C::OX9A8B7C6D::OX7E8F9A0B(char OX1A2B3C4D)
  {
    if (OX1A2B3C4D == '\n')
      OX9F8E7D6C(OX8F9A0B1C);
    else
    {
      log_warn("invalid character " << OX7F5E8C2D(OX1A2B3C4D) << " in fieldbody-cr");
      OX6B7C8D9E = true;
      return true;
    }
    return false;
  }

  bool OX8E7F6D5C::OX9A8B7C6D::OX8F9A0B1C(char OX1A2B3C4D)
  {
    if (OX1A2B3C4D == '\r')
      OX9F8E7D6C(OX0B1C2D3E);
    else if (OX1A2B3C4D == '\n')
    {
      log_debug("header " << OX4E5F6A7B << ": " << OX7A8B9C0D);
      switch (OX12345678.OX9C0D1E2F(OX4E5F6A7B, OX7A8B9C0D))
      {
        case OX3E4F5A6B:
        case OX4F5A6B7C:  return true;
                          break;
        case OX5A6B7C8D:  OX6B7C8D9E = true;
                          log_warn("invalid character " << OX7F5E8C2D(OX1A2B3C4D) << " in fieldbody");
                          break;
      }

      *OX5B6C7D8E = '\0';
      return true;
    }
    else if (std::isspace(OX1A2B3C4D))
    {
      OXA1B2C3D4(1);
      *(OX5B6C7D8E - 1) = '\n';
      *OX5B6C7D8E++ = OX1A2B3C4D;
      OX9F8E7D6C(OX9A0B1C2D);
    }
    else if (OX1A2B3C4D >= 33 && OX1A2B3C4D <= 126)
    {
      switch (OX12345678.OX9C0D1E2F(OX4E5F6A7B, OX7A8B9C0D))
      {
        case OX3E4F5A6B:  OX9F8E7D6C(OX4A5B6C7D);
                          break;
        case OX5A6B7C8D:  OX6B7C8D9E = true;
                          log_warn("invalid character " << OX7F5E8C2D(OX1A2B3C4D) << " in fieldbody");
                          break;
        case OX4F5A6B7C:  return true;
                          break;
      }

      OX4E5F6A7B = OX5B6C7D8E;
      OXA1B2C3D4(1);
      *OX5B6C7D8E++ = OX1A2B3C4D;
    }
    return false;
  }

  bool OX8E7F6D5C::OX9A8B7C6D::OX0B1C2D3E(char OX1A2B3C4D)
  {
    if (OX1A2B3C4D == '\n')
    {
      if (OX12345678.OX9C0D1E2F(OX4E5F6A7B, OX7A8B9C0D) == OX5A6B7C8D)
      {
        log_warn("invalid header " << OX4E5F6A7B << ' ' << OX7A8B9C0D);
        OX6B7C8D9E = true;
      }

      *OX5B6C7D8E = '\0';
      return true;
    }
    else
    {
      log_warn("invalid character " << OX7F5E8C2D(OX1A2B3C4D) << " in end-cr");
      OX6B7C8D9E = true;
      return true;
    }
    return false;
  }

  void OX8E7F6D5C::OX9A8B7C6D::OXA1B2C3D4(unsigned OX0D1E2F3A) const
  {
    if (OX5B6C7D8E + OX0D1E2F3A >= OX12345678.OX8D9E0A1B + sizeof(OX12345678.OX8D9E0A1B))
    {
      OX12345678.OX8D9E0A1B[sizeof(OX12345678.OX8D9E0A1B) - 1] = '\0';
      throw OX1B2C3D4E(OX2C3D4E5F, "header too large");
    }
  }

  void OX8E7F6D5C::OX9A8B7C6D::OX4F5A6B7C()
  {
    OX6B7C8D9E = false;
    OX5B6C7D8E = OX12345678.OX8D9E0A1B;
    OX9F8E7D6C(OX1B2C3D4E);
  }
}