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

#define GET_ATOM(atom_type, atom_name) \
    (atom_name = AP4_DYNAMIC_CAST(atom_type, stbl->GetChild(AP4_ATOM_TYPE_##atom_name)))

AP4_AtomSampleTable::AP4_AtomSampleTable(AP4_ContainerAtom* stbl, 
                                         AP4_ByteStream&    sample_stream) :
    m_SampleStream(sample_stream)
{
    GET_ATOM(AP4_StscAtom, m_StscAtom);
    GET_ATOM(AP4_StcoAtom, m_StcoAtom);
    GET_ATOM(AP4_StszAtom, m_StszAtom);
    GET_ATOM(AP4_Stz2Atom, m_Stz2Atom);
    GET_ATOM(AP4_CttsAtom, m_CttsAtom);
    GET_ATOM(AP4_SttsAtom, m_SttsAtom);
    GET_ATOM(AP4_StssAtom, m_StssAtom);
    GET_ATOM(AP4_StsdAtom, m_StsdAtom);
    GET_ATOM(AP4_Co64Atom, m_Co64Atom);

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
    if (!m_StscAtom || (m_StcoAtom == NULL && m_Co64Atom == NULL)) return AP4_ERROR_INVALID_FORMAT;

    index++;
    AP4_Ordinal chunk, skip, desc;
    if (AP4_FAILED(m_StscAtom->GetChunkForSample(index, chunk, skip, desc))) return AP4_ERROR_INVALID_FORMAT;
    if (skip > index) return AP4_ERROR_INTERNAL;

    AP4_UI64 offset;
    AP4_Result result = m_StcoAtom ? m_StcoAtom->GetChunkOffset(chunk, (AP4_UI32&)offset) : m_Co64Atom->GetChunkOffset(chunk, offset);
    if (AP4_FAILED(result)) return result;

    for (unsigned int i = index-skip; i < index; i++) {
        AP4_Size size = 0;
        result = m_StszAtom ? m_StszAtom->GetSampleSize(i, size) : m_Stz2Atom ? m_Stz2Atom->GetSampleSize(i, size) : AP4_ERROR_INVALID_FORMAT;
        if (AP4_FAILED(result)) return result;
        offset += size;
    }

    sample.SetDescriptionIndex(desc-1);

    AP4_UI32 cts_offset = 0;
    AP4_UI64 dts = 0;
    AP4_UI32 duration = 0;
    if (AP4_FAILED(m_SttsAtom->GetDts(index, dts, &duration))) return AP4_ERROR_INVALID_FORMAT;
    sample.SetDuration(duration);
    sample.SetDts(dts);
    if (!m_CttsAtom) {
        sample.SetCts(dts);
    } else {
        if (AP4_FAILED(m_CttsAtom->GetCtsOffset(index, cts_offset))) return AP4_ERROR_INVALID_FORMAT;
        sample.SetCtsDelta(cts_offset);
    }     

    AP4_Size sample_size = 0;
    result = m_StszAtom ? m_StszAtom->GetSampleSize(index, sample_size) : m_Stz2Atom ? m_Stz2Atom->GetSampleSize(index, sample_size) : AP4_ERROR_INVALID_FORMAT;
    if (AP4_FAILED(result)) return result;
    sample.SetSize(sample_size);

    sample.SetSync(!m_StssAtom || m_StssAtom->IsSampleSync(index));
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
    chunk_index = 0;
    position_in_chunk = 0;
    
    AP4_Ordinal sample_description_index;
    return GetChunkForSample(sample_index, chunk_index, position_in_chunk, sample_description_index);
}

AP4_Result 
AP4_AtomSampleTable::GetChunkForSample(AP4_Ordinal  sample_index,
                                       AP4_Ordinal& chunk_index,
                                       AP4_Ordinal& position_in_chunk,
                                       AP4_Ordinal& sample_description_index)
{
    if (!m_StscAtom) return AP4_ERROR_INVALID_STATE;
    
    AP4_Ordinal chunk = 0;
    AP4_Result result = m_StscAtom->GetChunkForSample(sample_index+1, chunk, position_in_chunk, sample_description_index);
    if (AP4_FAILED(result) || chunk == 0) return AP4_ERROR_INTERNAL;

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
        offset = AP4_SUCCEEDED(result) ? offset_32 : 0;
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
        return (offset >> 32) ? AP4_ERROR_OUT_OF_RANGE : m_StcoAtom->SetChunkOffset(chunk_index+1, (AP4_UI32)offset);
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
    return m_SttsAtom ? m_SttsAtom->GetSampleIndexForTimeStamp(ts, sample_index) : AP4_FAILURE;
}

AP4_Ordinal  
AP4_AtomSampleTable::GetNearestSyncSampleIndex(AP4_Ordinal sample_index, bool before)
{
    if (!m_StssAtom) return sample_index;
    
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
                return m_StssAtom->GetEntries()[i] ? m_StssAtom->GetEntries()[i]-1 : sample_index-1;
            }
        }
        return GetSampleCount();
    }
}