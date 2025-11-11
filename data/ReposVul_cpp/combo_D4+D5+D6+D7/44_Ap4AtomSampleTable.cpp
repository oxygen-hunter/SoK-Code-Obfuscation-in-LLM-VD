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

struct AP4_AtomSampleTable_Atoms {
    AP4_StscAtom* m_StscAtom;
    AP4_StcoAtom* m_StcoAtom;
    AP4_StszAtom* m_StszAtom;
    AP4_Stz2Atom* m_Stz2Atom;
    AP4_CttsAtom* m_CttsAtom;
    AP4_SttsAtom* m_SttsAtom;
    AP4_StssAtom* m_StssAtom;
    AP4_StsdAtom* m_StsdAtom;
    AP4_Co64Atom* m_Co64Atom;
};

AP4_AtomSampleTable::AP4_AtomSampleTable(AP4_ContainerAtom* stbl, 
                                         AP4_ByteStream& sample_stream) :
    m_SampleStream(sample_stream)
{
    AP4_AtomSampleTable_Atoms atoms;
    atoms.m_StscAtom = AP4_DYNAMIC_CAST(AP4_StscAtom, stbl->GetChild(AP4_ATOM_TYPE_STSC));
    atoms.m_StcoAtom = AP4_DYNAMIC_CAST(AP4_StcoAtom, stbl->GetChild(AP4_ATOM_TYPE_STCO));
    atoms.m_StszAtom = AP4_DYNAMIC_CAST(AP4_StszAtom, stbl->GetChild(AP4_ATOM_TYPE_STSZ));
    atoms.m_Stz2Atom = AP4_DYNAMIC_CAST(AP4_Stz2Atom, stbl->GetChild(AP4_ATOM_TYPE_STZ2));
    atoms.m_CttsAtom = AP4_DYNAMIC_CAST(AP4_CttsAtom, stbl->GetChild(AP4_ATOM_TYPE_CTTS));
    atoms.m_SttsAtom = AP4_DYNAMIC_CAST(AP4_SttsAtom, stbl->GetChild(AP4_ATOM_TYPE_STTS));
    atoms.m_StssAtom = AP4_DYNAMIC_CAST(AP4_StssAtom, stbl->GetChild(AP4_ATOM_TYPE_STSS));
    atoms.m_StsdAtom = AP4_DYNAMIC_CAST(AP4_StsdAtom, stbl->GetChild(AP4_ATOM_TYPE_STSD));
    atoms.m_Co64Atom = AP4_DYNAMIC_CAST(AP4_Co64Atom, stbl->GetChild(AP4_ATOM_TYPE_CO64));

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
    AP4_Ordinal c, s, d;
    r = m_StscAtom->GetChunkForSample(index, c, s, d);
    if (AP4_FAILED(r)) return r;
    
    if (s > index) return AP4_ERROR_INTERNAL;

    AP4_UI64 o;
    if (m_StcoAtom) {
        AP4_UI32 o_32;
        r = m_StcoAtom->GetChunkOffset(c, o_32);
        o = o_32;
    } else {
        r = m_Co64Atom->GetChunkOffset(c, o);
    }
    if (AP4_FAILED(r)) return r;
    
    for (unsigned int i = index-s; i < index; i++) {
        AP4_Size sz = 0;
        if (m_StszAtom) {
            r = m_StszAtom->GetSampleSize(i, sz); 
        } else if (m_Stz2Atom) {
            r = m_Stz2Atom->GetSampleSize(i, sz); 
        } else {
            r = AP4_ERROR_INVALID_FORMAT;
        }
        if (AP4_FAILED(r)) return r;
        o += sz;
    }

    sample.SetDescriptionIndex(d-1);

    AP4_UI32 co = 0;
    AP4_UI64 dt = 0;
    AP4_UI32 du = 0;
    r = m_SttsAtom->GetDts(index, dt, &du);
    if (AP4_FAILED(r)) return r;
    sample.SetDuration(du);
    sample.SetDts(dt);
    if (m_CttsAtom == NULL) {
        sample.SetCts(dt);
    } else {
        r = m_CttsAtom->GetCtsOffset(index, co); 
	    if (AP4_FAILED(r)) return r;
        sample.SetCtsDelta(co);
    }     

    AP4_Size ss = 0;
    if (m_StszAtom) {
        r = m_StszAtom->GetSampleSize(index, ss); 
    } else if (m_Stz2Atom) {
        r = m_Stz2Atom->GetSampleSize(index, ss); 
    } else {
        r = AP4_ERROR_INVALID_FORMAT;
    }
    if (AP4_FAILED(r)) return r;
    sample.SetSize(ss);

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
    
    AP4_Ordinal ch = 0;
    AP4_Result rs = m_StscAtom->GetChunkForSample(sample_index+1, 
                                                      ch, 
                                                      position_in_chunk, 
                                                      sample_description_index);
    if (AP4_FAILED(rs)) return rs;
    if (ch == 0) return AP4_ERROR_INTERNAL;

    chunk_index = ch-1;
    
    return AP4_SUCCESS;
}

AP4_Result 
AP4_AtomSampleTable::GetChunkOffset(AP4_Ordinal   chunk_index, 
                                    AP4_Position& offset)
{
    if (m_StcoAtom) {
        AP4_UI32 o_32;
        AP4_Result rs = m_StcoAtom->GetChunkOffset(chunk_index+1, o_32);
        if (AP4_SUCCEEDED(rs)) {
            offset = o_32;
        } else {
            offset = 0;
        }
        return rs;
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
        if ((offset >> 32) != 0) return AP4_ERROR_OUT_OF_RANGE;
        return m_StcoAtom->SetChunkOffset(chunk_index+1, (AP4_UI32)offset);
    } else if (m_Co64Atom) {
        return m_Co64Atom->SetChunkOffset(chunk_index+1, offset);
    } else {
        return AP4_FAILURE;
    }
}

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
        AP4_Ordinal cr = 0;    
        for (unsigned int i=0; i<entry_count; i++) {
            if (m_StssAtom->GetEntries()[i] >= sample_index) return cr;
            if (m_StssAtom->GetEntries()[i]) cr = m_StssAtom->GetEntries()[i]-1;
        }
        return cr;
    } else {
        for (unsigned int i=0; i<entry_count; i++) {
            if (m_StssAtom->GetEntries()[i] >= sample_index) {
                return m_StssAtom->GetEntries()[i]?m_StssAtom->GetEntries()[i]-1:sample_index-1;
            }
        }
        return GetSampleCount();
    }
}