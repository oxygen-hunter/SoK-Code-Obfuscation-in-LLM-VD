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

#define AP4_VM_PUSH 0x01
#define AP4_VM_POP 0x02
#define AP4_VM_ADD 0x03
#define AP4_VM_SUB 0x04
#define AP4_VM_JMP 0x05
#define AP4_VM_JZ 0x06
#define AP4_VM_LOAD 0x07
#define AP4_VM_STORE 0x08
#define AP4_VM_HALT 0xFF

class AP4_VirtualMachine {
public:
    AP4_VirtualMachine(unsigned char* code, int size) : 
        m_Code(code), m_Size(size), m_PC(0) {}

    void Execute() {
        bool running = true;
        while (running && m_PC < m_Size) {
            unsigned char opcode = m_Code[m_PC++];
            switch (opcode) {
                case AP4_VM_PUSH: {
                    int value = m_Code[m_PC++];
                    m_Stack[m_SP++] = value;
                    break;
                }
                case AP4_VM_POP: {
                    --m_SP;
                    break;
                }
                case AP4_VM_ADD: {
                    int a = m_Stack[--m_SP];
                    int b = m_Stack[--m_SP];
                    m_Stack[m_SP++] = a + b;
                    break;
                }
                case AP4_VM_SUB: {
                    int a = m_Stack[--m_SP];
                    int b = m_Stack[--m_SP];
                    m_Stack[m_SP++] = a - b;
                    break;
                }
                case AP4_VM_JMP: {
                    int addr = m_Code[m_PC++];
                    m_PC = addr;
                    break;
                }
                case AP4_VM_JZ: {
                    int addr = m_Code[m_PC++];
                    if (m_Stack[--m_SP] == 0) {
                        m_PC = addr;
                    }
                    break;
                }
                case AP4_VM_LOAD: {
                    int addr = m_Code[m_PC++];
                    m_Stack[m_SP++] = m_Memory[addr];
                    break;
                }
                case AP4_VM_STORE: {
                    int addr = m_Code[m_PC++];
                    m_Memory[addr] = m_Stack[--m_SP];
                    break;
                }
                case AP4_VM_HALT: {
                    running = false;
                    break;
                }
            }
        }
    }

private:
    unsigned char* m_Code;
    int m_Size;
    int m_PC;
    int m_SP = 0;
    int m_Stack[256];
    int m_Memory[256];
};

AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_AtomSampleTable)

AP4_AtomSampleTable::AP4_AtomSampleTable(AP4_ContainerAtom* stbl, 
                                         AP4_ByteStream&    sample_stream) :
    m_SampleStream(sample_stream)
{
    unsigned char code[] = {
        AP4_VM_PUSH, AP4_ATOM_TYPE_STSC,
        AP4_VM_LOAD, 0,
        AP4_VM_STORE, 1,
        AP4_VM_PUSH, AP4_ATOM_TYPE_STCO,
        AP4_VM_LOAD, 0,
        AP4_VM_STORE, 2,
        AP4_VM_PUSH, AP4_ATOM_TYPE_STSZ,
        AP4_VM_LOAD, 0,
        AP4_VM_STORE, 3,
        AP4_VM_PUSH, AP4_ATOM_TYPE_STZ2,
        AP4_VM_LOAD, 0,
        AP4_VM_STORE, 4,
        AP4_VM_PUSH, AP4_ATOM_TYPE_CTTS,
        AP4_VM_LOAD, 0,
        AP4_VM_STORE, 5,
        AP4_VM_PUSH, AP4_ATOM_TYPE_STTS,
        AP4_VM_LOAD, 0,
        AP4_VM_STORE, 6,
        AP4_VM_PUSH, AP4_ATOM_TYPE_STSS,
        AP4_VM_LOAD, 0,
        AP4_VM_STORE, 7,
        AP4_VM_PUSH, AP4_ATOM_TYPE_STSD,
        AP4_VM_LOAD, 0,
        AP4_VM_STORE, 8,
        AP4_VM_PUSH, AP4_ATOM_TYPE_CO64,
        AP4_VM_LOAD, 0,
        AP4_VM_STORE, 9,
        AP4_VM_HALT
    };
    int code_size = sizeof(code) / sizeof(code[0]);

    AP4_VirtualMachine vm(code, code_size);
    vm.Execute();

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