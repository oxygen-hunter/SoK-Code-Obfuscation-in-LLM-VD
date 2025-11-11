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
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                m_StscAtom = AP4_DYNAMIC_CAST(AP4_StscAtom, stbl->GetChild(AP4_ATOM_TYPE_STSC));
                __state = 1;
                break;
            case 1:
                m_StcoAtom = AP4_DYNAMIC_CAST(AP4_StcoAtom, stbl->GetChild(AP4_ATOM_TYPE_STCO));
                __state = 2;
                break;
            case 2:
                m_StszAtom = AP4_DYNAMIC_CAST(AP4_StszAtom, stbl->GetChild(AP4_ATOM_TYPE_STSZ));
                __state = 3;
                break;
            case 3:
                m_Stz2Atom = AP4_DYNAMIC_CAST(AP4_Stz2Atom, stbl->GetChild(AP4_ATOM_TYPE_STZ2));
                __state = 4;
                break;
            case 4:
                m_CttsAtom = AP4_DYNAMIC_CAST(AP4_CttsAtom, stbl->GetChild(AP4_ATOM_TYPE_CTTS));
                __state = 5;
                break;
            case 5:
                m_SttsAtom = AP4_DYNAMIC_CAST(AP4_SttsAtom, stbl->GetChild(AP4_ATOM_TYPE_STTS));
                __state = 6;
                break;
            case 6:
                m_StssAtom = AP4_DYNAMIC_CAST(AP4_StssAtom, stbl->GetChild(AP4_ATOM_TYPE_STSS));
                __state = 7;
                break;
            case 7:
                m_StsdAtom = AP4_DYNAMIC_CAST(AP4_StsdAtom, stbl->GetChild(AP4_ATOM_TYPE_STSD));
                __state = 8;
                break;
            case 8:
                m_Co64Atom = AP4_DYNAMIC_CAST(AP4_Co64Atom, stbl->GetChild(AP4_ATOM_TYPE_CO64));
                __state = 9;
                break;
            case 9:
                m_SampleStream.AddReference();
                __state = 10;
                break;
            case 10:
                return;
        }
    }
}

AP4_AtomSampleTable::~AP4_AtomSampleTable()
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                m_SampleStream.Release();
                __state = 1;
                break;
            case 1:
                return;
        }
    }
}

AP4_Result
AP4_AtomSampleTable::GetSample(AP4_Ordinal index, 
                               AP4_Sample& sample)
{
    AP4_Result result;
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                if (!m_StscAtom) {
                    return AP4_ERROR_INVALID_FORMAT;
                }
                __state = 1;
                break;
            case 1:
                if (m_StcoAtom == NULL && m_Co64Atom == NULL) {
                    return AP4_ERROR_INVALID_FORMAT;
                }
                __state = 2;
                break;
            case 2:
                index++;
                __state = 3;
                break;
            case 3:
                AP4_Ordinal chunk, skip, desc;
                result = m_StscAtom->GetChunkForSample(index, chunk, skip, desc);
                if (AP4_FAILED(result)) return result;
                __state = 4;
                break;
            case 4:
                if (skip > index) return AP4_ERROR_INTERNAL;
                __state = 5;
                break;
            case 5:
                AP4_UI64 offset;
                if (m_StcoAtom) {
                    AP4_UI32 offset_32;
                    result = m_StcoAtom->GetChunkOffset(chunk, offset_32);
                    offset = offset_32;
                } else {
                    result = m_Co64Atom->GetChunkOffset(chunk, offset);
                }
                if (AP4_FAILED(result)) return result;
                __state = 6;
                break;
            case 6:
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
                __state = 7;
                break;
            case 7:
                sample.SetDescriptionIndex(desc-1);
                __state = 8;
                break;
            case 8:
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
                __state = 9;
                break;
            case 9:
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
                __state = 10;
                break;
            case 10:
                if (m_StssAtom == NULL) {
                    sample.SetSync(true);
                } else {
                    sample.SetSync(m_StssAtom->IsSampleSync(index));
                }
                __state = 11;
                break;
            case 11:
                sample.SetOffset(offset);
                __state = 12;
                break;
            case 12:
                sample.SetDataStream(m_SampleStream);
                __state = 13;
                break;
            case 13:
                return AP4_SUCCESS;
        }
    }
}

AP4_Cardinal
AP4_AtomSampleTable::GetSampleCount()
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                if (m_StszAtom) {
                    return m_StszAtom->GetSampleCount();
                } else if (m_Stz2Atom) {
                    return m_Stz2Atom->GetSampleCount();
                } else {
                    return 0;
                }
        }
    }
}

AP4_SampleDescription*
AP4_AtomSampleTable::GetSampleDescription(AP4_Ordinal index)
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                return m_StsdAtom ? m_StsdAtom->GetSampleDescription(index) : NULL;
        }
    }
}

AP4_Cardinal
AP4_AtomSampleTable::GetSampleDescriptionCount()
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                return m_StsdAtom ? m_StsdAtom->GetSampleDescriptionCount() : 0;
        }
    }
}

AP4_Result   
AP4_AtomSampleTable::GetSampleChunkPosition(AP4_Ordinal  sample_index, 
                                            AP4_Ordinal& chunk_index,
                                            AP4_Ordinal& position_in_chunk)
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                chunk_index       = 0;
                position_in_chunk = 0;
                __state = 1;
                break;
            case 1:
                AP4_Ordinal sample_description_index;
                return GetChunkForSample(sample_index, 
                                         chunk_index, 
                                         position_in_chunk, 
                                         sample_description_index);
        }
    }
}

AP4_Result 
AP4_AtomSampleTable::GetChunkForSample(AP4_Ordinal  sample_index,
                                       AP4_Ordinal& chunk_index,
                                       AP4_Ordinal& position_in_chunk,
                                       AP4_Ordinal& sample_description_index)
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                chunk_index              = 0;
                position_in_chunk        = 0;
                sample_description_index = 0;
                __state = 1;
                break;
            case 1:
                if (m_StscAtom == NULL) return AP4_ERROR_INVALID_STATE;
                __state = 2;
                break;
            case 2:
                AP4_Ordinal chunk = 0;
                AP4_Result result = m_StscAtom->GetChunkForSample(sample_index+1, 
                                                                  chunk, 
                                                                  position_in_chunk, 
                                                                  sample_description_index);
                if (AP4_FAILED(result)) return result;
                if (chunk == 0) return AP4_ERROR_INTERNAL;
                __state = 3;
                break;
            case 3:
                chunk_index = chunk-1;
                __state = 4;
                break;
            case 4:
                return AP4_SUCCESS;
        }
    }
}

AP4_Result 
AP4_AtomSampleTable::GetChunkOffset(AP4_Ordinal   chunk_index, 
                                    AP4_Position& offset)
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
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
    }
}

AP4_Result 
AP4_AtomSampleTable::SetChunkOffset(AP4_Ordinal  chunk_index, 
                                    AP4_Position offset)
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                if (m_StcoAtom) {
                    if ((offset >> 32) != 0) return AP4_ERROR_OUT_OF_RANGE;
                    return m_StcoAtom->SetChunkOffset(chunk_index+1, (AP4_UI32)offset);
                } else if (m_Co64Atom) {
                    return m_Co64Atom->SetChunkOffset(chunk_index+1, offset);
                } else {
                    return AP4_FAILURE;
                }
        }
    }
}

AP4_Result 
AP4_AtomSampleTable::SetSampleSize(AP4_Ordinal sample_index, AP4_Size size)
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                if (m_StszAtom) {
                    return m_StszAtom->SetSampleSize(sample_index+1, size);
                } else if (m_Stz2Atom) {
                    return m_Stz2Atom->SetSampleSize(sample_index+1, size);
                } else {    
                    return AP4_FAILURE;
                }
        }
    }
}

AP4_Result 
AP4_AtomSampleTable::GetSampleIndexForTimeStamp(AP4_UI64     ts, 
                                                AP4_Ordinal& sample_index)
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                return m_SttsAtom ? m_SttsAtom->GetSampleIndexForTimeStamp(ts, sample_index) 
                                  : AP4_FAILURE;
        }
    }
}

AP4_Ordinal  
AP4_AtomSampleTable::GetNearestSyncSampleIndex(AP4_Ordinal sample_index, bool before)
{
    int __state = 0;
    while (1) {
        switch (__state) {
            case 0:
                if (m_StssAtom == NULL) return sample_index;
                __state = 1;
                break;
            case 1:
                sample_index += 1;
                AP4_Cardinal entry_count = m_StssAtom->GetEntries().ItemCount();
                __state = 2;
                break;
            case 2:
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
    }
}