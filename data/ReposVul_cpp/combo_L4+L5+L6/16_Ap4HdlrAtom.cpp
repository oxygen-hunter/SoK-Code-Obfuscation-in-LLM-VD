#include "Ap4HdlrAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_HdlrAtom)

AP4_HdlrAtom*
AP4_HdlrAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI08 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_HdlrAtom(size, version, flags, stream);
}

AP4_HdlrAtom::AP4_HdlrAtom(AP4_Atom::Type hdlr_type, const char* hdlr_name) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_HandlerType(hdlr_type),
    m_HandlerName(hdlr_name)
{
    m_Size32 += 20+m_HandlerName.GetLength()+1;
    m_Reserved[0] = m_Reserved[1] = m_Reserved[2] = 0;
}

AP4_HdlrAtom::AP4_HdlrAtom(AP4_UI32        size, 
                           AP4_UI08        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, size, version, flags)
{
    AP4_UI32 predefined;
    stream.ReadUI32(predefined);
    stream.ReadUI32(m_HandlerType);
    stream.ReadUI32(m_Reserved[0]);
    stream.ReadUI32(m_Reserved[1]);
    stream.ReadUI32(m_Reserved[2]);
    
    if (size < AP4_FULL_ATOM_HEADER_SIZE+20) return;
    AP4_UI32 name_size = size-(AP4_FULL_ATOM_HEADER_SIZE+20);
    char* name = new char[name_size+1];
    if (name == NULL) return;
    stream.Read(name, name_size);
    name[name_size] = '\0';
    m_HandlerName = ((AP4_UI08)name[0] == (AP4_UI08)(name_size-1)) ? name+1 : name;
    delete[] name;
}

AP4_Result
AP4_HdlrAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;
    result = stream.WriteUI32(0);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_HandlerType);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_Reserved[0]);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_Reserved[1]);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_Reserved[2]);
    if (AP4_FAILED(result)) return result;
    AP4_UI08 name_size = (AP4_UI08)m_HandlerName.GetLength();
    name_size = (AP4_FULL_ATOM_HEADER_SIZE+20+name_size > m_Size32) ? (AP4_UI08)(m_Size32-AP4_FULL_ATOM_HEADER_SIZE+20) : name_size;
    if (name_size) {
        result = stream.Write(m_HandlerName.GetChars(), name_size);
        if (AP4_FAILED(result)) return result;
    }
    
    AP4_Size padding = m_Size32-(AP4_FULL_ATOM_HEADER_SIZE+20+name_size);
    if (padding > 0) WritePadding(stream, padding);

    return AP4_SUCCESS;
}

void AP4_HdlrAtom::WritePadding(AP4_ByteStream& stream, AP4_Size padding) {
    if (padding == 0) return;
    stream.WriteUI08(0);
    WritePadding(stream, padding - 1);
}

AP4_Result
AP4_HdlrAtom::InspectFields(AP4_AtomInspector& inspector)
{
    char type[5];
    AP4_FormatFourChars(type, m_HandlerType);
    inspector.AddField("handler_type", type);
    inspector.AddField("handler_name", m_HandlerName.GetChars());

    return AP4_SUCCESS;
}