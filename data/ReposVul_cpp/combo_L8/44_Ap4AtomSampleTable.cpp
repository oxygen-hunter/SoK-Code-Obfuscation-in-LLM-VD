#include <Python.h>
#include <dlfcn.h>

typedef unsigned int AP4_Ordinal;
typedef unsigned int AP4_Size;
typedef unsigned int AP4_UI32;
typedef unsigned long long AP4_UI64;
typedef unsigned int AP4_Cardinal;
typedef long AP4_Position;
typedef int AP4_Result;
#define AP4_SUCCESS 0
#define AP4_FAILURE (-1)
#define AP4_ERROR_OUT_OF_RANGE (-2)
#define AP4_ERROR_INTERNAL (-3)
#define AP4_ERROR_INVALID_FORMAT (-4)
#define AP4_ERROR_INVALID_STATE (-5)
#define AP4_FAILED(x) ((x) != AP4_SUCCESS)
#define AP4_SUCCEEDED(x) ((x) == AP4_SUCCESS)

class AP4_ByteStream {};
class AP4_ContainerAtom {};
class AP4_Sample {};
class AP4_SampleDescription {};
class AP4_StscAtom {};
class AP4_StcoAtom {};
class AP4_StszAtom {};
class AP4_Stz2Atom {};
class AP4_CttsAtom {};
class AP4_SttsAtom {};
class AP4_StssAtom {};
class AP4_StsdAtom {};
class AP4_Co64Atom {};

class AP4_AtomSampleTable {
public:
    AP4_AtomSampleTable(AP4_ContainerAtom* stbl, AP4_ByteStream& sample_stream);
    ~AP4_AtomSampleTable();
    AP4_Result GetSample(AP4_Ordinal index, AP4_Sample& sample);
    AP4_Cardinal GetSampleCount();
    AP4_SampleDescription* GetSampleDescription(AP4_Ordinal index);
    AP4_Cardinal GetSampleDescriptionCount();
    AP4_Result GetSampleChunkPosition(AP4_Ordinal sample_index, AP4_Ordinal& chunk_index, AP4_Ordinal& position_in_chunk);
    AP4_Result GetChunkForSample(AP4_Ordinal sample_index, AP4_Ordinal& chunk_index, AP4_Ordinal& position_in_chunk, AP4_Ordinal& sample_description_index);
    AP4_Result GetChunkOffset(AP4_Ordinal chunk_index, AP4_Position& offset);
    AP4_Result SetChunkOffset(AP4_Ordinal chunk_index, AP4_Position offset);
    AP4_Result SetSampleSize(AP4_Ordinal sample_index, AP4_Size size);
    AP4_Result GetSampleIndexForTimeStamp(AP4_UI64 ts, AP4_Ordinal& sample_index);
    AP4_Ordinal GetNearestSyncSampleIndex(AP4_Ordinal sample_index, bool before);

private:
    AP4_ByteStream& m_SampleStream;
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

AP4_AtomSampleTable::AP4_AtomSampleTable(AP4_ContainerAtom* stbl, AP4_ByteStream& sample_stream) :
    m_SampleStream(sample_stream), m_StscAtom(nullptr), m_StcoAtom(nullptr), m_StszAtom(nullptr), m_Stz2Atom(nullptr),
    m_CttsAtom(nullptr), m_SttsAtom(nullptr), m_StssAtom(nullptr), m_StsdAtom(nullptr), m_Co64Atom(nullptr) {
    // Python interpreter initialization
    Py_Initialize();

    // Load shared library (for demonstration purpose only, not functional)
    void *handle = dlopen("libap4.so", RTLD_LAZY);
    if (!handle) {
        PyErr_Print();
    }

    // Casting atoms from container (mocked as nullptr for demonstration)
    m_StscAtom = static_cast<AP4_StscAtom*>(nullptr);
    m_StcoAtom = static_cast<AP4_StcoAtom*>(nullptr);
    m_StszAtom = static_cast<AP4_StszAtom*>(nullptr);
    m_Stz2Atom = static_cast<AP4_Stz2Atom*>(nullptr);
    m_CttsAtom = static_cast<AP4_CttsAtom*>(nullptr);
    m_SttsAtom = static_cast<AP4_SttsAtom*>(nullptr);
    m_StssAtom = static_cast<AP4_StssAtom*>(nullptr);
    m_StsdAtom = static_cast<AP4_StsdAtom*>(nullptr);
    m_Co64Atom = static_cast<AP4_Co64Atom*>(nullptr);

    // Reference sample stream
    // m_SampleStream.AddReference();
}

AP4_AtomSampleTable::~AP4_AtomSampleTable() {
    // Release resources
    // m_SampleStream.Release();
    Py_Finalize();
}

AP4_Result AP4_AtomSampleTable::GetSample(AP4_Ordinal index, AP4_Sample& sample) {
    AP4_Result result;

    if (!m_StscAtom) {
        return AP4_ERROR_INVALID_FORMAT;
    }

    if (m_StcoAtom == nullptr && m_Co64Atom == nullptr) {
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
    AP4_UI64 dts = 0;
    AP4_UI32 duration = 0;
    result = m_SttsAtom->GetDts(index, dts, &duration);
    if (AP4_FAILED(result)) return result;
    sample.SetDuration(duration);
    sample.SetDts(dts);
    if (m_CttsAtom == nullptr) {
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

    if (m_StssAtom == nullptr) {
        sample.SetSync(true);
    } else {
        sample.SetSync(m_StssAtom->IsSampleSync(index));
    }

    sample.SetOffset(offset);
    sample.SetDataStream(m_SampleStream);

    return AP4_SUCCESS;
}

AP4_Cardinal AP4_AtomSampleTable::GetSampleCount() {
    if (m_StszAtom) {
        return m_StszAtom->GetSampleCount();
    } else if (m_Stz2Atom) {
        return m_Stz2Atom->GetSampleCount();
    } else {
        return 0;
    }
}

AP4_SampleDescription* AP4_AtomSampleTable::GetSampleDescription(AP4_Ordinal index) {
    return m_StsdAtom ? m_StsdAtom->GetSampleDescription(index) : nullptr;
}

AP4_Cardinal AP4_AtomSampleTable::GetSampleDescriptionCount() {
    return m_StsdAtom ? m_StsdAtom->GetSampleDescriptionCount() : 0;
}

AP4_Result AP4_AtomSampleTable::GetSampleChunkPosition(AP4_Ordinal sample_index, AP4_Ordinal& chunk_index, AP4_Ordinal& position_in_chunk) {
    chunk_index = 0;
    position_in_chunk = 0;

    AP4_Ordinal sample_description_index;
    return GetChunkForSample(sample_index, chunk_index, position_in_chunk, sample_description_index);
}

AP4_Result AP4_AtomSampleTable::GetChunkForSample(AP4_Ordinal sample_index, AP4_Ordinal& chunk_index, AP4_Ordinal& position_in_chunk, AP4_Ordinal& sample_description_index) {
    chunk_index = 0;
    position_in_chunk = 0;
    sample_description_index = 0;

    if (m_StscAtom == nullptr) return AP4_ERROR_INVALID_STATE;

    AP4_Ordinal chunk = 0;
    AP4_Result result = m_StscAtom->GetChunkForSample(sample_index+1, chunk, position_in_chunk, sample_description_index);
    if (AP4_FAILED(result)) return result;
    if (chunk == 0) return AP4_ERROR_INTERNAL;

    chunk_index = chunk-1;

    return AP4_SUCCESS;
}

AP4_Result AP4_AtomSampleTable::GetChunkOffset(AP4_Ordinal chunk_index, AP4_Position& offset) {
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

AP4_Result AP4_AtomSampleTable::SetChunkOffset(AP4_Ordinal chunk_index, AP4_Position offset) {
    if (m_StcoAtom) {
        if ((offset >> 32) != 0) return AP4_ERROR_OUT_OF_RANGE;
        return m_StcoAtom->SetChunkOffset(chunk_index+1, static_cast<AP4_UI32>(offset));
    } else if (m_Co64Atom) {
        return m_Co64Atom->SetChunkOffset(chunk_index+1, offset);
    } else {
        return AP4_FAILURE;
    }
}

AP4_Result AP4_AtomSampleTable::SetSampleSize(AP4_Ordinal sample_index, AP4_Size size) {
    if (m_StszAtom) {
        return m_StszAtom->SetSampleSize(sample_index+1, size);
    } else if (m_Stz2Atom) {
        return m_Stz2Atom->SetSampleSize(sample_index+1, size);
    } else {
        return AP4_FAILURE;
    }
}

AP4_Result AP4_AtomSampleTable::GetSampleIndexForTimeStamp(AP4_UI64 ts, AP4_Ordinal& sample_index) {
    return m_SttsAtom ? m_SttsAtom->GetSampleIndexForTimeStamp(ts, sample_index) : AP4_FAILURE;
}

AP4_Ordinal AP4_AtomSampleTable::GetNearestSyncSampleIndex(AP4_Ordinal sample_index, bool before) {
    if (m_StssAtom == nullptr) return sample_index;

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