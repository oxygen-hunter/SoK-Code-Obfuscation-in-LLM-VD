extern "C" {
#include <Python.h>
}
#include <iostream>
#include <dlfcn.h>
#include <cstdlib>

#define AP4_DEFINE_DYNAMIC_CAST_ANCHOR(class_name) \
    AP4_DynamicCastAnchor<class_name> class_name##_DynamicCastAnchor

class AP4_AtomSampleTable {
    public:
        AP4_AtomSampleTable(void* stbl, PyObject* sample_stream);
        ~AP4_AtomSampleTable();
        int GetSample(int index, PyObject* sample);
        unsigned int GetSampleCount();
        PyObject* GetSampleDescription(int index);
        unsigned int GetSampleDescriptionCount();
        int GetSampleChunkPosition(int sample_index, int& chunk_index, int& position_in_chunk);
        int GetChunkForSample(int sample_index, int& chunk_index, int& position_in_chunk, int& sample_description_index);
        int GetChunkOffset(int chunk_index, unsigned long long& offset);
        int SetChunkOffset(int chunk_index, unsigned long long offset);
        int SetSampleSize(int sample_index, unsigned int size);
        int GetSampleIndexForTimeStamp(unsigned long long ts, int& sample_index);
        int GetNearestSyncSampleIndex(int sample_index, bool before);

    private:
        PyObject* m_SampleStream;
        void* m_StscAtom;
        void* m_StcoAtom;
        void* m_StszAtom;
        void* m_Stz2Atom;
        void* m_CttsAtom;
        void* m_SttsAtom;
        void* m_StssAtom;
        void* m_StsdAtom;
        void* m_Co64Atom;
};

AP4_AtomSampleTable::AP4_AtomSampleTable(void* stbl, PyObject* sample_stream) : m_SampleStream(sample_stream) {
    m_StscAtom = dlsym(stbl, "AP4_StscAtom");
    m_StcoAtom = dlsym(stbl, "AP4_StcoAtom");
    m_StszAtom = dlsym(stbl, "AP4_StszAtom");
    m_Stz2Atom = dlsym(stbl, "AP4_Stz2Atom");
    m_CttsAtom = dlsym(stbl, "AP4_CttsAtom");
    m_SttsAtom = dlsym(stbl, "AP4_SttsAtom");
    m_StssAtom = dlsym(stbl, "AP4_StssAtom");
    m_StsdAtom = dlsym(stbl, "AP4_StsdAtom");
    m_Co64Atom = dlsym(stbl, "AP4_Co64Atom");

    Py_INCREF(m_SampleStream);
}

AP4_AtomSampleTable::~AP4_AtomSampleTable() {
    Py_DECREF(m_SampleStream);
}

int AP4_AtomSampleTable::GetSample(int index, PyObject* sample) {
    int result;
    if (!m_StscAtom) {
        return -1;
    }
    if (m_StcoAtom == NULL && m_Co64Atom == NULL) {
        return -1;
    }
    index++;
    int chunk, skip, desc;
    result = ((int(*)(int, int&, int&, int&)) m_StscAtom)(index, chunk, skip, desc);
    if (result != 0) return result;

    if (skip > index) return -1;

    unsigned long long offset;
    if (m_StcoAtom) {
        unsigned int offset_32;
        result = ((int(*)(int, unsigned int&)) m_StcoAtom)(chunk, offset_32);
        offset = offset_32;
    } else {
        result = ((int(*)(int, unsigned long long&)) m_Co64Atom)(chunk, offset);
    }
    if (result != 0) return result;

    for (unsigned int i = index-skip; i < index; i++) {
        unsigned int size = 0;
        if (m_StszAtom) {
            result = ((int(*)(int, unsigned int&)) m_StszAtom)(i, size);
        } else if (m_Stz2Atom) {
            result = ((int(*)(int, unsigned int&)) m_Stz2Atom)(i, size);
        } else {
            return -1;
        }
        if (result != 0) return result;
        offset += size;
    }

    unsigned int cts_offset = 0;
    unsigned long long dts = 0;
    unsigned int duration = 0;
    result = ((int(*)(int, unsigned long long&, unsigned int*)) m_SttsAtom)(index, dts, &duration);
    if (result != 0) return result;

    if (m_CttsAtom == NULL) {
        ((void(*)(unsigned long long)) sample)(dts);
    } else {
        result = ((int(*)(int, unsigned int&)) m_CttsAtom)(index, cts_offset);
        if (result != 0) return result;
        ((void(*)(unsigned int)) sample)(cts_offset);
    }

    unsigned int sample_size = 0;
    if (m_StszAtom) {
        result = ((int(*)(int, unsigned int&)) m_StszAtom)(index, sample_size);
    } else if (m_Stz2Atom) {
        result = ((int(*)(int, unsigned int&)) m_Stz2Atom)(index, sample_size);
    } else {
        return -1;
    }
    if (result != 0) return result;

    if (m_StssAtom == NULL) {
        ((void(*)(bool)) sample)(true);
    } else {
        ((void(*)(bool)) sample)(((bool(*)(int)) m_StssAtom)(index));
    }

    ((void(*)(unsigned long long)) sample)(offset);
    ((void(*)(PyObject*)) sample)(m_SampleStream);

    return 0;
}

unsigned int AP4_AtomSampleTable::GetSampleCount() {
    if (m_StszAtom) {
        return ((unsigned int(*)()) m_StszAtom)();
    } else if (m_Stz2Atom) {
        return ((unsigned int(*)()) m_Stz2Atom)();
    } else {
        return 0;
    }
}

PyObject* AP4_AtomSampleTable::GetSampleDescription(int index) {
    return m_StsdAtom ? ((PyObject*(*)(int)) m_StsdAtom)(index) : NULL;
}

unsigned int AP4_AtomSampleTable::GetSampleDescriptionCount() {
    return m_StsdAtom ? ((unsigned int(*)()) m_StsdAtom)() : 0;
}

int AP4_AtomSampleTable::GetSampleChunkPosition(int sample_index, int& chunk_index, int& position_in_chunk) {
    chunk_index = 0;
    position_in_chunk = 0;
    int sample_description_index;
    return GetChunkForSample(sample_index, chunk_index, position_in_chunk, sample_description_index);
}

int AP4_AtomSampleTable::GetChunkForSample(int sample_index, int& chunk_index, int& position_in_chunk, int& sample_description_index) {
    chunk_index = 0;
    position_in_chunk = 0;
    sample_description_index = 0;

    if (m_StscAtom == NULL) return -1;
    int chunk = 0;
    int result = ((int(*)(int, int&, int&, int&)) m_StscAtom)(sample_index+1, chunk, position_in_chunk, sample_description_index);
    if (result != 0) return result;
    if (chunk == 0) return -1;

    chunk_index = chunk-1;
    return 0;
}

int AP4_AtomSampleTable::GetChunkOffset(int chunk_index, unsigned long long& offset) {
    if (m_StcoAtom) {
        unsigned int offset_32;
        int result = ((int(*)(int, unsigned int&)) m_StcoAtom)(chunk_index+1, offset_32);
        if (result == 0) {
            offset = offset_32;
        } else {
            offset = 0;
        }
        return result;
    } else if (m_Co64Atom) {
        return ((int(*)(int, unsigned long long&)) m_Co64Atom)(chunk_index+1, offset);
    } else {
        offset = 0;
        return -1;
    }
}

int AP4_AtomSampleTable::SetChunkOffset(int chunk_index, unsigned long long offset) {
    if (m_StcoAtom) {
        if ((offset >> 32) != 0) return -1;
        return ((int(*)(int, unsigned int)) m_StcoAtom)(chunk_index+1, (unsigned int)offset);
    } else if (m_Co64Atom) {
        return ((int(*)(int, unsigned long long)) m_Co64Atom)(chunk_index+1, offset);
    } else {
        return -1;
    }
}

int AP4_AtomSampleTable::SetSampleSize(int sample_index, unsigned int size) {
    if (m_StszAtom) {
        return ((int(*)(int, unsigned int)) m_StszAtom)(sample_index+1, size);
    } else if (m_Stz2Atom) {
        return ((int(*)(int, unsigned int)) m_Stz2Atom)(sample_index+1, size);
    } else {
        return -1;
    }
}

int AP4_AtomSampleTable::GetSampleIndexForTimeStamp(unsigned long long ts, int& sample_index) {
    return m_SttsAtom ? ((int(*)(unsigned long long, int&)) m_SttsAtom)(ts, sample_index) : -1;
}

int AP4_AtomSampleTable::GetNearestSyncSampleIndex(int sample_index, bool before) {
    if (m_StssAtom == NULL) return sample_index;

    sample_index += 1;
    unsigned int entry_count = ((unsigned int(*)()) m_StssAtom)();
    if (before) {
        int cursor = 0;
        for (unsigned int i=0; i<entry_count; i++) {
            if (((int(*)(int)) m_StssAtom)(i) >= sample_index) return cursor;
            if (((int(*)(int)) m_StssAtom)(i)) cursor = ((int(*)(int)) m_StssAtom)(i)-1;
        }
        return cursor;
    } else {
        for (unsigned int i=0; i<entry_count; i++) {
            if (((int(*)(int)) m_StssAtom)(i) >= sample_index) {
                return ((int(*)(int)) m_StssAtom)(i) ? ((int(*)(int)) m_StssAtom)(i)-1 : sample_index-1;
            }
        }
        return GetSampleCount();
    }
}