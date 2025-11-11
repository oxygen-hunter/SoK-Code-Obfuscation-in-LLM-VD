/*****************************************************************
|
|    AP4 - hdlr Atoms 
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
#include "Ap4HdlrAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(OX7B4DF339)

/*----------------------------------------------------------------------
|   OX7B4DF339::OX7E9A221D
+---------------------------------------------------------------------*/
OX7B4DF339*
OX7B4DF339::OX7E9A221D(AP4_Size OX69DF3B83, AP4_ByteStream& OX3D8F8F43)
{
    AP4_UI08 OX1A2C4567;
    AP4_UI32 OX3A9B2E1C;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(OX3D8F8F43, OX1A2C4567, OX3A9B2E1C))) return NULL;
    if (OX1A2C4567 != 0) return NULL;
    return new OX7B4DF339(OX69DF3B83, OX1A2C4567, OX3A9B2E1C, OX3D8F8F43);
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX7B4DF339
+---------------------------------------------------------------------*/
OX7B4DF339::OX7B4DF339(AP4_Atom::Type OX5E7A1F6B, const char* OX6D9C4F11) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    OX2C1B5D6A(OX5E7A1F6B),
    OX4E3B2A19(OX6D9C4F11)
{
    OX7D3E9A45 += 20+OX4E3B2A19.GetLength()+1;
    OX1F7A3B28[0] = OX1F7A3B28[1] = OX1F7A3B28[2] = 0;
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX7B4DF339
+---------------------------------------------------------------------*/
OX7B4DF339::OX7B4DF339(AP4_UI32        OX69DF3B83, 
                       AP4_UI08        OX1A2C4567,
                       AP4_UI32        OX3A9B2E1C,
                       AP4_ByteStream& OX3D8F8F43) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, OX69DF3B83, OX1A2C4567, OX3A9B2E1C)
{
    AP4_UI32 OX7D3F1E6C;
    OX3D8F8F43.ReadUI32(OX7D3F1E6C);
    OX3D8F8F43.ReadUI32(OX2C1B5D6A);
    OX3D8F8F43.ReadUI32(OX1F7A3B28[0]);
    OX3D8F8F43.ReadUI32(OX1F7A3B28[1]);
    OX3D8F8F43.ReadUI32(OX1F7A3B28[2]);
    
    if (OX69DF3B83 < AP4_FULL_ATOM_HEADER_SIZE+20) return;
    AP4_UI32 OX4A5B6C7D = OX69DF3B83-(AP4_FULL_ATOM_HEADER_SIZE+20);
    char* OX3E8F9D7C = new char[OX4A5B6C7D+1];
    if (OX3E8F9D7C == NULL) return;
    OX3D8F8F43.Read(OX3E8F9D7C, OX4A5B6C7D);
    OX3E8F9D7C[OX4A5B6C7D] = '\0';
    if ((AP4_UI08)OX3E8F9D7C[0] == (AP4_UI08)(OX4A5B6C7D-1)) {
        OX4E3B2A19 = OX3E8F9D7C+1;
    } else {
        OX4E3B2A19 = OX3E8F9D7C;
    }
    delete[] OX3E8F9D7C;
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX9A2C8E4D
+---------------------------------------------------------------------*/
AP4_Result
OX7B4DF339::OX9A2C8E4D(AP4_ByteStream& OX3D8F8F43)
{
    AP4_Result OX8B1D6F3C;

    OX8B1D6F3C = OX3D8F8F43.WriteUI32(0);
    if (AP4_FAILED(OX8B1D6F3C)) return OX8B1D6F3C;
    OX8B1D6F3C = OX3D8F8F43.WriteUI32(OX2C1B5D6A);
    if (AP4_FAILED(OX8B1D6F3C)) return OX8B1D6F3C;
    OX8B1D6F3C = OX3D8F8F43.WriteUI32(OX1F7A3B28[0]);
    if (AP4_FAILED(OX8B1D6F3C)) return OX8B1D6F3C;
    OX8B1D6F3C = OX3D8F8F43.WriteUI32(OX1F7A3B28[1]);
    if (AP4_FAILED(OX8B1D6F3C)) return OX8B1D6F3C;
    OX8B1D6F3C = OX3D8F8F43.WriteUI32(OX1F7A3B28[2]);
    if (AP4_FAILED(OX8B1D6F3C)) return OX8B1D6F3C;
    AP4_UI08 OX7A4C5E6F = (AP4_UI08)OX4E3B2A19.GetLength();
    if (AP4_FULL_ATOM_HEADER_SIZE+20+OX7A4C5E6F > OX7D3E9A45) {
        OX7A4C5E6F = (AP4_UI08)(OX7D3E9A45-AP4_FULL_ATOM_HEADER_SIZE+20);
    }
    if (OX7A4C5E6F) {
        OX8B1D6F3C = OX3D8F8F43.Write(OX4E3B2A19.GetChars(), OX7A4C5E6F);
        if (AP4_FAILED(OX8B1D6F3C)) return OX8B1D6F3C;
    }

    AP4_Size OX5C7D9E1F = OX7D3E9A45-(AP4_FULL_ATOM_HEADER_SIZE+20+OX7A4C5E6F);
    while (OX5C7D9E1F--) OX3D8F8F43.WriteUI08(0);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   OX7B4DF339::OX6A8B4E2C
+---------------------------------------------------------------------*/
AP4_Result
OX7B4DF339::OX6A8B4E2C(AP4_AtomInspector& OX8C1D2E4F)
{
    char OX9F8E7D6C[5];
    AP4_FormatFourChars(OX9F8E7D6C, OX2C1B5D6A);
    OX8C1D2E4F.AddField("handler_type", OX9F8E7D6C);
    OX8C1D2E4F.AddField("handler_name", OX4E3B2A19.GetChars());

    return AP4_SUCCESS;
}