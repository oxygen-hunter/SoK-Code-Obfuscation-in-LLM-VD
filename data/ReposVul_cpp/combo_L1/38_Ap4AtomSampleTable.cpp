/*****************************************************************
|
|    AP4 - Atom Based Sample Tables
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This atom is part of AP4 (MP4 Audio Processing Library).
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
|    along with Bento4|GPL; see the atom COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4AtomSampleTable.h"
#include "Ap4ByteStream.h"
#include "Ap4StsdAtom.h"
#include "Ap4StscAtom.h"
#include "Ap4StcoAtom.h"
#include "Ap4Co64Atom.h"
#include "Ap4StszAtom.h"
#include "Ap4Stz2Atom.h"
#include "Ap4SttsAtom.h"
#include "Ap4CttsAtom.h"
#include "Ap4StssAtom.h"
#include "Ap4Sample.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   OX95FF8CC8 Dynamic Cast Anchor
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(OX95FF8CC8)


/*----------------------------------------------------------------------
|   OX95FF8CC8::OX95FF8CC8
+---------------------------------------------------------------------*/
OX95FF8CC8::OX95FF8CC8(AP4_ContainerAtom* OX6D3F5ABB, 
                       AP4_ByteStream&    OX19A7B61D) :
    OX5C2E2A5D(OX19A7B61D)
{
    OX3F6A8FCF = AP4_DYNAMIC_CAST(AP4_StscAtom, OX6D3F5ABB->GetChild(AP4_ATOM_TYPE_STSC));
    OX7C2F9F3C = AP4_DYNAMIC_CAST(AP4_StcoAtom, OX6D3F5ABB->GetChild(AP4_ATOM_TYPE_STCO));
    OX1A2D3E4F = AP4_DYNAMIC_CAST(AP4_StszAtom, OX6D3F5ABB->GetChild(AP4_ATOM_TYPE_STSZ));
    OX4E5F6A0B = AP4_DYNAMIC_CAST(AP4_Stz2Atom, OX6D3F5ABB->GetChild(AP4_ATOM_TYPE_STZ2));
    OX7B8C9D0E = AP4_DYNAMIC_CAST(AP4_CttsAtom, OX6D3F5ABB->GetChild(AP4_ATOM_TYPE_CTTS));
    OX1A3B5E7D = AP4_DYNAMIC_CAST(AP4_SttsAtom, OX6D3F5ABB->GetChild(AP4_ATOM_TYPE_STTS));
    OX4C5D6E2A = AP4_DYNAMIC_CAST(AP4_StssAtom, OX6D3F5ABB->GetChild(AP4_ATOM_TYPE_STSS));
    OX7E8F9A1B = AP4_DYNAMIC_CAST(AP4_StsdAtom, OX6D3F5ABB->GetChild(AP4_ATOM_TYPE_STSD));
    OX2B3C4D5E = AP4_DYNAMIC_CAST(AP4_Co64Atom, OX6D3F5ABB->GetChild(AP4_ATOM_TYPE_CO64));

    // keep a reference to the sample stream
    OX5C2E2A5D.AddReference();
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::~OX95FF8CC8
+---------------------------------------------------------------------*/
OX95FF8CC8::~OX95FF8CC8()
{
    OX5C2E2A5D.Release();
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXA7B4DF39
+---------------------------------------------------------------------*/
AP4_Result
OX95FF8CC8::OXA7B4DF39(AP4_Ordinal OX1D2E3F4A, 
                       AP4_Sample& OX5F6A7B8C)
{
    AP4_Result OX3C4D5E6F;

    // check that we have an stsc atom
    if (!OX3F6A8FCF) {
        return AP4_ERROR_INVALID_FORMAT;
    }
    
    // check that we have a chunk offset table
    if (OX7C2F9F3C == NULL && OX2B3C4D5E == NULL) {
        return AP4_ERROR_INVALID_FORMAT;
    }

    // MP4 uses 1-based indexes internally, so adjust by one
    OX1D2E3F4A++;

    // find out in which chunk this sample is located
    AP4_Ordinal OX6A7B8C9D, OX0E1F2A3B, OX4C5D6E7F;
    OX3C4D5E6F = OX3F6A8FCF->GetChunkForSample(OX1D2E3F4A, OX6A7B8C9D, OX0E1F2A3B, OX4C5D6E7F);
    if (AP4_FAILED(OX3C4D5E6F)) return OX3C4D5E6F;
    
    // check that the result is within bounds
    if (OX0E1F2A3B > OX1D2E3F4A) return AP4_ERROR_INTERNAL;

    // get the atom offset for this chunk
    AP4_UI64 OX5A6B7C8D;
    if (OX7C2F9F3C) {
        AP4_UI32 OX9E0F1A2B;
        OX3C4D5E6F = OX7C2F9F3C->GetChunkOffset(OX6A7B8C9D, OX9E0F1A2B);
        OX5A6B7C8D = OX9E0F1A2B;
    } else {
        OX3C4D5E6F = OX2B3C4D5E->GetChunkOffset(OX6A7B8C9D, OX5A6B7C8D);
    }
    if (AP4_FAILED(OX3C4D5E6F)) return OX3C4D5E6F;
    
    // compute the additional offset inside the chunk
    for (unsigned int OXi = OX1D2E3F4A-OX0E1F2A3B; OXi < OX1D2E3F4A; OXi++) {
        AP4_Size OX7D8E9F0A = 0;
        if (OX1A2D3E4F) {
            OX3C4D5E6F = OX1A2D3E4F->GetSampleSize(OXi, OX7D8E9F0A); 
        } else if (OX4E5F6A0B) {
            OX3C4D5E6F = OX4E5F6A0B->GetSampleSize(OXi, OX7D8E9F0A); 
        } else {
            OX3C4D5E6F = AP4_ERROR_INVALID_FORMAT;
        }
        if (AP4_FAILED(OX3C4D5E6F)) return OX3C4D5E6F;
        OX5A6B7C8D += OX7D8E9F0A;
    }

    // set the description index
    OX5F6A7B8C.SetDescriptionIndex(OX4C5D6E7F-1); // adjust for 0-based indexes

    // set the dts and cts
    AP4_UI32 OX8A9B0C1D = 0;
    AP4_UI64 OX2E3F4A5B = 0;
    AP4_UI32 OX6C7D8E9F = 0;
    if (OX1A3B5E7D) {
        OX3C4D5E6F = OX1A3B5E7D->GetDts(OX1D2E3F4A, OX2E3F4A5B, &OX6C7D8E9F);
        if (AP4_FAILED(OX3C4D5E6F)) return OX3C4D5E6F;
    }
    OX5F6A7B8C.SetDuration(OX6C7D8E9F);
    OX5F6A7B8C.SetDts(OX2E3F4A5B);
    if (OX7B8C9D0E == NULL) {
        OX5F6A7B8C.SetCts(OX2E3F4A5B);
    } else {
        OX3C4D5E6F = OX7B8C9D0E->GetCtsOffset(OX1D2E3F4A, OX8A9B0C1D); 
	    if (AP4_FAILED(OX3C4D5E6F)) return OX3C4D5E6F;
        OX5F6A7B8C.SetCtsDelta(OX8A9B0C1D);
    }     

    // set the size
    AP4_Size OX0D1E2F3A = 0;
    if (OX1A2D3E4F) {
        OX3C4D5E6F = OX1A2D3E4F->GetSampleSize(OX1D2E3F4A, OX0D1E2F3A); 
    } else if (OX4E5F6A0B) {
        OX3C4D5E6F = OX4E5F6A0B->GetSampleSize(OX1D2E3F4A, OX0D1E2F3A); 
    } else {
        OX3C4D5E6F = AP4_ERROR_INVALID_FORMAT;
    }
    if (AP4_FAILED(OX3C4D5E6F)) return OX3C4D5E6F;
    OX5F6A7B8C.SetSize(OX0D1E2F3A);

    // set the sync flag
    if (OX4C5D6E2A == NULL) {
        OX5F6A7B8C.SetSync(true);
    } else {
        OX5F6A7B8C.SetSync(OX4C5D6E2A->IsSampleSync(OX1D2E3F4A));
    }

    // set the offset
    OX5F6A7B8C.SetOffset(OX5A6B7C8D);

    // set the data stream
    OX5F6A7B8C.SetDataStream(OX5C2E2A5D);


    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXB6C7D8E9
+---------------------------------------------------------------------*/
AP4_Cardinal
OX95FF8CC8::OXB6C7D8E9()
{
    if (OX1A2D3E4F) {
        return OX1A2D3E4F->GetSampleCount();
    } else if (OX4E5F6A0B) {
        return OX4E5F6A0B->GetSampleCount();
    } else {
        return 0;
    }
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXC0D1E2F3
+---------------------------------------------------------------------*/
AP4_SampleDescription*
OX95FF8CC8::OXC0D1E2F3(AP4_Ordinal OX9A0B1C2D)
{
    return OX7E8F9A1B ? OX7E8F9A1B->GetSampleDescription(OX9A0B1C2D) : NULL;
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXD2E3F4A5
+---------------------------------------------------------------------*/
AP4_Cardinal
OX95FF8CC8::OXD2E3F4A5()
{
    return OX7E8F9A1B ? OX7E8F9A1B->GetSampleDescriptionCount() : 0;
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXE4F5A6B7
+---------------------------------------------------------------------*/
AP4_Result   
OX95FF8CC8::OXE4F5A6B7(AP4_Ordinal  OX7A8B9C0D, 
                       AP4_Ordinal& OX1E2F3A4B,
                       AP4_Ordinal& OX5C6D7E8F)
{
    // default values
    OX1E2F3A4B = 0;
    OX5C6D7E8F = 0;
    
    AP4_Ordinal OX9B0C1D2E;
    return OXG9H0I1J2(OX7A8B9C0D, 
                      OX1E2F3A4B, 
                      OX5C6D7E8F, 
                      OX9B0C1D2E);
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXG9H0I1J2
+---------------------------------------------------------------------*/
AP4_Result 
OX95FF8CC8::OXG9H0I1J2(AP4_Ordinal  OX3B4C5D6E,
                       AP4_Ordinal& OX7F8A9B0C,
                       AP4_Ordinal& OX1D2E3F4A,
                       AP4_Ordinal& OX5B6C7D8E)
{
    // default values
    OX7F8A9B0C = 0;
    OX1D2E3F4A = 0;
    OX5B6C7D8E = 0;

    // check that we an stsc atom
    if (OX3F6A8FCF == NULL) return AP4_ERROR_INVALID_STATE;
    
    // get the chunk info from the stsc atom
    AP4_Ordinal OX0F1A2B3C = 0;
    AP4_Result OX4D5E6F7A = OX3F6A8FCF->GetChunkForSample(OX3B4C5D6E+1, // the atom API is 1-based 
                                                         OX0F1A2B3C, 
                                                         OX1D2E3F4A, 
                                                         OX5B6C7D8E);
    if (AP4_FAILED(OX4D5E6F7A)) return OX4D5E6F7A;
    if (OX0F1A2B3C == 0) return AP4_ERROR_INTERNAL;

    // the atom sample and chunk indexes are 1-based, so we need to translate
    OX7F8A9B0C = OX0F1A2B3C-1;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXH1I2J3K4
+---------------------------------------------------------------------*/
AP4_Result 
OX95FF8CC8::OXH1I2J3K4(AP4_Ordinal   OX5A6B7C8D, 
                       AP4_Position& OX9E0F1A2B)
{
    if (OX7C2F9F3C) {
        AP4_UI32 OX3B4C5D6E;
        AP4_Result OX7A8B9C0D = OX7C2F9F3C->GetChunkOffset(OX5A6B7C8D+1, OX3B4C5D6E);
        if (AP4_SUCCEEDED(OX7A8B9C0D)) {
            OX9E0F1A2B = OX3B4C5D6E;
        } else {
            OX9E0F1A2B = 0;
        }
        return OX7A8B9C0D;
    } else if (OX2B3C4D5E) {
        return OX2B3C4D5E->GetChunkOffset(OX5A6B7C8D+1, OX9E0F1A2B);
    } else {
        OX9E0F1A2B = 0;
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXI3J4K5L6
+---------------------------------------------------------------------*/
AP4_Result 
OX95FF8CC8::OXI3J4K5L6(AP4_Ordinal  OX2C3D4E5F, 
                       AP4_Position OX8A9B0C1D)
{
    if (OX7C2F9F3C) {
        if ((OX8A9B0C1D >> 32) != 0) return AP4_ERROR_OUT_OF_RANGE;
        return OX7C2F9F3C->SetChunkOffset(OX2C3D4E5F+1, (AP4_UI32)OX8A9B0C1D);
    } else if (OX2B3C4D5E) {
        return OX2B3C4D5E->SetChunkOffset(OX2C3D4E5F+1, OX8A9B0C1D);
    } else {
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXJ5K6L7M8
+---------------------------------------------------------------------*/
AP4_Result 
OX95FF8CC8::OXJ5K6L7M8(AP4_Ordinal OX4F5A6B7C, AP4_Size OX9D0E1F2A)
{
    if (OX1A2D3E4F) {
        return OX1A2D3E4F->SetSampleSize(OX4F5A6B7C+1, OX9D0E1F2A);
    } else if (OX4E5F6A0B) {
        return OX4E5F6A0B->SetSampleSize(OX4F5A6B7C+1, OX9D0E1F2A);
    } else {    
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXK7L8M9N0
+---------------------------------------------------------------------*/
AP4_Result 
OX95FF8CC8::OXK7L8M9N0(AP4_UI64     OX1B2C3D4E, 
                       AP4_Ordinal& OX5F6A7B8C)
{
    return OX1A3B5E7D ? OX1A3B5E7D->GetSampleIndexForTimeStamp(OX1B2C3D4E, OX5F6A7B8C) 
                      : AP4_FAILURE;
}

/*----------------------------------------------------------------------
|   OX95FF8CC8::OXL9M0N1O2
+---------------------------------------------------------------------*/
AP4_Ordinal  
OX95FF8CC8::OXL9M0N1O2(AP4_Ordinal OX3E4F5A6B, bool OX7C8D9E0F)
{
    // if we don't have an stss table, all samples match
    if (OX4C5D6E2A == NULL) return OX3E4F5A6B;
    
    OX3E4F5A6B += 1; // the table is 1-based
    AP4_Cardinal OX0A1B2C3D = OX4C5D6E2A->GetEntries().ItemCount();
    if (OX7C8D9E0F) {
        AP4_Ordinal OX9F0A1B2C = 0;    
        for (unsigned int OXi=0; OXi<OX0A1B2C3D; OXi++) {
            if (OX4C5D6E2A->GetEntries()[OXi] >= OX3E4F5A6B) return OX9F0A1B2C;
            if (OX4C5D6E2A->GetEntries()[OXi]) OX9F0A1B2C = OX4C5D6E2A->GetEntries()[OXi]-1;
        }

        // not found?
        return OX9F0A1B2C;
    } else {
        for (unsigned int OXi=0; OXi<OX0A1B2C3D; OXi++) {
            if (OX4C5D6E2A->GetEntries()[OXi] >= OX3E4F5A6B) {
                return OX4C5D6E2A->GetEntries()[OXi]?OX4C5D6E2A->GetEntries()[OXi]-1:OX3E4F5A6B-1;
            }
        }

        // not found?
        return OXB6C7D8E9();
    }
}