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
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_AtomSampleTable)


/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::AP4_AtomSampleTable
+---------------------------------------------------------------------*/
AP4_AtomSampleTable::AP4_AtomSampleTable(AP4_ContainerAtom* stbl, 
                                         AP4_ByteStream&    sample_stream) :
    m_SampleStream(sample_stream)
{
    m_StscAtom = AP4_DYNAMIC_CAST(AP4_StscAtom, stbl->GetChild(AP4_ATOM_TYPE_STSC));
    m_StcoAtom = AP4_DYNAMIC_CAST(AP4_StcoAtom, stbl->GetChild(AP4_ATOM_TYPE_STCO));
    m_StszAtom = AP4_DYNAMIC_CAST(AP4_StszAtom, stbl->GetChild(AP4_ATOM_TYPE_STSZ));
    m_Stz2Atom = AP4_DYNAMIC_CAST(AP4_Stz2Atom, stbl->GetChild(AP4_ATOM_TYPE_STZ2));
    m_CttsAtom = AP4_DYNAMIC_CAST(AP4_CttsAtom, stbl->GetChild(AP4_ATOM_TYPE_CTTS));
    m_SttsAtom = AP4_DYNAMIC_CAST(AP4_SttsAtom, stbl->GetChild(AP4_ATOM_TYPE_STTS));
    m_StssAtom = AP4_DYNAMIC_CAST(AP4_StssAtom, stbl->GetChild(AP4_ATOM_TYPE_STSS));
    m_StsdAtom = AP4_DYNAMIC_CAST(AP4_StsdAtom, stbl->GetChild(AP4_ATOM_TYPE_STSD));
    m_Co64Atom = AP4_DYNAMIC_CAST(AP4_Co64Atom, stbl->GetChild(AP4_ATOM_TYPE_CO64));

    // keep a reference to the sample stream
    m_SampleStream.AddReference();
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::~AP4_AtomSampleTable
+---------------------------------------------------------------------*/
AP4_AtomSampleTable::~AP4_AtomSampleTable()
{
    m_SampleStream.Release();
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_AtomSampleTable::GetSample(AP4_Ordinal index, 
                               AP4_Sample& sample)
{
    return GetSampleRecursive(index, sample, 0);
}

AP4_Result
AP4_AtomSampleTable::GetSampleRecursive(AP4_Ordinal index, 
                               AP4_Sample& sample, 
                               unsigned int i)
{
    if (i == 0) {
        AP4_Result result;
        if (!m_StscAtom) {
            return AP4_ERROR_INVALID_FORMAT;
        }
        if (m_StcoAtom == NULL && m_Co64Atom == NULL) {
            return AP4_ERROR_INVALID_FORMAT;
        }
        index++;
        AP4_Ordinal chunk, skip, desc;
        result = m_StscAtom->GetChunkForSample(index, chunk, skip, desc);
        if (AP4_FAILED(result)) return result;
        if (skip > index) return AP4_ERROR_INTERNAL;
        AP4_UI64 offset;
        if (m_StcoAtom) {
            AP4_UI32 offset_32;
            result = m_StcoAtom->GetChunkOffset(chunk, offset_32);
            offset = offset_32;
        } else {
            result = m_Co64Atom->GetChunkOffset(chunk, offset);
        }
        if (AP4_FAILED(result)) return result;
        return GetSampleRecursive(index, sample, i + index - skip);
    } else {
        AP4_Size size = 0;
        AP4_Result result;
        if (m_StszAtom) {
            result = m_StszAtom->GetSampleSize(i, size); 
        } else if (m_Stz2Atom) {
            result = m_Stz2Atom->GetSampleSize(i, size); 
        } else {
            result = AP4_ERROR_INVALID_FORMAT;
        }
        if (AP4_FAILED(result)) return result;
        return GetSampleRecursive(index, sample, i + 1);
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleCount
+---------------------------------------------------------------------*/
AP4_Cardinal
AP4_AtomSampleTable::GetSampleCount()
{
    return GetSampleCountRecursive();
}

AP4_Cardinal
AP4_AtomSampleTable::GetSampleCountRecursive()
{
    if (m_StszAtom) {
        return m_StszAtom->GetSampleCount();
    } else if (m_Stz2Atom) {
        return m_Stz2Atom->GetSampleCount();
    } else {
        return 0;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription*
AP4_AtomSampleTable::GetSampleDescription(AP4_Ordinal index)
{
    if (m_StsdAtom) {
        switch (m_StsdAtom->GetSampleDescription(index) != NULL) {
            case 1:
                return m_StsdAtom->GetSampleDescription(index);
            default:
                return NULL;
        }
    } else {
        return NULL;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleDescriptionCount
+---------------------------------------------------------------------*/
AP4_Cardinal
AP4_AtomSampleTable::GetSampleDescriptionCount()
{
    return m_StsdAtom ? m_StsdAtom->GetSampleDescriptionCount() : 0;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleChunkPosition
+---------------------------------------------------------------------*/
AP4_Result   
AP4_AtomSampleTable::GetSampleChunkPosition(AP4_Ordinal  sample_index, 
                                            AP4_Ordinal& chunk_index,
                                            AP4_Ordinal& position_in_chunk)
{
    return GetSampleChunkPositionRecursive(sample_index, chunk_index, position_in_chunk, 0);
}

AP4_Result   
AP4_AtomSampleTable::GetSampleChunkPositionRecursive(AP4_Ordinal  sample_index, 
                                            AP4_Ordinal& chunk_index,
                                            AP4_Ordinal& position_in_chunk,
                                            AP4_Ordinal sample_description_index)
{
    chunk_index       = 0;
    position_in_chunk = 0;
    sample_description_index = 0;
    return GetChunkForSampleRecursive(sample_index, chunk_index, position_in_chunk, sample_description_index, 0);
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetChunkForSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_AtomSampleTable::GetChunkForSample(AP4_Ordinal  sample_index,
                                       AP4_Ordinal& chunk_index,
                                       AP4_Ordinal& position_in_chunk,
                                       AP4_Ordinal& sample_description_index)
{
    return GetChunkForSampleRecursive(sample_index, chunk_index, position_in_chunk, sample_description_index, 0);
}

AP4_Result 
AP4_AtomSampleTable::GetChunkForSampleRecursive(AP4_Ordinal  sample_index,
                                       AP4_Ordinal& chunk_index,
                                       AP4_Ordinal& position_in_chunk,
                                       AP4_Ordinal& sample_description_index,
                                       AP4_Ordinal k)
{
    if (k == 0) {
        if (m_StscAtom == NULL) return AP4_ERROR_INVALID_STATE;
        AP4_Ordinal chunk = 0;
        AP4_Result result = m_StscAtom->GetChunkForSample(sample_index+1, // the atom API is 1-based 
                                                          chunk, 
                                                          position_in_chunk, 
                                                          sample_description_index);
        if (AP4_FAILED(result)) return result;
        if (chunk == 0) return AP4_ERROR_INTERNAL;
        chunk_index = chunk-1;
        return AP4_SUCCESS;
    }
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetChunkOffset
+---------------------------------------------------------------------*/
AP4_Result 
AP4_AtomSampleTable::GetChunkOffset(AP4_Ordinal   chunk_index, 
                                    AP4_Position& offset)
{
    return GetChunkOffsetRecursive(chunk_index, offset);
}

AP4_Result 
AP4_AtomSampleTable::GetChunkOffsetRecursive(AP4_Ordinal   chunk_index, 
                                    AP4_Position& offset)
{
    if (m_StcoAtom) {
        AP4_UI32 offset_32;
        AP4_Result result = m_StcoAtom->GetChunkOffset(chunk_index+1, offset_32);
        if (AP4_SUCCEEDED(result)) {
            offset = offset_32;
        } else {
            offset = 0;
        }
        return result;
    } else if (m_Co64Atom) {
        return m_Co64Atom->GetChunkOffset(chunk_index+1, offset);
    } else {
        offset = 0;
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::SetChunkOffset
+---------------------------------------------------------------------*/
AP4_Result 
AP4_AtomSampleTable::SetChunkOffset(AP4_Ordinal  chunk_index, 
                                    AP4_Position offset)
{
    return SetChunkOffsetRecursive(chunk_index, offset);
}

AP4_Result 
AP4_AtomSampleTable::SetChunkOffsetRecursive(AP4_Ordinal  chunk_index, 
                                    AP4_Position offset)
{
    if (m_StcoAtom) {
        if ((offset >> 32) != 0) return AP4_ERROR_OUT_OF_RANGE;
        return m_StcoAtom->SetChunkOffset(chunk_index+1, (AP4_UI32)offset);
    } else if (m_Co64Atom) {
        return m_Co64Atom->SetChunkOffset(chunk_index+1, offset);
    } else {
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::SetSampleSize
+---------------------------------------------------------------------*/
AP4_Result 
AP4_AtomSampleTable::SetSampleSize(AP4_Ordinal sample_index, AP4_Size size)
{
    return SetSampleSizeRecursive(sample_index, size);
}

AP4_Result 
AP4_AtomSampleTable::SetSampleSizeRecursive(AP4_Ordinal sample_index, AP4_Size size)
{
    if (m_StszAtom) {
        return m_StszAtom->SetSampleSize(sample_index+1, size);
    } else if (m_Stz2Atom) {
        return m_Stz2Atom->SetSampleSize(sample_index+1, size);
    } else {    
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleIndexForTimeStamp
+---------------------------------------------------------------------*/
AP4_Result 
AP4_AtomSampleTable::GetSampleIndexForTimeStamp(AP4_UI64     ts, 
                                                AP4_Ordinal& sample_index)
{
    return m_SttsAtom ? m_SttsAtom->GetSampleIndexForTimeStamp(ts, sample_index) 
                      : AP4_FAILURE;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetNearestSyncSampleIndex
+---------------------------------------------------------------------*/
AP4_Ordinal  
AP4_AtomSampleTable::GetNearestSyncSampleIndex(AP4_Ordinal sample_index, bool before)
{
    return GetNearestSyncSampleIndexRecursive(sample_index, before, 0);
}

AP4_Ordinal  
AP4_AtomSampleTable::GetNearestSyncSampleIndexRecursive(AP4_Ordinal sample_index, bool before, unsigned int i)
{
    if (i == 0) {
        if (m_StssAtom == NULL) return sample_index;
        sample_index += 1; 
        AP4_Cardinal entry_count = m_StssAtom->GetEntries().ItemCount();
        return GetNearestSyncSampleIndexRecursive(sample_index, before, i + 1);
    } else {
        if (before) {
            AP4_Ordinal cursor = 0;    
            for (unsigned int j=0; j<m_StssAtom->GetEntries().ItemCount(); j++) {
                if (m_StssAtom->GetEntries()[j] >= sample_index) return cursor;
                if (m_StssAtom->GetEntries()[j]) cursor = m_StssAtom->GetEntries()[j]-1;
            }
            return cursor;
        } else {
            for (unsigned int j=0; j<m_StssAtom->GetEntries().ItemCount(); j++) {
                if (m_StssAtom->GetEntries()[j] >= sample_index) {
                    return m_StssAtom->GetEntries()[j]?m_StssAtom->GetEntries()[j]-1:sample_index-1;
                }
            }
            return GetSampleCount();
        }
    }
}