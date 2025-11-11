#include "Ap4HdlrAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_HdlrAtom)

AP4_HdlrAtom*
AP4_HdlrAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI08 ver;
    AP4_UI32 flgs;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, ver, flgs))) return NULL;
    if (ver != 0) return NULL;
    return new AP4_HdlrAtom(size, ver, flgs, stream);
}

AP4_HdlrAtom::AP4_HdlrAtom(AP4_Atom::Type typ, const char* nm) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_HandlerType(typ),
    m_HandlerName(nm)
{
    m_Size32 += 20+m_HandlerName.GetLength()+1;
    m_Reserved[0] = m_Reserved[1] = m_Reserved[2] = 0;
}

AP4_HdlrAtom::AP4_HdlrAtom(AP4_UI32 sz, 
                           AP4_UI08 ver,
                           AP4_UI32 flgs,
                           AP4_ByteStream& strm) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, sz, ver, flgs)
{
    AP4_UI32 prdfnd;
    strm.ReadUI32(prdfnd);
    strm.ReadUI32(m_HandlerType);
    strm.ReadUI32(m_Reserved[0]);
    strm.ReadUI32(m_Reserved[1]);
    strm.ReadUI32(m_Reserved[2]);
    
    if (sz < AP4_FULL_ATOM_HEADER_SIZE+20) return;
    AP4_UI32 nm_sz = sz-(AP4_FULL_ATOM_HEADER_SIZE+20);
    char* nm = new char[nm_sz+1];
    if (nm == NULL) return;
    strm.Read(nm, nm_sz);
    nm[nm_sz] = '\0';
    if (nm[0] == nm_sz-1) {
        m_HandlerName = nm+1;
    } else {
        m_HandlerName = nm;
    }
    delete[] nm;
}

AP4_Result
AP4_HdlrAtom::WriteFields(AP4_ByteStream& strm)
{
    AP4_Result rslt;

    rslt = strm.WriteUI32(0);
    if (AP4_FAILED(rslt)) return rslt;
    rslt = strm.WriteUI32(m_HandlerType);
    if (AP4_FAILED(rslt)) return rslt;
    rslt = strm.WriteUI32(m_Reserved[0]);
    if (AP4_FAILED(rslt)) return rslt;
    rslt = strm.WriteUI32(m_Reserved[1]);
    if (AP4_FAILED(rslt)) return rslt;
    rslt = strm.WriteUI32(m_Reserved[2]);
    if (AP4_FAILED(rslt)) return rslt;
    AP4_UI08 nm_sz = (AP4_UI08)m_HandlerName.GetLength();
    if (AP4_FULL_ATOM_HEADER_SIZE+20+nm_sz > m_Size32) {
        nm_sz = (AP4_UI08)(m_Size32-AP4_FULL_ATOM_HEADER_SIZE+20);
    }
    if (nm_sz) {
        rslt = strm.Write(m_HandlerName.GetChars(), nm_sz);
        if (AP4_FAILED(rslt)) return rslt;
    }

    AP4_Size paddng = m_Size32-(AP4_FULL_ATOM_HEADER_SIZE+20+nm_sz);
    while (paddng--) strm.WriteUI08(0);

    return AP4_SUCCESS;
}

AP4_Result
AP4_HdlrAtom::InspectFields(AP4_AtomInspector& inspctr)
{
    char typ[5];
    AP4_FormatFourChars(typ, m_HandlerType);
    inspctr.AddField("handler_type", typ);
    inspctr.AddField("handler_name", m_HandlerName.GetChars());

    return AP4_SUCCESS;
}