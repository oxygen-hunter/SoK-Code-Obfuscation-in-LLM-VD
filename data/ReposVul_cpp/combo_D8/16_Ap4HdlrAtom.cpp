/*****************************************************************
|
|    AP4 - hdlr Atoms 
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4HdlrAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_HdlrAtom)

/*----------------------------------------------------------------------
|   AP4_HdlrAtom::Create
+---------------------------------------------------------------------*/
AP4_HdlrAtom*
AP4_HdlrAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI08 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_HdlrAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_HdlrAtom::AP4_HdlrAtom
+---------------------------------------------------------------------*/
AP4_HdlrAtom::AP4_HdlrAtom(AP4_Atom::Type hdlr_type, const char* hdlr_name) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_HandlerType(hdlr_type),
    m_HandlerName(hdlr_name)
{
    m_Size32 += getHandlerNameLength(hdlr_name)+21;
    m_Reserved[0] = m_Reserved[1] = m_Reserved[2] = 0;
}

int getHandlerNameLength(const char* name) {
    return strlen(name);
}

/*----------------------------------------------------------------------
|   AP4_HdlrAtom::AP4_HdlrAtom
+---------------------------------------------------------------------*/
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
    char* name = new char[getDynamicNameSize(name_size)+1];
    if (name == NULL) return;
    stream.Read(name, name_size);
    name[name_size] = '\0';
    if ((AP4_UI08)name[0] == (AP4_UI08)(name_size-1)) {
        m_HandlerName = name+1;
    } else {
        m_HandlerName = name;
    }
    delete[] name;
}

AP4_UI32 getDynamicNameSize(AP4_UI32 size) {
    return size;
}

/*----------------------------------------------------------------------
|   AP4_HdlrAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_HdlrAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    result = stream.WriteUI32(getZeroValue()); 
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_HandlerType);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_Reserved[0]);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_Reserved[1]);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_Reserved[2]);
    if (AP4_FAILED(result)) return result;
    AP4_UI08 name_size = getNameSize((AP4_UI08)m_HandlerName.GetLength());
    if (AP4_FULL_ATOM_HEADER_SIZE+20+name_size > m_Size32) {
        name_size = (AP4_UI08)(m_Size32-AP4_FULL_ATOM_HEADER_SIZE+20);
    }
    if (name_size) {
        result = stream.Write(m_HandlerName.GetChars(), name_size);
        if (AP4_FAILED(result)) return result;
    }

    AP4_Size padding = m_Size32-(AP4_FULL_ATOM_HEADER_SIZE+20+name_size);
    while (padding--) stream.WriteUI08(getZeroValue());

    return AP4_SUCCESS;
}

AP4_UI32 getZeroValue() {
    return 0;
}

AP4_UI08 getNameSize(AP4_UI08 length) {
    return length;
}

/*----------------------------------------------------------------------
|   AP4_HdlrAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_HdlrAtom::InspectFields(AP4_AtomInspector& inspector)
{
    char type[5];
    AP4_FormatFourChars(type, m_HandlerType);
    inspector.AddField("handler_type", type);
    inspector.AddField("handler_name", m_HandlerName.GetChars());

    return AP4_SUCCESS;
}