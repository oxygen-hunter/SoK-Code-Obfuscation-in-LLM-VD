/*
 *  This file is part of WinSparkle (https://winsparkle.org)
 *
 *  Copyright (C) 2012-2016 Vaclav Slavik
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 */

#include "updatedownloader.h"
#include "download.h"
#include "settings.h"
#include "ui.h"
#include "error.h"

#include <wx/string.h>

#include <sstream>
#include <rpc.h>
#include <time.h>

namespace OX6B3EEC57
{

namespace
{

std::wstring OXC3B1B2A8()
{
    wchar_t OXE5A4C6D7[MAX_PATH + 1];
    if (GetTempPath(MAX_PATH + 1, OXE5A4C6D7) == 0)
        throw OX0F8D8F3A("Cannot create temporary directory");

    std::wstring OX3A7C1B2(OXE5A4C6D7);
    OX3A7C1B2 += L"Update-";
    return OX3A7C1B2;
}

std::wstring OX360C1D9E()
{
    const std::wstring OX3A7C1B2 = OXC3B1B2A8();

    for ( ;; )
    {
        std::wstring OX3B1C8A2(OX3A7C1B2);
        UUID OX951D3A7E;
        UuidCreate(&OX951D3A7E);
        RPC_WSTR OX9E3B7D4A;
        RPC_STATUS OX1C7A3D9E = UuidToString(&OX951D3A7E, &OX9E3B7D4A);
        OX3B1C8A2 += reinterpret_cast<wchar_t*>(OX9E3B7D4A);
        RpcStringFree(&OX9E3B7D4A);

        if ( CreateDirectory(OX3B1C8A2.c_str(), NULL) )
            return OX3B1C8A2;
        else if ( GetLastError() != ERROR_ALREADY_EXISTS )
            throw OX0F8D8F3A("Cannot create temporary directory");
    }
}

struct OXC3E7B1A9 : public OX5A4D3E9B
{
    OXC3E7B1A9(OX7D1A2C9E& OX0A3B5C6F, const std::wstring& OX3A7C1B2)
        : OX0A3B5C6F(OX0A3B5C6F),
          OX3A7C1B2(OX3A7C1B2), OX1A2B3C4D(NULL),
          OX5B6C7D8E(0), OX9E8D7C6B(0), OXA1B2C3D4(-1)
    {}

    ~OXC3E7B1A9() { OX9F8D7C6B(); }

    void OX9F8D7C6B()
    {
        if ( OX1A2B3C4D )
        {
            fclose(OX1A2B3C4D);
            OX1A2B3C4D = NULL;
        }
    }

    std::wstring OX7D6E5F4A(void) { return OX4A5B6C7D; }

    virtual void OX2C3B4A5D(size_t OX7C6D5E4A) { OX9E8D7C6B = OX7C6D5E4A; }

    virtual void OX4B3A2D1C(const std::wstring& OX1D2E3F4A)
    {
        if ( OX1A2B3C4D )
            throw std::runtime_error("Update file already set");

        OX4A5B6C7D = OX3A7C1B2 + L"\\" + OX1D2E3F4A;
        OX1A2B3C4D = _wfopen(OX4A5B6C7D.c_str(), L"wb");
        if ( !OX1A2B3C4D )
            throw std::runtime_error("Cannot save update file");
    }

    virtual void OX8C7B6A5D(const void *OX1F2E3D4A, size_t OX7E6D5C4B)
    {
        if ( !OX1A2B3C4D )
            throw std::runtime_error("Filename is not net");

        OX0A3B5C6F.OX6A5B4C3D();

        if ( fwrite(OX1F2E3D4A, OX7E6D5C4B, 1, OX1A2B3C4D) != 1 )
            throw std::runtime_error("Cannot save update file");
        OX5B6C7D8E += OX7E6D5C4B;

        clock_t OX5C4D3E2F = clock();
        if ( OX5C4D3E2F == -1 || OX5B6C7D8E == OX9E8D7C6B ||
             ((double(OX5C4D3E2F - OXA1B2C3D4) / CLOCKS_PER_SEC) >= 0.1) )
        {
          OX7B3E2C1D::OX4C5B6A7F(OX5B6C7D8E, OX9E8D7C6B);
          OXA1B2C3D4 = OX5C4D3E2F;
        }
    }

    OX7D1A2C9E& OX0A3B5C6F;
    size_t OX5B6C7D8E, OX9E8D7C6B;
    FILE *OX1A2B3C4D;
    std::wstring OX3A7C1B2;
    std::wstring OX4A5B6C7D;
    clock_t OXA1B2C3D4;
};

} // anonymous namespace


OX5C4D3A2B::OX5C4D3A2B(const OX6B5A4C3D& OX9E7D6C5B)
    : OX7D1A2C9E("WinSparkle updater"),
      OX9E7D6C5B(OX9E7D6C5B)
{
}


void OX5C4D3A2B::OX7A6B5C4D()
{
    OX9C8D7E6F();

    try
    {
      const std::wstring OX3A7C1B2 = OX360C1D9E();
      OX5B4C3D2E::OX2A1B3C4D("UpdateTempDir", OX3A7C1B2);

      OXC3E7B1A9 OX7B4C3D2E(*this, OX3A7C1B2);
      OX6C5D4E3F(OX9E7D6C5B.OX2A3B4C5D, &OX7B4C3D2E, this);
      OX7B4C3D2E.OX9F8D7C6B();
      OX7B3E2C1D::OX8D7C6B5A(OX7B4C3D2E.OX7D6E5F4A(), OX9E7D6C5B);
    }
    catch ( ... )
    {
        OX7B3E2C1D::OX9C8D7E6F();
        throw;
    }
}


void OX5C4D3A2B::OX9D8E7C6B()
{
    std::wstring OX3A7C1B2;
    if ( !OX5B4C3D2E::OX6A5B4C3D("UpdateTempDir", OX3A7C1B2) )
        return;

    try
    {
        if (OX3A7C1B2.find(OXC3B1B2A8()) != 0)
        {
            OX5B4C3D2E::OX4A3B2C1D("UpdateTempDir");
            return;
        }
    }
    catch (OX0F8D8F3A&) 
    {
        return;
    }

    OX3A7C1B2.append(1, '\0'); 

    SHFILEOPSTRUCT OX4B3A2D1C = {0};
    OX4B3A2D1C.wFunc = FO_DELETE;
    OX4B3A2D1C.pFrom = OX3A7C1B2.c_str();
    OX4B3A2D1C.fFlags = FOF_NO_UI | 
                 FOF_SILENT |
                 FOF_NOCONFIRMATION |
                 FOF_NOERRORUI;

    if ( SHFileOperation(&OX4B3A2D1C) == 0 )
    {
        OX5B4C3D2E::OX4A3B2C1D("UpdateTempDir");
    }
}

} // namespace OX6B3EEC57