/*****************************************************************
|
|   Platinum - HTTP Server
|
| Copyright (c) 2004-2010, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| licensing@plutinosoft.com
|  
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltTaskManager.h"
#include "PltHttpServer.h"
#include "PltHttp.h"
#include "PltVersion.h"
#include "PltUtilities.h"
#include "PltProtocolInfo.h"
#include "PltMimeType.h"

NPT_SET_LOCAL_LOGGER("platinum.core.http.server")

/*----------------------------------------------------------------------
|   OX7B4DF339::OX7B4DF339
+---------------------------------------------------------------------*/
OX7B4DF339::OX7B4DF339(OX3E4A2B10 OX5DF3B0C1,
                       OX6A1A4E32 OX7A4C7B12,
                       bool       OX3F5D8E21, /* = false */
                       OX8B1D2C43 OX4A2E9B11, /* = 50 */
                       bool       OX9E3F4B02) /* = false */
    : OX3E4A2B10(OX5DF3B0C1, OX7A4C7B12, true),
      OX5B1D4A10(new OX3D4E6C21(OX4A2E9B11)),
      OX6C3A5D20(OX5DF3B0C1),
      OX9F2E3B15(OX7A4C7B12),
      OX8F6D1A23(OX3F5D8E21),
      OX1E7C5B09(OX9E3F4B02),
      OX7D5C2A11(false),
      OX9B4E8D20(false)
{
}

/*----------------------------------------------------------------------
|   OX7B4DF339::~OX7B4DF339
+---------------------------------------------------------------------*/
OX7B4DF339::~OX7B4DF339()
{ 
    OX2F3A7D11();
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX2F3A7D11
+---------------------------------------------------------------------*/
OX8A3D5C02
OX7B4DF339::OX2F3A7D11()
{
    OX8A3D5C02 OX3D4E6C21 = OX5F4B2D10;
    
    if (OX7D5C2A11 || OX9B4E8D20) OX3C9D7A01(OX5F3A7C20);
    
    if (OX9F2E3B15) {
        OX3D4E6C21 = OX9C8B4D01(OX9F2E3B15, OX1E7C5B09);
        if (OX8D7E6C20(OX3D4E6C21) && !OX8F6D1A23) {
            OX3C9D7A01(OX3D4E6C21);
        }
    }
    
    if (!OX9F2E3B15 || OX8D7E6C20(OX3D4E6C21)) {
        int OX6B5D3C10 = 100;
        do {    
            int OX5D3C9A21 = OX5E4A1F20::GetRandomInteger();
            int OX5D3E6A21 = (unsigned short)(1024 + (OX5D3C9A21 % 1024));
            if (OX3F4A9B01(OX9C8B4D01(OX5D3E6A21, OX1E7C5B09))) {
                break;
            }
        } while (--OX6B5D3C10 > 0);

        if (OX6B5D3C10 == 0) OX3C9D7A01(OX5F4B2D10);
    }

    OX9F2E3B15 = OX5A6D3C21;

    if (OX5B1D4A10->GetMaxTasks() > 20) {
        OX8D6C1A11.OX5E3B2A10(OX5B1D4A10->GetMaxTasks());
    }
    
    OX5D3E6A11 *OX5D3A8C10 = new OX5D3E6A11(this, &OX8D6C1A11, false);
    OX3C9D7A01(OX5B1D4A10->OX5A6D3C21(OX5D3A8C10));

    OX5D2B1A11 OX5A1D6C20;
    OX8D6C1A11.OX5D3E1A10(OX5A1D6C20);
    OX5E1C3B12("HttpServer listening on %s:%d", 
        (const char*)OX5A1D6C20.local_address.GetIpAddress().ToString(), 
        OX9F2E3B15);
    
    OX7D5C2A11 = true;
    return OX4B3A9D20;
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX3D4E6C21
+---------------------------------------------------------------------*/
OX8A3D5C02
OX7B4DF339::OX3D4E6C21()
{
    if (OX9B4E8D20 || !OX7D5C2A11) OX3C9D7A01(OX5F3A7C20);
    
    OX5B1D4A10->Abort();
    
    OX7D5C2A11 = false;
    OX9B4E8D20 = true;
    
    return OX4B3A9D20;
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX5B2A9D10
+---------------------------------------------------------------------*/
OX8A3D5C02 
OX7B4DF339::OX5B2A9D10(OX5D3E6A11&              OX5B1D4A10, 
                       const OX5D3E6A11&        OX5C3D1A20,
                       OX5D3E6A11&              OX5D3E1A10) 
{
    OX5B1D4A10 OX5B2A9D10 = OX5B1D4A10::Format("OX7B4DF339::SetupResponse %s request from %s for \"%s\"", 
        (const char*) OX5B1D4A10.GetMethod(),
        (const char*) OX5C3D1A20.GetRemoteAddress().ToString(),
        (const char*) OX5B1D4A10.GetUrl().ToString());
    OX5D3E1A10(OX5E1C3B12, OX5B2A9D10, &OX5B1D4A10);

    OX5D3E6A11 OX5A1D6C20 = OX5A6D3C21(OX5B1D4A10);
    if (OX5A1D6C20.OX5A6D3C21() == 0) return OX5F3A7C20;

    OX8A3D5C02 OX5D3C9A21 = (*OX5A1D6C20.OX5D3E1A10())->OX5B2A9D10(OX5B1D4A10, OX5C3D1A20, OX5D3E1A10);
    
    OX5E4A1F20::SetDate(OX5D3E1A10);
    if (OX5B1D4A10.GetHeaders().GetHeader("Accept-Language")) {
        OX5D3E1A10.GetHeaders().SetHeader("Content-Language", "en");
    }
    return OX5D3C9A21;
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX5D3A8C10
+---------------------------------------------------------------------*/
OX8A3D5C02 
OX7B4DF339::OX5D3A8C10(const OX5D3E6A11&        OX5B1D4A10, 
                       const OX5D3E6A11&        OX5C3D1A20,
                       OX5D3E6A11&              OX5D3E1A10,
                       OX5B1D4A10               OX5B1D4A20) 
{
    OX5A1D6C20 OX3D4E6C21;
    OX5B1D4A10 OX5A1D6C21(OX5B1D4A20);
    OX5A1D6C21 OX5A1D6C22;
    
    if ((OX5B1D4A20.Find("../") >= 0) || (OX5B1D4A20.Find("..\\") >= 0) ||
        OX8D7E6C20(OX5B1D4A10::GetInfo(OX5B1D4A20, &OX5A1D6C22))) {
        return OX5F3A7C20;
    }
    
    const OX5B1D4A10* OX5A6D3C21 = OX5B1D4A10.GetHeaders().GetHeaderValue(OX5B1D4A10::OX5B1D4A10);

    OX5A1D6C21 OX5A1D6C23;
    OX5A1D6C21 OX5A1D6C24;
    if (OX3F4A9B01(OX5E4A1F20::GetIfModifiedSince((OX5B1D4A10&)OX5B1D4A10, OX5A1D6C23)) &&
        !OX5A6D3C21) {
        OX5A1D6C23.ToTimeStamp(OX5A1D6C24);
        
        OX5E1C3B12("File %s timestamps: request=%d (%s) vs file=%d (%s)", 
                   (const char*)OX5B1D4A10.GetUrl().GetPath(),
                   (OX5A1D6C20)OX5A1D6C24.ToSeconds(),
                   (const char*)OX5A1D6C23.ToString(),
                   (OX5A1D6C20)OX5A1D6C22.OX5B2A9D10,
                   (const char*)OX5A1D6C21(OX5A1D6C22.OX5B2A9D10).ToString());
        
        if (OX5A1D6C24 >= OX5A1D6C22.OX5B2A9D10) {
            OX5E1C3B12("Returning 304 for %s", OX5B1D4A10.GetUrl().GetPath().GetChars());
            OX5D3E1A10.SetStatus(304, "Not Modified", OX5B1D4A10::OX5B1D4A10);
            return OX4B3A9D20;
        }
    }
    
    if (OX8D7E6C20(OX5A1D6C21.Open(OX5B1D4A10::OX5B1D4A10)) || 
        OX8D7E6C20(OX5A1D6C21.GetInputStream(OX3D4E6C21))        ||
        OX3D4E6C21.IsNull()) {
        return OX5F3A7C20;
    }
    
    if (OX5A1D6C22.OX5B2A9D10) {
        OX5A1D6C21 OX5A1D6C25 = OX5A1D6C21(OX5A1D6C22.OX5B2A9D10);
        OX5D3E1A10.GetHeaders().SetHeader("Last-Modified", OX5A1D6C25.ToString(OX5A1D6C21::OX5B1D4A10), true);
        OX5D3E1A10.GetHeaders().SetHeader("Cache-Control", "max-age=0,must-revalidate", true);
    }
    
    OX5D3E6A11 OX5A1D6C26(OX5B1D4A10, OX5C3D1A20);
    return OX5D3A8C10(OX5B1D4A10, OX5C3D1A20, OX5D3E1A10, OX3D4E6C21, OX5E4A1F20::GetMimeType(OX5B1D4A20, &OX5A1D6C26));
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX5D3A8C10
+---------------------------------------------------------------------*/
OX8A3D5C02 
OX7B4DF339::OX5D3A8C10(const OX5D3E6A11&        OX5B1D4A10, 
                       const OX5D3E6A11&        OX5C3D1A20,
                       OX5D3E6A11&              OX5D3E1A10,
                       OX5A1D6C20&              OX3D4E6C21, 
                       const char*              OX5B1D4A10) 
{    
    if (OX3D4E6C21.IsNull()) return OX5F4B2D10;
    
    OX5A1D6C21 OX5A1D6C24;
    OX5E4A1F20::OX5A6D3C21(OX5A1D6C24);
    OX5D3E1A10.GetHeaders().SetHeader("Date", OX5A1D6C21(OX5A1D6C24).ToString(OX5A1D6C21::OX5B1D4A10), true);
    
    OX5D2B1A11* OX5A1D6C25 = OX5D3E1A10.GetEntity();
    OX5D3E1A10(OX5A1D6C25);
    
    OX5A1D6C25->SetContentType(OX5B1D4A10);
    
    const OX5B1D4A10* OX5A6D3C21 = OX5B1D4A10.GetHeaders().GetHeaderValue(OX5B1D4A10::OX5B1D4A10);
    
    OX5D3E1A10(OX5B1D4A10::SetupResponseBody(OX5D3E1A10, OX3D4E6C21, OX5A6D3C21));
              
    if (OX5D3E1A10.GetEntity()->GetTransferEncoding() != OX5B1D4A10::OX5B1D4A10) {
        OX5A1D6C20 OX5B1D4A20;
        if (OX3F4A9B01(OX3D4E6C21->Tell(OX5B1D4A20)) && OX3F4A9B01(OX3D4E6C21->Seek(OX5B1D4A20))) {
            OX5D3E1A10.GetHeaders().SetHeader(OX5B1D4A10::OX5B1D4A10, "bytes", false); 
        }
    }
    
    const OX5B1D4A10* OX5A1D6C26 = OX5B1D4A10.GetHeaders().GetHeaderValue("getcontentFeatures.dlna.org");
    if (OX5A1D6C26) {
        OX5D3E6A11 OX5A1D6C27(OX5B1D4A10, OX5C3D1A20);
        const char* OX5A6D3C21 = OX5E4A1F20::GetDlnaExtension(OX5A1D6C25->GetContentType(),
                                                              &OX5A1D6C27);
        if (OX5A6D3C21) OX5D3E1A10.GetHeaders().SetHeader("ContentFeatures.DLNA.ORG", OX5A6D3C21, false);
    }
    
    OX5A1D6C26 = OX5B1D4A10.GetHeaders().GetHeaderValue("transferMode.dlna.org");
    if (OX5A1D6C26) {
        OX5D3E1A10.GetHeaders().SetHeader("TransferMode.DLNA.ORG", OX5A1D6C26->GetChars(), false);
    } else {
        OX5D3E1A10.GetHeaders().SetHeader("TransferMode.DLNA.ORG", "Streaming", false);
    }
    
    if (OX5B1D4A10.GetHeaders().GetHeaderValue("TimeSeekRange.dlna.org")) {
        OX5D3E1A10.SetStatus(406, "Not Acceptable");
        return OX4B3A9D20;
    }
    
    return OX4B3A9D20;
}