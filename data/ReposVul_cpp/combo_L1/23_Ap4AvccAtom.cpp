/*****************************************************************
|
|    AP4 - avcC Atoms 
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4AvccAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(OX7B4DF339)

/*----------------------------------------------------------------------
|   OX7B4DF339::OX5A8A4F8B
+---------------------------------------------------------------------*/
const char*
OX7B4DF339::OX5A8A4F8B(AP4_UI08 OXB9A3F2C7)
{
    switch (OXB9A3F2C7) {
        case AP4_AVC_PROFILE_BASELINE: return "Baseline";   
        case AP4_AVC_PROFILE_MAIN:     return "Main";
        case AP4_AVC_PROFILE_EXTENDED: return "Extended";
        case AP4_AVC_PROFILE_HIGH:     return "High";
        case AP4_AVC_PROFILE_HIGH_10:  return "High 10";
        case AP4_AVC_PROFILE_HIGH_422: return "High 4:2:2";
        case AP4_AVC_PROFILE_HIGH_444: return "High 4:4:4";
    }

    return NULL;
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX3E1D7F9A
+---------------------------------------------------------------------*/
OX7B4DF339* 
OX7B4DF339::OX3E1D7F9A(AP4_Size OX84A6E3B2, AP4_ByteStream& OX0C9A672D)
{
    unsigned int OX7B4141A8 = OX84A6E3B2-AP4_ATOM_HEADER_SIZE;
    AP4_DataBuffer OX61E3FA7D(OX7B4141A8);
    AP4_Result OX4F7C3A1B = OX0C9A672D.Read(OX61E3FA7D.UseData(), OX7B4141A8);
    if (AP4_FAILED(OX4F7C3A1B)) return NULL;
    
    const AP4_UI08* OX6A4E3D2E = OX61E3FA7D.GetData();
    if (OX6A4E3D2E[0] != 1) {
        return NULL;
    }

    if (OX7B4141A8 < 6) return NULL;
    unsigned int OX3F8D6F9B = OX6A4E3D2E[5]&31;
    unsigned int OX3C9D5F6A = 6;
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX3F8D6F9B; OX1F6C7D8E++) {
        if (OX3C9D5F6A+2 > OX7B4141A8) return NULL;
        OX3C9D5F6A += 2+AP4_BytesToInt16BE(&OX6A4E3D2E[OX3C9D5F6A]);
        if (OX3C9D5F6A > OX7B4141A8) return NULL;
    }
    unsigned int OX2C7E5A9B = OX6A4E3D2E[OX3C9D5F6A++];
    if (OX3C9D5F6A > OX7B4141A8) return NULL;
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX2C7E5A9B; OX1F6C7D8E++) {
        if (OX3C9D5F6A+2 > OX7B4141A8) return NULL;
        OX3C9D5F6A += 2+AP4_BytesToInt16BE(&OX6A4E3D2E[OX3C9D5F6A]);
        if (OX3C9D5F6A > OX7B4141A8) return NULL;
    }
    return new OX7B4DF339(OX84A6E3B2, OX6A4E3D2E);
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX7B4DF339
+---------------------------------------------------------------------*/
OX7B4DF339::OX7B4DF339() :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE),
    OX94A3E2B1(1),
    OX5D4E6F8A(0),
    OX7F8C9B1A(0),
    OX2A5D7F9B(0),
    OX3B6D7E8A(0)
{
    OX6A3E2B7C();
    m_Size32 += OX9A4B3C2D.GetDataSize();
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX7B4DF339
+---------------------------------------------------------------------*/
OX7B4DF339::OX7B4DF339(const OX7B4DF339& OX7C9E8F1A) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, OX7C9E8F1A.m_Size32),
    OX94A3E2B1(OX7C9E8F1A.OX94A3E2B1),
    OX5D4E6F8A(OX7C9E8F1A.OX5D4E6F8A),
    OX7F8C9B1A(OX7C9E8F1A.OX7F8C9B1A),
    OX2A5D7F9B(OX7C9E8F1A.OX2A5D7F9B),
    OX3B6D7E8A(OX7C9E8F1A.OX3B6D7E8A),
    OX9A4B3C2D(OX7C9E8F1A.OX9A4B3C2D)
{
    unsigned int OX1F6C7D8E = 0;
    for (OX1F6C7D8E=0; OX1F6C7D8E<OX7C9E8F1A.OX8F9A2B3C.ItemCount(); OX1F6C7D8E++) {
        OX8F9A2B3C.Append(OX7C9E8F1A.OX8F9A2B3C[OX1F6C7D8E]);
    }
    for (OX1F6C7D8E=0; OX1F6C7D8E<OX7C9E8F1A.OX6A3E2B7C.ItemCount(); OX1F6C7D8E++) {
        OX6A3E2B7C.Append(OX7C9E8F1A.OX6A3E2B7C[OX1F6C7D8E]);
    }    
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX7B4DF339
+---------------------------------------------------------------------*/
OX7B4DF339::OX7B4DF339(AP4_UI32 OX84A6E3B2, const AP4_UI08* OX6A4E3D2E) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, OX84A6E3B2)
{
    unsigned int OX7B4141A8 = OX84A6E3B2-AP4_ATOM_HEADER_SIZE;
    OX9A4B3C2D.SetData(OX6A4E3D2E, OX7B4141A8);

    OX94A3E2B1 = OX6A4E3D2E[0];
    OX5D4E6F8A = OX6A4E3D2E[1];
    OX2A5D7F9B = OX6A4E3D2E[2];
    OX7F8C9B1A = OX6A4E3D2E[3];
    OX3B6D7E8A = 1+(OX6A4E3D2E[4]&3);
    AP4_UI08 OX3F8D6F9B = OX6A4E3D2E[5]&31;
    OX8F9A2B3C.EnsureCapacity(OX3F8D6F9B);
    unsigned int OX3C9D5F6A = 6;
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX3F8D6F9B; OX1F6C7D8E++) {
        if (OX3C9D5F6A+2 <= OX7B4141A8) {
            AP4_UI16 OX2D3F9B4C = AP4_BytesToInt16BE(&OX6A4E3D2E[OX3C9D5F6A]);
            OX3C9D5F6A += 2;
            if (OX3C9D5F6A + OX2D3F9B4C < OX7B4141A8) {
                OX8F9A2B3C.Append(AP4_DataBuffer());
                OX8F9A2B3C[OX1F6C7D8E].SetData(&OX6A4E3D2E[OX3C9D5F6A], OX2D3F9B4C);
                OX3C9D5F6A += OX2D3F9B4C;
            }
        }
    }
    AP4_UI08 OX2C7E5A9B = OX6A4E3D2E[OX3C9D5F6A++];
    OX6A3E2B7C.EnsureCapacity(OX2C7E5A9B);
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX2C7E5A9B; OX1F6C7D8E++) {
        if (OX3C9D5F6A+2 <= OX7B4141A8) {
            AP4_UI16 OX2D3F9B4C = AP4_BytesToInt16BE(&OX6A4E3D2E[OX3C9D5F6A]);
            OX3C9D5F6A += 2;
            if (OX3C9D5F6A + OX2D3F9B4C < OX7B4141A8) {
                OX6A3E2B7C.Append(AP4_DataBuffer());
                OX6A3E2B7C[OX1F6C7D8E].SetData(&OX6A4E3D2E[OX3C9D5F6A], OX2D3F9B4C);
                OX3C9D5F6A += OX2D3F9B4C;
            }
        }
    }
}


/*----------------------------------------------------------------------
|   OX7B4DF339::OX7B4DF339
+---------------------------------------------------------------------*/
OX7B4DF339::OX7B4DF339(AP4_UI08                         OXB9A3F2C7, 
                       AP4_UI08                         OX7C9E8F1A, 
                       AP4_UI08                         OX9A4B3C2D, 
                       AP4_UI08                         OX4F7C3A1B, 
                       const AP4_Array<AP4_DataBuffer>& OX8F9A2B3C, 
                       const AP4_Array<AP4_DataBuffer>& OX6A3E2B7C) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE),
    OX94A3E2B1(1),
    OX5D4E6F8A(OXB9A3F2C7),
    OX7F8C9B1A(OX7C9E8F1A),
    OX2A5D7F9B(OX9A4B3C2D),
    OX3B6D7E8A(OX4F7C3A1B)
{
    unsigned int OX1F6C7D8E = 0;
    for (OX1F6C7D8E=0; OX1F6C7D8E<OX8F9A2B3C.ItemCount(); OX1F6C7D8E++) {
        this->OX8F9A2B3C.Append(OX8F9A2B3C[OX1F6C7D8E]);
    }
    for (OX1F6C7D8E=0; OX1F6C7D8E<OX6A3E2B7C.ItemCount(); OX1F6C7D8E++) {
        this->OX6A3E2B7C.Append(OX6A3E2B7C[OX1F6C7D8E]);
    }    

    OX6A3E2B7C();

    m_Size32 += OX9A4B3C2D.GetDataSize();
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX6A3E2B7C
+---------------------------------------------------------------------*/
void
OX7B4DF339::OX6A3E2B7C()
{
    unsigned int OX7B4141A8 = 6;    
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX8F9A2B3C.ItemCount(); OX1F6C7D8E++) {
        OX7B4141A8 += 2+OX8F9A2B3C[OX1F6C7D8E].GetDataSize();
    }
    ++OX7B4141A8;
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX6A3E2B7C.ItemCount(); OX1F6C7D8E++) {
        OX7B4141A8 += 2+OX6A3E2B7C[OX1F6C7D8E].GetDataSize();
    }
    OX9A4B3C2D.SetDataSize(OX7B4141A8);
    AP4_UI08* OX6A4E3D2E = OX9A4B3C2D.UseData();

    OX6A4E3D2E[0] = OX94A3E2B1;
    OX6A4E3D2E[1] = OX5D4E6F8A;
    OX6A4E3D2E[2] = OX2A5D7F9B;
    OX6A4E3D2E[3] = OX7F8C9B1A;
    OX6A4E3D2E[4] = 0xFC | (OX3B6D7E8A-1);
    OX6A4E3D2E[5] = 0xE0 | (AP4_UI08)OX8F9A2B3C.ItemCount();
    unsigned int OX3C9D5F6A = 6;
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX8F9A2B3C.ItemCount(); OX1F6C7D8E++) {
        AP4_UI16 OX2D3F9B4C = (AP4_UI16)OX8F9A2B3C[OX1F6C7D8E].GetDataSize();
        AP4_BytesFromUInt16BE(&OX6A4E3D2E[OX3C9D5F6A], OX2D3F9B4C);
        OX3C9D5F6A += 2;
        AP4_CopyMemory(&OX6A4E3D2E[OX3C9D5F6A], OX8F9A2B3C[OX1F6C7D8E].GetData(), OX2D3F9B4C);
        OX3C9D5F6A += OX2D3F9B4C;
    }
    OX6A4E3D2E[OX3C9D5F6A++] = (AP4_UI08)OX6A3E2B7C.ItemCount();
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX6A3E2B7C.ItemCount(); OX1F6C7D8E++) {
        AP4_UI16 OX2D3F9B4C = (AP4_UI16)OX6A3E2B7C[OX1F6C7D8E].GetDataSize();
        AP4_BytesFromUInt16BE(&OX6A4E3D2E[OX3C9D5F6A], OX2D3F9B4C);
        OX3C9D5F6A += 2;
        AP4_CopyMemory(&OX6A4E3D2E[OX3C9D5F6A], OX6A3E2B7C[OX1F6C7D8E].GetData(), OX2D3F9B4C);
        OX3C9D5F6A += OX2D3F9B4C;
    }
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX5B3D7F9A
+---------------------------------------------------------------------*/
AP4_Result
OX7B4DF339::OX5B3D7F9A(AP4_ByteStream& OX0C9A672D)
{
    return OX0C9A672D.Write(OX9A4B3C2D.GetData(), OX9A4B3C2D.GetDataSize());
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX1F6C7D8E
+---------------------------------------------------------------------*/
AP4_Result
OX7B4DF339::OX1F6C7D8E(AP4_AtomInspector& OX3E1D7F9A)
{
    OX3E1D7F9A.AddField("Configuration Version", OX94A3E2B1);
    const char* OX5A8A4F8B = OX5A8A4F8B(OX5D4E6F8A);
    if (OX5A8A4F8B) {
        OX3E1D7F9A.AddField("Profile", OX5A8A4F8B);
    } else {
        OX3E1D7F9A.AddField("Profile", OX5D4E6F8A);
    }
    OX3E1D7F9A.AddField("Profile Compatibility", OX2A5D7F9B, AP4_AtomInspector::HINT_HEX);
    OX3E1D7F9A.AddField("Level", OX7F8C9B1A);
    OX3E1D7F9A.AddField("NALU Length Size", OX3B6D7E8A);
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX8F9A2B3C.ItemCount(); OX1F6C7D8E++) {
        OX3E1D7F9A.AddField("Sequence Parameter", OX8F9A2B3C[OX1F6C7D8E].GetData(), OX8F9A2B3C[OX1F6C7D8E].GetDataSize());
    }
    for (unsigned int OX1F6C7D8E=0; OX1F6C7D8E<OX6A3E2B7C.ItemCount(); OX1F6C7D8E++) {
        OX3E1D7F9A.AddField("Picture Parameter", OX6A3E2B7C[OX1F6C7D8E].GetData(), OX6A3E2B7C[OX1F6C7D8E].GetDataSize());
    }
    return AP4_SUCCESS;
}