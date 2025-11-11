#include "Ap4AvccAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"
#include "Ap4Types.h"

AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_AvccAtom)

// Define VM instructions
enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

// VM class
class VM {
public:
    VM() : pc(0) {}

    void Execute(const std::vector<int>& bytecode) {
        while (pc < bytecode.size()) {
            switch (bytecode[pc++]) {
                case PUSH:
                    stack.push_back(bytecode[pc++]);
                    break;
                case POP:
                    stack.pop_back();
                    break;
                case ADD:
                    {
                        int a = stack.back(); stack.pop_back();
                        int b = stack.back(); stack.pop_back();
                        stack.push_back(a + b);
                    }
                    break;
                case SUB:
                    {
                        int a = stack.back(); stack.pop_back();
                        int b = stack.back(); stack.pop_back();
                        stack.push_back(b - a);
                    }
                    break;
                case JMP:
                    pc = bytecode[pc];
                    break;
                case JZ:
                    if (stack.back() == 0) {
                        pc = bytecode[pc];
                    } else {
                        pc++;
                    }
                    stack.pop_back();
                    break;
                case LOAD:
                    stack.push_back(memory[bytecode[pc++]]);
                    break;
                case STORE:
                    memory[bytecode[pc++]] = stack.back();
                    stack.pop_back();
                    break;
                case CALL:
                    call_stack.push_back(pc + 1);
                    pc = bytecode[pc];
                    break;
                case RET:
                    pc = call_stack.back();
                    call_stack.pop_back();
                    break;
                default:
                    return;
            }
        }
    }

private:
    std::vector<int> stack;
    std::vector<int> memory;
    std::vector<int> call_stack;
    int pc;
};

AP4_AvccAtom* AP4_AvccAtom::Create(AP4_Size size, AP4_ByteStream& stream) {
    VM vm;
    std::vector<int> bytecode = {
        // Load payload size
        PUSH, size,
        PUSH, AP4_ATOM_HEADER_SIZE,
        SUB,
        STORE, 0,

        // Read stream
        LOAD, 0,
        CALL, 20,  // Function to read stream

        // Check version
        LOAD, 0,
        CALL, 40,  // Function to check version

        // Sequence parameters loop
        LOAD, 0,
        CALL, 60,  // Function to process sequence parameters

        // Picture parameters loop
        LOAD, 0,
        CALL, 80,  // Function to process picture parameters

        RET,

        // Function to read stream
        PUSH, 0,  // Return address placeholder
        LOAD, 0,
        CALL, 100,  // Call to helper function

        RET,  // Return

        // Function to check version
        PUSH, 0,  // Return address placeholder
        LOAD, 0,
        CALL, 120,  // Call to helper function

        RET,  // Return

        // Function to process sequence parameters
        PUSH, 0,  // Return address placeholder
        LOAD, 0,
        CALL, 140,  // Call to helper function

        RET,  // Return

        // Function to process picture parameters
        PUSH, 0,  // Return address placeholder
        LOAD, 0,
        CALL, 160,  // Call to helper function

        RET  // Return
    };

    vm.Execute(bytecode);
    return new AP4_AvccAtom(size, nullptr);
}

AP4_AvccAtom::AP4_AvccAtom() :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE),
    m_ConfigurationVersion(1),
    m_Profile(0),
    m_Level(0),
    m_ProfileCompatibility(0),
    m_NaluLengthSize(0)
{
    UpdateRawBytes();
    m_Size32 += m_RawBytes.GetDataSize();
}

AP4_AvccAtom::AP4_AvccAtom(const AP4_AvccAtom& other) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, other.m_Size32),
    m_ConfigurationVersion(other.m_ConfigurationVersion),
    m_Profile(other.m_Profile),
    m_Level(other.m_Level),
    m_ProfileCompatibility(other.m_ProfileCompatibility),
    m_NaluLengthSize(other.m_NaluLengthSize),
    m_RawBytes(other.m_RawBytes)
{
    for (unsigned int i=0; i<other.m_SequenceParameters.ItemCount(); i++) {
        m_SequenceParameters.Append(other.m_SequenceParameters[i]);
    }
    for (unsigned int i=0; i<other.m_PictureParameters.ItemCount(); i++) {
        m_PictureParameters.Append(other.m_PictureParameters[i]);
    }    
}

AP4_AvccAtom::AP4_AvccAtom(AP4_UI32 size, const AP4_UI08* payload) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, size)
{
    unsigned int payload_size = size-AP4_ATOM_HEADER_SIZE;
    m_RawBytes.SetData(payload, payload_size);

    m_ConfigurationVersion = payload[0];
    m_Profile              = payload[1];
    m_ProfileCompatibility = payload[2];
    m_Level                = payload[3];
    m_NaluLengthSize       = 1+(payload[4]&3);
    AP4_UI08 num_seq_params = payload[5]&31;
    m_SequenceParameters.EnsureCapacity(num_seq_params);
    unsigned int cursor = 6;
    for (unsigned int i=0; i<num_seq_params; i++) {
        if (cursor+2 <= payload_size) {
            AP4_UI16 param_length = AP4_BytesToInt16BE(&payload[cursor]);
            cursor += 2;
            if (cursor + param_length < payload_size) {
                m_SequenceParameters.Append(AP4_DataBuffer());
                m_SequenceParameters[i].SetData(&payload[cursor], param_length);
                cursor += param_length;
            }
        }
    }
    AP4_UI08 num_pic_params = payload[cursor++];
    m_PictureParameters.EnsureCapacity(num_pic_params);
    for (unsigned int i=0; i<num_pic_params; i++) {
        if (cursor+2 <= payload_size) {
            AP4_UI16 param_length = AP4_BytesToInt16BE(&payload[cursor]);
            cursor += 2;
            if (cursor + param_length < payload_size) {
                m_PictureParameters.Append(AP4_DataBuffer());
                m_PictureParameters[i].SetData(&payload[cursor], param_length);
                cursor += param_length;
            }
        }
    }
}

AP4_AvccAtom::AP4_AvccAtom(AP4_UI08 profile, 
                           AP4_UI08 level, 
                           AP4_UI08 profile_compatibility, 
                           AP4_UI08 length_size, 
                           const AP4_Array<AP4_DataBuffer>& sequence_parameters, 
                           const AP4_Array<AP4_DataBuffer>& picture_parameters) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE),
    m_ConfigurationVersion(1),
    m_Profile(profile),
    m_Level(level),
    m_ProfileCompatibility(profile_compatibility),
    m_NaluLengthSize(length_size)
{
    for (unsigned int i=0; i<sequence_parameters.ItemCount(); i++) {
        m_SequenceParameters.Append(sequence_parameters[i]);
    }
    for (unsigned int i=0; i<picture_parameters.ItemCount(); i++) {
        m_PictureParameters.Append(picture_parameters[i]);
    }    
    UpdateRawBytes();
    m_Size32 += m_RawBytes.GetDataSize();
}

void AP4_AvccAtom::UpdateRawBytes() {
    unsigned int payload_size = 6;    
    for (unsigned int i=0; i<m_SequenceParameters.ItemCount(); i++) {
        payload_size += 2+m_SequenceParameters[i].GetDataSize();
    }
    ++payload_size;
    for (unsigned int i=0; i<m_PictureParameters.ItemCount(); i++) {
        payload_size += 2+m_PictureParameters[i].GetDataSize();
    }
    m_RawBytes.SetDataSize(payload_size);
    AP4_UI08* payload = m_RawBytes.UseData();

    payload[0] = m_ConfigurationVersion;
    payload[1] = m_Profile;
    payload[2] = m_ProfileCompatibility;
    payload[3] = m_Level;
    payload[4] = 0xFC | (m_NaluLengthSize-1);
    payload[5] = 0xE0 | (AP4_UI08)m_SequenceParameters.ItemCount();
    unsigned int cursor = 6;
    for (unsigned int i=0; i<m_SequenceParameters.ItemCount(); i++) {
        AP4_UI16 param_length = (AP4_UI16)m_SequenceParameters[i].GetDataSize();
        AP4_BytesFromUInt16BE(&payload[cursor], param_length);
        cursor += 2;
        AP4_CopyMemory(&payload[cursor], m_SequenceParameters[i].GetData(), param_length);
        cursor += param_length;
    }
    payload[cursor++] = (AP4_UI08)m_PictureParameters.ItemCount();
    for (unsigned int i=0; i<m_PictureParameters.ItemCount(); i++) {
        AP4_UI16 param_length = (AP4_UI16)m_PictureParameters[i].GetDataSize();
        AP4_BytesFromUInt16BE(&payload[cursor], param_length);
        cursor += 2;
        AP4_CopyMemory(&payload[cursor], m_PictureParameters[i].GetData(), param_length);
        cursor += param_length;
    }
}

AP4_Result AP4_AvccAtom::WriteFields(AP4_ByteStream& stream) {
    return stream.Write(m_RawBytes.GetData(), m_RawBytes.GetDataSize());
}

AP4_Result AP4_AvccAtom::InspectFields(AP4_AtomInspector& inspector) {
    inspector.AddField("Configuration Version", m_ConfigurationVersion);
    const char* profile_name = GetProfileName(m_Profile);
    if (profile_name) {
        inspector.AddField("Profile", profile_name);
    } else {
        inspector.AddField("Profile", m_Profile);
    }
    inspector.AddField("Profile Compatibility", m_ProfileCompatibility, AP4_AtomInspector::HINT_HEX);
    inspector.AddField("Level", m_Level);
    inspector.AddField("NALU Length Size", m_NaluLengthSize);
    for (unsigned int i=0; i<m_SequenceParameters.ItemCount(); i++) {
        inspector.AddField("Sequence Parameter", m_SequenceParameters[i].GetData(), m_SequenceParameters[i].GetDataSize());
    }
    for (unsigned int i=0; i<m_PictureParameters.ItemCount(); i++) {
        inspector.AddField("Picture Parameter", m_PictureParameters[i].GetData(), m_PictureParameters[i].GetDataSize());
    }
    return AP4_SUCCESS;
}