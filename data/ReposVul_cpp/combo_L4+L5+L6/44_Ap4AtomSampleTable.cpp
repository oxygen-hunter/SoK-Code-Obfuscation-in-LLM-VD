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

AP4_Result SampleLoop(AP4_Ordinal index, AP4_UI64 offset, AP4_Result result, AP4_UI64& new_offset) {
    if (index <= 0) {
        new_offset = offset;
        return result;
    }
    
    AP4_Size size = 0;
    if (m_StszAtom) {
        result = m_StszAtom->GetSampleSize(index, size); 
    } else if (m_Stz2Atom) {
        result = m_Stz2Atom->GetSampleSize(index, size); 
    } else {
        result = AP4_ERROR_INVALID_FORMAT;
    }
    if (AP4_FAILED(result)) return result;
    SampleLoop(index-1, offset + size, result, new_offset);
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetSample
+---------------------------------------------------------------------*/
AP4_Result
AP4_AtomSampleTable::GetSample(AP4_Ordinal index, 
                               AP4_Sample& sample)
{
    AP4_Result result;

    // check that we have an stsc atom
    if (!m_StscAtom) {
        return AP4_ERROR_INVALID_FORMAT;
    }
    
    // check that we have a chunk offset table
    if (m_StcoAtom == NULL && m_Co64Atom == NULL) {
        return AP4_ERROR_INVALID_FORMAT;
    }

    // MP4 uses 1-based indexes internally, so adjust by one
    index++;

    // find out in which chunk this sample is located
    AP4_Ordinal chunk, skip, desc;
    result = m_StscAtom->GetChunkForSample(index, chunk, skip, desc);
    if (AP4_FAILED(result)) return result;
    
    // check that the result is within bounds
    if (skip > index) return AP4_ERROR_INTERNAL;

    // get the atom offset for this chunk
    AP4_UI64 offset;
    if (m_StcoAtom) {
        AP4_UI32 offset_32;
        result = m_StcoAtom->GetChunkOffset(chunk, offset_32);
        offset = offset_32;
    } else {
        result = m_Co64Atom->GetChunkOffset(chunk, offset);
    }
    if (AP4_FAILED(result)) return result;
    
    // compute the additional offset inside the chunk
    AP4_UI64 new_offset;
    result = SampleLoop(index-skip, offset, result, new_offset);
    if (AP4_FAILED(result)) return result;
    offset = new_offset;

    // set the description index
    sample.SetDescriptionIndex(desc-1); // adjust for 0-based indexes

    // set the dts and cts
    AP4_UI32 cts_offset = 0;
    AP4_UI64 dts        = 0;
    AP4_UI32 duration   = 0;
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

    // set the size
    AP4_Size sample_size = 0;
    if (m_StszAtom) {
        result = m_StszAtom->GetSampleSize(index, sample_size); 
    } else if (m_Stz2Atom) {
        result = m_Stz2Atom->GetSampleSize(index, sample_size); 
    } else {
        result = AP4_ERROR_INVALID_FORMAT;
    }
    if (AP4_FAILED(result)) return result;
    sample.SetSize(sample_size);

    // set the sync flag
    sample.SetSync(m_StssAtom ? m_StssAtom->IsSampleSync(index) : true);

    // set the offset
    sample.SetOffset(offset);

    // set the data stream
    sample.SetDataStream(m_SampleStream);

    return AP4_SUCCESS;
}

AP4_Result SyncSampleLoop(AP4_Ordinal cursor, AP4_Ordinal sample_index, bool before, AP4_Cardinal entry_count, AP4_Ordinal& result_cursor) {
    if (entry_count <= 0) {
        result_cursor = cursor;
        return AP4_SUCCESS;
    }
    
    if (before) {
        if (m_StssAtom->GetEntries()[entry_count-1] >= sample_index) {
            result_cursor = cursor;
            return AP4_SUCCESS;
        }
        if (m_StssAtom->GetEntries()[entry_count-1]) cursor = m_StssAtom->GetEntries()[entry_count-1]-1;
    } else {
        if (m_StssAtom->GetEntries()[entry_count-1] >= sample_index) {
            result_cursor = m_StssAtom->GetEntries()[entry_count-1] ? m_StssAtom->GetEntries()[entry_count-1]-1 : sample_index-1;
            return AP4_SUCCESS;
        }
    }
    
    return SyncSampleLoop(cursor, sample_index, before, entry_count-1, result_cursor);
}

/*----------------------------------------------------------------------
|   AP4_AtomSampleTable::GetNearestSyncSampleIndex
+---------------------------------------------------------------------*/
AP4_Ordinal  
AP4_AtomSampleTable::GetNearestSyncSampleIndex(AP4_Ordinal sample_index, bool before)
{
    // if we don't have an stss table, all samples match
    if (m_StssAtom == NULL) return sample_index;
    
    sample_index += 1; // the table is 1-based
    AP4_Cardinal entry_count = m_StssAtom->GetEntries().ItemCount();
    AP4_Ordinal result_cursor;
    SyncSampleLoop(0, sample_index, before, entry_count, result_cursor);
    return before ? result_cursor : GetSampleCount();
}