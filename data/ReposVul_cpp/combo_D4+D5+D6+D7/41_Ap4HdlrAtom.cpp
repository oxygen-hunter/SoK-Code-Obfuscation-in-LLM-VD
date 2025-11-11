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
AP4_HdlrAtom::Create(AP4_Size sz, AP4_ByteStream& s)
{
    std::vector<AP4_UI32> vf;
    AP4_UI08 v;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(s, v, vf.emplace_back(0)))) return NULL;
    if (v != 0) return NULL;
    return new AP4_HdlrAtom(sz, v, vf[0], s);
}

/*----------------------------------------------------------------------
|   AP4_HdlrAtom::AP4_HdlrAtom
+---------------------------------------------------------------------*/
AP4_HdlrAtom::AP4_HdlrAtom(AP4_Atom::Type t, const char* n) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_HandlerName(n),
    m_HandlerType(t)
{
    m_Size32 += 20+m_HandlerName.GetLength()+1;
    m_Reserved[0] = m_Reserved[1] = m_Reserved[2] = 0;
}

/*----------------------------------------------------------------------
|   AP4_HdlrAtom::AP4_HdlrAtom
+---------------------------------------------------------------------*/
AP4_HdlrAtom::AP4_HdlrAtom(AP4_UI32 sz, 
                           AP4_UI08 v,
                           AP4_UI32 f,
                           AP4_ByteStream& s) :
    AP4_Atom(AP4_ATOM_TYPE_HDLR, sz, v, f)
{
    std::vector<AP4_UI32> r(3);
    AP4_UI32 p;
    s.ReadUI32(p);
    s.ReadUI32(m_HandlerType);
    s.ReadUI32(r[0]);
    s.ReadUI32(r[1]);
    s.ReadUI32(r[2]);
    
    if (sz < AP4_FULL_ATOM_HEADER_SIZE+20) return;
    AP4_UI32 ns = sz-(AP4_FULL_ATOM_HEADER_SIZE+20);
    char* n = new char[ns+1];
    if (n == NULL) return;
    s.Read(n, ns);
    n[ns] = '\0';
    if (n[0] == ns-1) {
        m_HandlerName = n+1;
    } else {
        m_HandlerName = n;
    }
    delete[] n;
    m_Reserved[0] = r[0];
    m_Reserved[1] = r[1];
    m_Reserved[2] = r[2];
}

/*----------------------------------------------------------------------
|   AP4_HdlrAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_HdlrAtom::WriteFields(AP4_ByteStream& s)
{
    AP4_Result r;
    r = s.WriteUI32(0);
    if (AP4_FAILED(r)) return r;
    r = s.WriteUI32(m_HandlerType);
    if (AP4_FAILED(r)) return r;
    std::vector<AP4_UI32> res(m_Reserved, m_Reserved + 3);
    for (auto x : res) {
        r = s.WriteUI32(x);
        if (AP4_FAILED(r)) return r;
    }
    AP4_UI08 ns = (AP4_UI08)m_HandlerName.GetLength();
    if (AP4_FULL_ATOM_HEADER_SIZE+20+ns > m_Size32) {
        ns = (AP4_UI08)(m_Size32-AP4_FULL_ATOM_HEADER_SIZE+20);
    }
    if (ns) {
        r = s.Write(m_HandlerName.GetChars(), ns);
        if (AP4_FAILED(r)) return r;
    }
    AP4_Size p = m_Size32-(AP4_FULL_ATOM_HEADER_SIZE+20+ns);
    while (p--) s.WriteUI08(0);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_HdlrAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_HdlrAtom::InspectFields(AP4_AtomInspector& i)
{
    char t[5];
    AP4_FormatFourChars(t, m_HandlerType);
    i.AddField("handler_type", t);
    i.AddField("handler_name", m_HandlerName.GetChars());

    return AP4_SUCCESS;
}