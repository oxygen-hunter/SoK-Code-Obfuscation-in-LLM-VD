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
    int state = 0;
    while (state != 9) {
        switch (state) {
            case 0:
                m_StscAtom = AP4_DYNAMIC_CAST(AP4_StscAtom, stbl->GetChild(AP4_ATOM_TYPE_STSC));
                state = 1;
                break;
            case 1:
                m_StcoAtom = AP4_DYNAMIC_CAST(AP4_StcoAtom, stbl->GetChild(AP4_ATOM_TYPE_STCO));
                state = 2;
                break;
            case 2:
                m_StszAtom = AP4_DYNAMIC_CAST(AP4_StszAtom, stbl->GetChild(AP4_ATOM_TYPE_STSZ));
                state = 3;
                break;
            case 3:
                m_Stz2Atom = AP4_DYNAMIC_CAST(AP4_Stz2Atom, stbl->GetChild(AP4_ATOM_TYPE_STZ2));
                state = 4;
                break;
            case 4:
                m_CttsAtom = AP4_DYNAMIC_CAST(AP4_CttsAtom, stbl->GetChild(AP4_ATOM_TYPE_CTTS));
                state = 5;
                break;
            case 5:
                m_SttsAtom = AP4_DYNAMIC_CAST(AP4_SttsAtom, stbl->GetChild(AP4_ATOM_TYPE_STTS));
                state = 6;
                break;
            case 6:
                m_StssAtom = AP4_DYNAMIC_CAST(AP4_StssAtom, stbl->GetChild(AP4_ATOM_TYPE_STSS));
                state = 7;
                break;
            case 7:
                m_StsdAtom = AP4_DYNAMIC_CAST(AP4_StsdAtom, stbl->GetChild(AP4_ATOM_TYPE_STSD));
                state = 8;
                break;
            case 8:
                m_Co64Atom = AP4_DYNAMIC_CAST(AP4_Co64Atom, stbl->GetChild(AP4_ATOM_TYPE_CO64));
                state = 9;
                break;
            default:
                break;
        }
    }
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
    AP4_Result result;

    int state = 0;
    AP4_Ordinal chunk = 0, skip = 0, desc = 0;
    AP4_UI64 offset = 0;
    AP4_UI32 offset_32 = 0;
    AP4_Size sample_size = 0;
    AP4_UI32 cts_offset = 0;
    AP4_UI64 dts = 0;
    AP4_UI32 duration = 0;
    AP4_Size size = 0;

    while (state != 11) {
        switch (state) {
            case 0:
                if (!m_StscAtom) return AP4_ERROR_INVALID_FORMAT;
                state = 1;
                break;
            case 1:
                if (m_StcoAtom == NULL && m_Co64Atom == NULL) return AP4_ERROR_INVALID_FORMAT;
                state = 2;
                break;
            case 2:
                index++;
                state = 3;
                break;
            case 3:
                result = m_StscAtom->GetChunkForSample(index, chunk, skip, desc);
                if (AP4_FAILED(result)) return result;
                state = 4;
                break;
            case 4:
                if (skip > index) return AP4_ERROR_INTERNAL;
                state = 5;
                break;
            case 5:
                if (m_StcoAtom) {
                    result = m_StcoAtom->GetChunkOffset(chunk, offset_32);
                    offset = offset_32;
                } else {
                    result = m_Co64Atom->GetChunkOffset(chunk, offset);
                }
                if (AP4_FAILED(result)) return result;
                state = 6;
                break;
            case 6:
                for (unsigned int i = index-skip; i < index; i++) {
                    if (m_StszAtom) {
                        result = m_StszAtom->GetSampleSize(i, size); 
                    } else if (m_Stz2Atom) {
                        result = m_Stz2Atom->GetSampleSize(i, size); 
                    } else {
                        result = AP4_ERROR_INVALID_FORMAT;
                    }
                    if (AP4_FAILED(result)) return result;
                    offset += size;
                }
                state = 7;
                break;
            case 7:
                sample.SetDescriptionIndex(desc-1);
                state = 8;
                break;
            case 8:
                result = m_SttsAtom->GetDts(index, dts, &duration);
                if (AP4_FAILED(result)) return result;
                sample.SetDuration(duration);
                sample.SetDts(dts);
                if (m_CttsAtom == NULL) {
                    sample.SetCts(dts);
                } else {
                    result = m_CttsAtom->GetCtsOffset(index, cts_offset); 
                    if (AP4_FAILED(result)) return result;
                    sample.SetCtsDelta(cts_offset);
                }
                state = 9;
                break;
            case 9:
                if (m_StszAtom) {
                    result = m_StszAtom->GetSampleSize(index, sample_size); 
                } else if (m_Stz2Atom) {
                    result = m_Stz2Atom->GetSampleSize(index, sample_size); 
                } else {
                    result = AP4_ERROR_INVALID_FORMAT;
                }
                if (AP4_FAILED(result)) return result;
                sample.SetSize(sample_size);
                state = 10;
                break;
            case 10:
                if (m_StssAtom == NULL) {
                    sample.SetSync(true);
                } else {
                    sample.SetSync(m_StssAtom->IsSampleSync(index));
                }
                sample.SetOffset(offset);
                sample.SetDataStream(m_SampleStream);
                state = 11;
                break;
            default:
                break;
        }
    }
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleCount
+---------------------------------------------------------------------*/
AP4_Cardinal
AP4_AtomSampleTable::GetSampleCount()
{
    int state = 0;
    AP4_Cardinal result = 0;
    while (state != 3) {
        switch (state) {
            case 0:
                if (m_StszAtom) {
                    result = m_StszAtom->GetSampleCount();
                    state = 3;
                } else {
                    state = 1;
                }
                break;
            case 1:
                if (m_Stz2Atom) {
                    result = m_Stz2Atom->GetSampleCount();
                    state = 3;
                } else {
                    state = 2;
                }
                break;
            case 2:
                result = 0;
                state = 3;
                break;
            default:
                break;
        }
    }
    return result;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription*
AP4_AtomSampleTable::GetSampleDescription(AP4_Ordinal index)
{
    return m_StsdAtom ? m_StsdAtom->GetSampleDescription(index) : NULL;
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
    chunk_index       = 0;
    position_in_chunk = 0;
    
    AP4_Ordinal sample_description_index;
    return GetChunkForSample(sample_index, 
                             chunk_index, 
                             position_in_chunk, 
                             sample_description_index);
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
    int state = 0;
    AP4_Ordinal chunk = 0;

    while (state != 3) {
        switch (state) {
            case 0:
                if (m_StscAtom == NULL) return AP4_ERROR_INVALID_STATE;
                state = 1;
                break;
            case 1:
                chunk_index = 0;
                position_in_chunk = 0;
                sample_description_index = 0;
                state = 2;
                break;
            case 2:
                AP4_Result result = m_StscAtom->GetChunkForSample(sample_index+1, chunk, position_in_chunk, sample_description_index);
                if (AP4_FAILED(result)) return result;
                if (chunk == 0) return AP4_ERROR_INTERNAL;
                chunk_index = chunk - 1;
                state = 3;
                break;
            default:
                break;
        }
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
    int state = 0;
    AP4_Result result;
    AP4_UI32 offset_32 = 0;
    while (state != 4) {
        switch (state) {
            case 0:
                if (m_StcoAtom) {
                    result = m_StcoAtom->GetChunkOffset(chunk_index+1, offset_32);
                    if (AP4_SUCCEEDED(result)) {
                        offset = offset_32;
                    } else {
                        offset = 0;
                    }
                    return result;
                } else {
                    state = 1;
                }
                break;
            case 1:
                if (m_Co64Atom) {
                    return m_Co64Atom->GetChunkOffset(chunk_index+1, offset);
                } else {
                    state = 2;
                }
                break;
            case 2:
                offset = 0;
                state = 3;
                break;
            case 3:
                return AP4_FAILURE;
            default:
                break;
        }
    }
    return AP4_FAILURE;
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::SetChunkOffset
+---------------------------------------------------------------------*/
AP4_Result 
AP4_AtomSampleTable::SetChunkOffset(AP4_Ordinal  chunk_index, 
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
    if (m_StssAtom == NULL) return sample_index;
    
    sample_index += 1;
    AP4_Cardinal entry_count = m_StssAtom->GetEntries().ItemCount();
    AP4_Ordinal cursor = 0;    
    int state = 0;
    while (state != 5) {
        switch (state) {
            case 0:
                if (before) {
                    for (unsigned int i=0; i<entry_count; i++) {
                        if (m_StssAtom->GetEntries()[i] >= sample_index) return cursor;
                        if (m_StssAtom->GetEntries()[i]) cursor = m_StssAtom->GetEntries()[i]-1;
                    }
                    state = 1;
                } else {
                    state = 2;
                }
                break;
            case 1:
                return cursor;
            case 2:
                for (unsigned int i=0; i<entry_count; i++) {
                    if (m_StssAtom->GetEntries()[i] >= sample_index) {
                        return m_StssAtom->GetEntries()[i]?m_StssAtom->GetEntries()[i]-1:sample_index-1;
                    }
                }
                state = 3;
                break;
            case 3:
                return GetSampleCount();
            default:
                break;
        }
    }
    return sample_index;
}