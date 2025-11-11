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
|   AP4_AtomSampleTable Dynamic Cast Anchor
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(OX7B4DF339)


/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::AP4_AtomSampleTable
+---------------------------------------------------------------------*/
OX7B4DF339::OX7B4DF339(AP4_ContainerAtom* OX7BC19F64, 
                                         AP4_ByteStream&    OX391C0B91) :
    OX4A22F453(OX391C0B91)
{
    OX6F9E4B6D = AP4_DYNAMIC_CAST(AP4_StscAtom, OX7BC19F64->GetChild(AP4_ATOM_TYPE_STSC));
    OX3C0A4BBD = AP4_DYNAMIC_CAST(AP4_StcoAtom, OX7BC19F64->GetChild(AP4_ATOM_TYPE_STCO));
    OX6E9D7F4A = AP4_DYNAMIC_CAST(AP4_StszAtom, OX7BC19F64->GetChild(AP4_ATOM_TYPE_STSZ));
    OX2E1FB237 = AP4_DYNAMIC_CAST(AP4_Stz2Atom, OX7BC19F64->GetChild(AP4_ATOM_TYPE_STZ2));
    OX48A7A8B5 = AP4_DYNAMIC_CAST(AP4_CttsAtom, OX7BC19F64->GetChild(AP4_ATOM_TYPE_CTTS));
    OX199A1B34 = AP4_DYNAMIC_CAST(AP4_SttsAtom, OX7BC19F64->GetChild(AP4_ATOM_TYPE_STTS));
    OX3DF1C2F9 = AP4_DYNAMIC_CAST(AP4_StssAtom, OX7BC19F64->GetChild(AP4_ATOM_TYPE_STSS));
    OX4EEA3A2F = AP4_DYNAMIC_CAST(AP4_StsdAtom, OX7BC19F64->GetChild(AP4_ATOM_TYPE_STSD));
    OX5A0C4E0D = AP4_DYNAMIC_CAST(AP4_Co64Atom, OX7BC19F64->GetChild(AP4_ATOM_TYPE_CO64));

    // keep a reference to the sample stream
    OX4A22F453.AddReference();
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::~AP4_AtomSampleTable
+---------------------------------------------------------------------*/
OX7B4DF339::~OX7B4DF339()
{
    OX4A22F453.Release();
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSample
+---------------------------------------------------------------------*/
AP4_Result
OX7B4DF339::OX1EFD4D53(AP4_Ordinal OX2C9E3EFC, 
                               AP4_Sample& OXE6E9B6ED)
{
    AP4_Result OX2B3E7F5E;

    // check that we have an stsc atom
    if (!OX6F9E4B6D) {
        return AP4_ERROR_INVALID_FORMAT;
    }
    
    // check that we have a chunk offset table
    if (OX3C0A4BBD == NULL && OX5A0C4E0D == NULL) {
        return AP4_ERROR_INVALID_FORMAT;
    }

    // MP4 uses 1-based indexes internally, so adjust by one
    OX2C9E3EFC++;

    // find out in which chunk this sample is located
    AP4_Ordinal OX61B1E0A1, OX6E7D9F4D, OX4E3E1F2B;
    OX2B3E7F5E = OX6F9E4B6D->GetChunkForSample(OX2C9E3EFC, OX61B1E0A1, OX6E7D9F4D, OX4E3E1F2B);
    if (AP4_FAILED(OX2B3E7F5E)) return OX2B3E7F5E;
    
    // check that the result is within bounds
    if (OX6E7D9F4D > OX2C9E3EFC) return AP4_ERROR_INTERNAL;

    // get the atom offset for this chunk
    AP4_UI64 OX4E7C1A3D;
    if (OX3C0A4BBD) {
        AP4_UI32 OX6A2D7E4C;
        OX2B3E7F5E = OX3C0A4BBD->GetChunkOffset(OX61B1E0A1, OX6A2D7E4C);
        OX4E7C1A3D = OX6A2D7E4C;
    } else {
        OX2B3E7F5E = OX5A0C4E0D->GetChunkOffset(OX61B1E0A1, OX4E7C1A3D);
    }
    if (AP4_FAILED(OX2B3E7F5E)) return OX2B3E7F5E;
    
    // compute the additional offset inside the chunk
    for (unsigned int OX7E3F9C5D = OX2C9E3EFC-OX6E7D9F4D; OX7E3F9C5D < OX2C9E3EFC; OX7E3F9C5D++) {
        AP4_Size OX5F6C3A2B = 0;
        if (OX6E9D7F4A) {
            OX2B3E7F5E = OX6E9D7F4A->GetSampleSize(OX7E3F9C5D, OX5F6C3A2B); 
        } else if (OX2E1FB237) {
            OX2B3E7F5E = OX2E1FB237->GetSampleSize(OX7E3F9C5D, OX5F6C3A2B); 
        } else {
            OX2B3E7F5E = AP4_ERROR_INVALID_FORMAT;
        }
        if (AP4_FAILED(OX2B3E7F5E)) return OX2B3E7F5E;
        OX4E7C1A3D += OX5F6C3A2B;
    }

    // set the description index
    OXE6E9B6ED.SetDescriptionIndex(OX4E3E1F2B-1); // adjust for 0-based indexes

    // set the dts and cts
    AP4_UI32 OX3CABF6D1 = 0;
    AP4_UI64 OX3E1F7A4C        = 0;
    AP4_UI32 OX7A2E5D3B   = 0;
    OX2B3E7F5E = OX199A1B34->GetDts(OX2C9E3EFC, OX3E1F7A4C, &OX7A2E5D3B);
    if (AP4_FAILED(OX2B3E7F5E)) return OX2B3E7F5E;
    OXE6E9B6ED.SetDuration(OX7A2E5D3B);
    OXE6E9B6ED.SetDts(OX3E1F7A4C);
    if (OX48A7A8B5 == NULL) {
        OXE6E9B6ED.SetCts(OX3E1F7A4C);
    } else {
        OX2B3E7F5E = OX48A7A8B5->GetCtsOffset(OX2C9E3EFC, OX3CABF6D1); 
	    if (AP4_FAILED(OX2B3E7F5E)) return OX2B3E7F5E;
        OXE6E9B6ED.SetCtsDelta(OX3CABF6D1);
    }     

    // set the size
    AP4_Size OX3C2E6A1B = 0;
    if (OX6E9D7F4A) {
        OX2B3E7F5E = OX6E9D7F4A->GetSampleSize(OX2C9E3EFC, OX3C2E6A1B); 
    } else if (OX2E1FB237) {
        OX2B3E7F5E = OX2E1FB237->GetSampleSize(OX2C9E3EFC, OX3C2E6A1B); 
    } else {
        OX2B3E7F5E = AP4_ERROR_INVALID_FORMAT;
    }
    if (AP4_FAILED(OX2B3E7F5E)) return OX2B3E7F5E;
    OXE6E9B6ED.SetSize(OX3C2E6A1B);

    // set the sync flag
    if (OX3DF1C2F9 == NULL) {
        OXE6E9B6ED.SetSync(true);
    } else {
        OXE6E9B6ED.SetSync(OX3DF1C2F9->IsSampleSync(OX2C9E3EFC));
    }

    // set the offset
    OXE6E9B6ED.SetOffset(OX4E7C1A3D);

    // set the data stream
    OXE6E9B6ED.SetDataStream(OX4A22F453);


    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleCount
+---------------------------------------------------------------------*/
AP4_Cardinal
OX7B4DF339::OX6A3E4D1B()
{
    if (OX6E9D7F4A) {
        return OX6E9D7F4A->GetSampleCount();
    } else if (OX2E1FB237) {
        return OX2E1FB237->GetSampleCount();
    } else {
        return 0;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription*
OX7B4DF339::OX5C1F3B2A(AP4_Ordinal OX2C9E3EFC)
{
    return OX4EEA3A2F ? OX4EEA3A2F->GetSampleDescription(OX2C9E3EFC) : NULL;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleDescriptionCount
+---------------------------------------------------------------------*/
AP4_Cardinal
OX7B4DF339::OX7E3F9C5D()
{
    return OX4EEA3A2F ? OX4EEA3A2F->GetSampleDescriptionCount() : 0;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleChunkPosition
+---------------------------------------------------------------------*/
AP4_Result   
OX7B4DF339::OX5A2E6D3C(AP4_Ordinal  OX2C9E3EFC, 
                                            AP4_Ordinal& OX61B1E0A1,
                                            AP4_Ordinal& OX6E7D9F4D)
{
    // default values
    OX61B1E0A1       = 0;
    OX6E7D9F4D = 0;
    
    AP4_Ordinal OX4E3E1F2B;
    return OX5C1D7B4A(OX2C9E3EFC, 
                             OX61B1E0A1, 
                             OX6E7D9F4D, 
                             OX4E3E1F2B);
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetChunkForSample
+---------------------------------------------------------------------*/
AP4_Result 
OX7B4DF339::OX5C1D7B4A(AP4_Ordinal  OX2C9E3EFC,
                                       AP4_Ordinal& OX61B1E0A1,
                                       AP4_Ordinal& OX6E7D9F4D,
                                       AP4_Ordinal& OX4E3E1F2B)
{
    // default values
    OX61B1E0A1              = 0;
    OX6E7D9F4D        = 0;
    OX4E3E1F2B = 0;

    // check that we an stsc atom
    if (OX6F9E4B6D == NULL) return AP4_ERROR_INVALID_STATE;
    
    // get the chunk info from the stsc atom
    AP4_Ordinal OX7BC19F64 = 0;
    AP4_Result OX2B3E7F5E = OX6F9E4B6D->GetChunkForSample(OX2C9E3EFC+1, // the atom API is 1-based 
                                                      OX7BC19F64, 
                                                      OX6E7D9F4D, 
                                                      OX4E3E1F2B);
    if (AP4_FAILED(OX2B3E7F5E)) return OX2B3E7F5E;
    if (OX7BC19F64 == 0) return AP4_ERROR_INTERNAL;

    // the atom sample and chunk indexes are 1-based, so we need to translate
    OX61B1E0A1 = OX7BC19F64-1;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetChunkOffset
+---------------------------------------------------------------------*/
AP4_Result 
OX7B4DF339::OX3C2F7A1B(AP4_Ordinal   OX61B1E0A1, 
                                    AP4_Position& OX4E7C1A3D)
{
    if (OX3C0A4BBD) {
        AP4_UI32 OX6A2D7E4C;
        AP4_Result OX2B3E7F5E = OX3C0A4BBD->GetChunkOffset(OX61B1E0A1+1, OX6A2D7E4C);
        if (AP4_SUCCEEDED(OX2B3E7F5E)) {
            OX4E7C1A3D = OX6A2D7E4C;
        } else {
            OX4E7C1A3D = 0;
        }
        return OX2B3E7F5E;
    } else if (OX5A0C4E0D) {
        return OX5A0C4E0D->GetChunkOffset(OX61B1E0A1+1, OX4E7C1A3D);
    } else {
        OX4E7C1A3D = 0;
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::SetChunkOffset
+---------------------------------------------------------------------*/
AP4_Result 
OX7B4DF339::OX4D1E2C3A(AP4_Ordinal  OX61B1E0A1, 
                                    AP4_Position OX4E7C1A3D)
{
    if (OX3C0A4BBD) {
        if ((OX4E7C1A3D >> 32) != 0) return AP4_ERROR_OUT_OF_RANGE;
        return OX3C0A4BBD->SetChunkOffset(OX61B1E0A1+1, (AP4_UI32)OX4E7C1A3D);
    } else if (OX5A0C4E0D) {
        return OX5A0C4E0D->SetChunkOffset(OX61B1E0A1+1, OX4E7C1A3D);
    } else {
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::SetSampleSize
+---------------------------------------------------------------------*/
AP4_Result 
OX7B4DF339::OX3E7C4A1B(AP4_Ordinal OX2C9E3EFC, AP4_Size OX3C2E6A1B)
{
    if (OX6E9D7F4A) {
        return OX6E9D7F4A->SetSampleSize(OX2C9E3EFC+1, OX3C2E6A1B);
    } else if (OX2E1FB237) {
        return OX2E1FB237->SetSampleSize(OX2C9E3EFC+1, OX3C2E6A1B);
    } else {    
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleIndexForTimeStamp
+---------------------------------------------------------------------*/
AP4_Result 
OX7B4DF339::OX2F1D7A3E(AP4_UI64     OX3E1F7A4C, 
                                                AP4_Ordinal& OX2C9E3EFC)
{
    return OX199A1B34 ? OX199A1B34->GetSampleIndexForTimeStamp(OX3E1F7A4C, OX2C9E3EFC) 
                      : AP4_FAILURE;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetNearestSyncSampleIndex
+---------------------------------------------------------------------*/
AP4_Ordinal  
OX7B4DF339::OX1F7B6C2A(AP4_Ordinal OX2C9E3EFC, bool OX2A1E7C3F)
{
    // if we don't have an stss table, all samples match
    if (OX3DF1C2F9 == NULL) return OX2C9E3EFC;
    
    OX2C9E3EFC += 1; // the table is 1-based
    AP4_Cardinal OX2E9B7C5D = OX3DF1C2F9->GetEntries().ItemCount();
    if (OX2A1E7C3F) {
        AP4_Ordinal OX1C3A7F5E = 0;    
        for (unsigned int OX7E3F9C5D=0; OX7E3F9C5D<OX2E9B7C5D; OX7E3F9C5D++) {
            if (OX3DF1C2F9->GetEntries()[OX7E3F9C5D] >= OX2C9E3EFC) return OX1C3A7F5E;
            if (OX3DF1C2F9->GetEntries()[OX7E3F9C5D]) OX1C3A7F5E = OX3DF1C2F9->GetEntries()[OX7E3F9C5D]-1;
        }

        // not found?
        return OX1C3A7F5E;
    } else {
        for (unsigned int OX7E3F9C5D=0; OX7E3F9C5D<OX2E9B7C5D; OX7E3F9C5D++) {
            if (OX3DF1C2F9->GetEntries()[OX7E3F9C5D] >= OX2C9E3EFC) {
                return OX3DF1C2F9->GetEntries()[OX7E3F9C5D]?OX3DF1C2F9->GetEntries()[OX7E3F9C5D]-1:OX2C9E3EFC-1;
            }
        }

        // not found?
        return OX6A3E4D1B();
    }
}