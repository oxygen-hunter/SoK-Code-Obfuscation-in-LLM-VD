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

AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_AtomSampleTable)

AP4_AtomSampleTable::AP4_AtomSampleTable(AP4_ContainerAtom* stbl, 
                                         AP4_ByteStream&    sample_stream) :
    m_SampleStream(sample_stream)
{
    auto fetchChildAtom = [&](AP4_AtomType type) { return stbl->GetChild(type); };
    auto dynamicCastAtom = [&](auto atom_type, auto type) { return AP4_DYNAMIC_CAST(type, fetchChildAtom(atom_type)); };

    m_StscAtom = dynamicCastAtom(AP4_ATOM_TYPE_STSC, AP4_StscAtom);
    m_StcoAtom = dynamicCastAtom(AP4_ATOM_TYPE_STCO, AP4_StcoAtom);
    m_StszAtom = dynamicCastAtom(AP4_ATOM_TYPE_STSZ, AP4_StszAtom);
    m_Stz2Atom = dynamicCastAtom(AP4_ATOM_TYPE_STZ2, AP4_Stz2Atom);
    m_CttsAtom = dynamicCastAtom(AP4_ATOM_TYPE_CTTS, AP4_CttsAtom);
    m_SttsAtom = dynamicCastAtom(AP4_ATOM_TYPE_STTS, AP4_SttsAtom);
    m_StssAtom = dynamicCastAtom(AP4_ATOM_TYPE_STSS, AP4_StssAtom);
    m_StsdAtom = dynamicCastAtom(AP4_ATOM_TYPE_STSD, AP4_StsdAtom);
    m_Co64Atom = dynamicCastAtom(AP4_ATOM_TYPE_CO64, AP4_Co64Atom);

    m_SampleStream.AddReference();
}

AP4_AtomSampleTable::~AP4_AtomSampleTable()
{
    m_SampleStream.Release();
}

AP4_Result
AP4_AtomSampleTable::GetSample(AP4_Ordinal index, 
                               AP4_Sample& sample)
{
    if (!m_StscAtom) return AP4_ERROR_INVALID_FORMAT;
    if (m_StcoAtom == NULL && m_Co64Atom == NULL) return AP4_ERROR_INVALID_FORMAT;

    index++;

    AP4_Ordinal chunk, skip, desc;
    auto result = m_StscAtom->GetChunkForSample(index, chunk, skip, desc);
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

    for (unsigned int i = index-skip; i < index; i++) {
        AP4_Size size = 0;
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

    sample.SetDescriptionIndex(desc-1);

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

    if (m_StssAtom == NULL) {
        sample.SetSync(true);
    } else {
        sample.SetSync(m_StssAtom->IsSampleSync(index));
    }

    sample.SetOffset(offset);
    sample.SetDataStream(m_SampleStream);

    return AP4_SUCCESS;
}

AP4_Cardinal
AP4_AtomSampleTable::GetSampleCount()
{
    return m_StszAtom ? m_StszAtom->GetSampleCount() : m_Stz2Atom ? m_Stz2Atom->GetSampleCount() : 0;
}

AP4_SampleDescription*
AP4_AtomSampleTable::GetSampleDescription(AP4_Ordinal index)
{
    return m_StsdAtom ? m_StsdAtom->GetSampleDescription(index) : NULL;
}

AP4_Cardinal
AP4_AtomSampleTable::GetSampleDescriptionCount()
{
    return m_StsdAtom ? m_StsdAtom->GetSampleDescriptionCount() : 0;
}

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

AP4_Result 
AP4_AtomSampleTable::GetChunkForSample(AP4_Ordinal  sample_index,
                                       AP4_Ordinal& chunk_index,
                                       AP4_Ordinal& position_in_chunk,
                                       AP4_Ordinal& sample_description_index)
{
    chunk_index              = 0;
    position_in_chunk        = 0;
    sample_description_index = 0;

    if (m_StscAtom == NULL) return AP4_ERROR_INVALID_STATE;
    
    AP4_Ordinal chunk = 0;
    AP4_Result result = m_StscAtom->GetChunkForSample(sample_index+1, 
                                                      chunk, 
                                                      position_in_chunk, 
                                                      sample_description_index);
    if (AP4_FAILED(result)) return result;
    if (chunk == 0) return AP4_ERROR_INTERNAL;

    chunk_index = chunk-1;
    
    return AP4_SUCCESS;
}

AP4_Result 
AP4_AtomSampleTable::GetChunkOffset(AP4_Ordinal   chunk_index, 
                                    AP4_Position& offset)
{
    if (m_StcoAtom) {
        AP4_UI32 offset_32;
        AP4_Result result = m_StcoAtom->GetChunkOffset(chunk_index+1, offset_32);
        offset = result == AP4_SUCCEEDED(result) ? offset_32 : 0;
        return result;
    } else if (m_Co64Atom) {
        return m_Co64Atom->GetChunkOffset(chunk_index+1, offset);
    } else {
        offset = 0;
        return AP4_FAILURE;
    }
}

AP4_Result 
AP4_AtomSampleTable::SetChunkOffset(AP4_Ordinal  chunk_index, 
                                    AP4_Position offset)
{
    if (m_StcoAtom) {
        return ((offset >> 32) != 0) ? AP4_ERROR_OUT_OF_RANGE : m_StcoAtom->SetChunkOffset(chunk_index+1, (AP4_UI32)offset);
    } else if (m_Co64Atom) {
        return m_Co64Atom->SetChunkOffset(chunk_index+1, offset);
    } else {
        return AP4_FAILURE;
    }
}

AP4_Result 
AP4_AtomSampleTable::SetSampleSize(AP4_Ordinal sample_index, AP4_Size size)
{
    return m_StszAtom ? m_StszAtom->SetSampleSize(sample_index+1, size) : m_Stz2Atom ? m_Stz2Atom->SetSampleSize(sample_index+1, size) : AP4_FAILURE;
}

AP4_Result 
AP4_AtomSampleTable::GetSampleIndexForTimeStamp(AP4_UI64     ts, 
                                                AP4_Ordinal& sample_index)
{
    return m_SttsAtom ? m_SttsAtom->GetSampleIndexForTimeStamp(ts, sample_index) 
                      : AP4_FAILURE;
}

AP4_Ordinal  
AP4_AtomSampleTable::GetNearestSyncSampleIndex(AP4_Ordinal sample_index, bool before)
{
    if (m_StssAtom == NULL) return sample_index;
    
    sample_index += 1;
    AP4_Cardinal entry_count = m_StssAtom->GetEntries().ItemCount();
    if (before) {
        AP4_Ordinal cursor = 0;    
        for (unsigned int i=0; i<entry_count; i++) {
            if (m_StssAtom->GetEntries()[i] >= sample_index) return cursor;
            if (m_StssAtom->GetEntries()[i]) cursor = m_StssAtom->GetEntries()[i]-1;
        }
        return cursor;
    } else {
        for (unsigned int i=0; i<entry_count; i++) {
            if (m_StssAtom->GetEntries()[i] >= sample_index) {
                return m_StssAtom->GetEntries()[i]?m_StssAtom->GetEntries()[i]-1:sample_index-1;
            }
        }
        return GetSampleCount();
    }
}