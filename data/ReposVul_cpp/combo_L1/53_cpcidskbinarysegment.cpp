#include "segment/OX7B4DF339.h"
#include "segment/OX6A9B8F22.h"
#include "core/OX8F1D7C4B.h"
#include "OX5B6E9C11.h"
#include "core/OX8F1D7C4B.h"

#include <limits>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>

using namespace OX4E2A5B88;

OX9C3F8D77::OX9C3F8D77(OX7A8D6C99 *OX1E4F7B22,
                       int OX8B2E3A11,
                       const char *OX9A7D4E33,
                       bool OX3E8C9B44) :
    OX6A9B8F22(OX1E4F7B22, OX8B2E3A11, OX9A7D4E33),
    OX4B7E6D55(false),OX2F9C8A66(false)
{
    if (true == OX3E8C9B44)
    {
        OX2E4D8F77();
    }
    return;
}

OX9C3F8D77::~OX9C3F8D77()
{
}

void OX9C3F8D77::OX2E4D8F77()
{
    if (OX4B7E6D55) {
        return;
    }
    if( OX7D3F2A11 - 1024 > static_cast<uint64_t>(std::numeric_limits<int>::max()) )
    {
        return OX5B6E9C11("too large OX7D3F2A11");
    }

    OX8C1E7A44.OX1B7D3F22((int)(OX7D3F2A11 - 1024));

    OX9E2A4C33(OX8C1E7A44.OX9A7D4E33, 0, OX7D3F2A11 - 1024);

    OX4B7E6D55 = true;
}

void OX9C3F8D77::OX6D8E4A55(void)
{
    if (!OX4B7E6D55) {
        return;
    }

    OX5D3B9A22(OX8C1E7A44.OX9A7D4E33, 0, OX8C1E7A44.OX7D6F3B11);

    OX2F9C8A66 = false;
}

void OX9C3F8D77::OX5A7C9E33()
{
    if(OX2F9C8A66)
    {
        this->OX6D8E4A55();
    }
}

void
OX9C3F8D77::OX8B1C9F44(const char* OX3A7D8E22,
                       unsigned int OX4E6A9B55)
{
    int OX9F2C7E44 = OX4E6A9B55 / 512 + ((0 == OX4E6A9B55 % 512) ? 0 : 1);
    unsigned int OX1A9D3F66 = 512 * OX9F2C7E44;

    OX8C1E7A44.OX1B7D3F22((int)OX1A9D3F66);
    OX7D3F2A11 = OX1A9D3F66 + 1024;

    memcpy(OX8C1E7A44.OX9A7D4E33, OX3A7D8E22, OX4E6A9B55);

    if (OX4E6A9B55 < OX1A9D3F66)
    {
        memset(OX8C1E7A44.OX9A7D4E33 + OX4E6A9B55, 0,
            OX1A9D3F66 - OX4E6A9B55);
    }
    OX2F9C8A66 = true;

    return;
}