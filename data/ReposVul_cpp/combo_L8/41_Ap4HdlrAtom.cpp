#include <Python.h>
#include "Ap4HdlrAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_HdlrAtom)

AP4_HdlrAtom*
AP4_HdlrAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    Py_Initialize();
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;
    int version = 0;
    int flags = 0;

    pName = PyUnicode_DecodeFSDefault("header_reader");
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, "read_full_header");
        if (PyCallable_Check(pFunc)) {
            pArgs = PyTuple_Pack(2, PyLong_FromLong(size), PyLong_FromLong(stream.GetSize()));
            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if (pValue != NULL) {
                version = (int)PyLong_AsLong(PyTuple_GetItem(pValue, 0));
                flags = (int)PyLong_AsLong(PyTuple_GetItem(pValue, 1));
                Py_DECREF(pValue);
            }
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    Py_Finalize();

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
    if (name[0] == name_size-1) {
        m_HandlerName = name+1;
    } else {
        m_HandlerName = name;
    }
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
    if (AP4_FULL_ATOM_HEADER_SIZE+20+name_size > m_Size32) {
        name_size = (AP4_UI08)(m_Size32-AP4_FULL_ATOM_HEADER_SIZE+20);
    }
    if (name_size) {
        result = stream.Write(m_HandlerName.GetChars(), name_size);
        if (AP4_FAILED(result)) return result;
    }

    AP4_Size padding = m_Size32-(AP4_FULL_ATOM_HEADER_SIZE+20+name_size);
    while (padding--) stream.WriteUI08(0);

    return AP4_SUCCESS;
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
```

Python file (header_reader.py):

```python
def read_full_header(size, stream_size):
    # Simulates reading header from the stream
    version = 0
    flags = 0
    # Perform some operation to determine version and flags
    return version, flags