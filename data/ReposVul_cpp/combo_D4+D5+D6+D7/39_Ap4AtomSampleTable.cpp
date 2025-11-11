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
    m_StscAtom = AP4_DYNAMIC_CAST(AP4_StscAtom, stbl->GetChild(AP4_ATOM_TYPE_STSC));
    m_StcoAtom = AP4_DYNAMIC_CAST(AP4_StcoAtom, stbl->GetChild(AP4_ATOM_TYPE_STCO));
    m_StszAtom = AP4_DYNAMIC_CAST(AP4_StszAtom, stbl->GetChild(AP4_ATOM_TYPE_STSZ));
    m_Stz2Atom = AP4_DYNAMIC_CAST(AP4_Stz2Atom, stbl->GetChild(AP4_ATOM_TYPE_STZ2));
    m_CttsAtom = AP4_DYNAMIC_CAST(AP4_CttsAtom, stbl->GetChild(AP4_ATOM_TYPE_CTTS));
    m_SttsAtom = AP4_DYNAMIC_CAST(AP4_SttsAtom, stbl->GetChild(AP4_ATOM_TYPE_STTS));
    m_StssAtom = AP4_DYNAMIC_CAST(AP4_StssAtom, stbl->GetChild(AP4_ATOM_TYPE_STSS));
    m_StsdAtom = AP4_DYNAMIC_CAST(AP4_StsdAtom, stbl->GetChild(AP4_ATOM_TYPE_STSD));
    m_Co64Atom = AP4_DYNAMIC_CAST(AP4_Co64Atom, stbl->GetChild(AP4_ATOM_TYPE_CO64));

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
    AP4_Result r;

    if (!m_StscAtom) {
        return AP4_ERROR_INVALID_FORMAT;
    }
    
    if (m_StcoAtom == NULL && m_Co64Atom == NULL) {
        return AP4_ERROR_INVALID_FORMAT;
    }

    index++;

    AP4_Ordinal chunk, skip, desc;
    r = m_StscAtom->GetChunkForSample(index, chunk, skip, desc);
    if (AP4_FAILED(r)) return r;
    
    if (skip > index) return AP4_ERROR_INTERNAL;

    AP4_UI64 o;
    if (m_StcoAtom) {
        AP4_UI32 o_32;
        r = m_StcoAtom->GetChunkOffset(chunk, o_32);
        o = o_32;
    } else {
        r = m_Co64Atom->GetChunkOffset(chunk, o);
    }
    if (AP4_FAILED(r)) return r;
    
    for (unsigned int i = index-skip; i < index; i++) {
        AP4_Size s = 0;
        if (m_StszAtom) {
            r = m_StszAtom->GetSampleSize(i, s); 
        } else if (m_Stz2Atom) {
            r = m_Stz2Atom->GetSampleSize(i, s); 
        } else {
            r = AP4_ERROR_INVALID_FORMAT;
        }
        if (AP4_FAILED(r)) return r;
        o += s;
    }

    sample.SetDescriptionIndex(desc-1);

    AP4_UI32 cts_o = 0;
    AP4_UI64 dts   = 0;
    AP4_UI32 dur   = 0;
    r = m_SttsAtom->GetDts(index, dts, &dur);
    if (AP4_FAILED(r)) return r;
    sample.SetDuration(dur);
    sample.SetDts(dts);
    if (m_CttsAtom == NULL) {
        sample.SetCts(dts);
    } else {
        r = m_CttsAtom->GetCtsOffset(index, cts_o); 
        if (AP4_FAILED(r)) return r;
        sample.SetCtsDelta(cts_o);
    }     

    AP4_Size s_size = 0;
    if (m_StszAtom) {
        r = m_StszAtom->GetSampleSize(index, s_size); 
    } else if (m_Stz2Atom) {
        r = m_Stz2Atom->GetSampleSize(index, s_size); 
    } else {
        r = AP4_ERROR_INVALID_FORMAT;
    }
    if (AP4_FAILED(r)) return r;
    sample.SetSize(s_size);

    if (m_StssAtom == NULL) {
        sample.SetSync(true);
    } else {
        sample.SetSync(m_StssAtom->IsSampleSync(index));
    }

    sample.SetOffset(o);

    sample.SetDataStream(m_SampleStream);

    return AP4_SUCCESS;
}

AP4_Cardinal
AP4_AtomSampleTable::GetSampleCount()
{
    if (m_StszAtom) {
        return m_StszAtom->GetSampleCount();
    } else if (m_Stz2Atom) {
        return m_Stz2Atom->GetSampleCount();
    } else {
        return 0;
    }
}

AP4_SampleDescription*
AP4_AtomSampleTable::GetSampleDescription(AP4_Ordinal i)
{
    return m_StsdAtom ? m_StsdAtom->GetSampleDescription(i) : NULL;
}

AP4_Cardinal
AP4_AtomSampleTable::GetSampleDescriptionCount()
{
    return m_StsdAtom ? m_StsdAtom->GetSampleDescriptionCount() : 0;
}

AP4_Result   
AP4_AtomSampleTable::GetSampleChunkPosition(AP4_Ordinal  s_index, 
                                            AP4_Ordinal& c_index,
                                            AP4_Ordinal& pos_in_chunk)
{
    c_index       = 0;
    pos_in_chunk = 0;
    
    AP4_Ordinal s_desc_index;
    return GetChunkForSample(s_index, 
                             c_index, 
                             pos_in_chunk, 
                             s_desc_index);
}

AP4_Result 
AP4_AtomSampleTable::GetChunkForSample(AP4_Ordinal  s_index,
                                       AP4_Ordinal& c_index,
                                       AP4_Ordinal& pos_in_chunk,
                                       AP4_Ordinal& s_desc_index)
{
    c_index              = 0;
    pos_in_chunk        = 0;
    s_desc_index = 0;

    if (m_StscAtom == NULL) return AP4_ERROR_INVALID_STATE;
    
    AP4_Ordinal c = 0;
    AP4_Result r = m_StscAtom->GetChunkForSample(s_index+1,
                                                      c, 
                                                      pos_in_chunk, 
                                                      s_desc_index);
    if (AP4_FAILED(r)) return r;
    if (c == 0) return AP4_ERROR_INTERNAL;

    c_index = c-1;
    
    return AP4_SUCCESS;
}

AP4_Result 
AP4_AtomSampleTable::GetChunkOffset(AP4_Ordinal   c_index, 
                                    AP4_Position& o)
{
    if (m_StcoAtom) {
        AP4_UI32 o_32;
        AP4_Result r = m_StcoAtom->GetChunkOffset(c_index+1, o_32);
        if (AP4_SUCCEEDED(r)) {
            o = o_32;
        } else {
            o = 0;
        }
        return r;
    } else if (m_Co64Atom) {
        return m_Co64Atom->GetChunkOffset(c_index+1, o);
    } else {
        o = 0;
        return AP4_FAILURE;
    }
}

AP4_Result 
AP4_AtomSampleTable::SetChunkOffset(AP4_Ordinal  c_index, 
                                    AP4_Position o)
{
    if (m_StcoAtom) {
        if ((o >> 32) != 0) return AP4_ERROR_OUT_OF_RANGE;
        return m_StcoAtom->SetChunkOffset(c_index+1, (AP4_UI32)o);
    } else if (m_Co64Atom) {
        return m_Co64Atom->SetChunkOffset(c_index+1, o);
    } else {
        return AP4_FAILURE;
    }
}

AP4_Result 
AP4_AtomSampleTable::SetSampleSize(AP4_Ordinal s_index, AP4_Size s)
{
    if (m_StszAtom) {
        return m_StszAtom->SetSampleSize(s_index+1, s);
    } else if (m_Stz2Atom) {
        return m_Stz2Atom->SetSampleSize(s_index+1, s);
    } else {    
        return AP4_FAILURE;
    }
}

AP4_Result 
AP4_AtomSampleTable::GetSampleIndexForTimeStamp(AP4_UI64     ts, 
                                                AP4_Ordinal& s_index)
{
    return m_SttsAtom ? m_SttsAtom->GetSampleIndexForTimeStamp(ts, s_index) 
                      : AP4_FAILURE;
}

AP4_Ordinal  
AP4_AtomSampleTable::GetNearestSyncSampleIndex(AP4_Ordinal s_index, bool before)
{
    if (m_StssAtom == NULL) return s_index;
    
    s_index += 1;
    AP4_Cardinal e_count = m_StssAtom->GetEntries().ItemCount();
    if (before) {
        AP4_Ordinal cursor = 0;    
        for (unsigned int i=0; i<e_count; i++) {
            if (m_StssAtom->GetEntries()[i] >= s_index) return cursor;
            if (m_StssAtom->GetEntries()[i]) cursor = m_StssAtom->GetEntries()[i]-1;
        }

        return cursor;
    } else {
        for (unsigned int i=0; i<e_count; i++) {
            if (m_StssAtom->GetEntries()[i] >= s_index) {
                return m_StssAtom->GetEntries()[i]?m_StssAtom->GetEntries()[i]-1:s_index-1;
            }
        }

        return GetSampleCount();
    }
}