#include <Python.h>
#include <iostream>
#include <cstring>
#include <cassert>
#include <climits>

namespace HPHP {

class MemFile {
public:
    MemFile(const std::string& wrapper, const std::string& stream)
        : m_data(nullptr), m_len(-1), m_cursor(0), m_malloced(false) {
        setIsLocal(true);
    }

    MemFile(const char *data, int64_t len, const std::string& wrapper, const std::string& stream)
        : m_data(nullptr), m_len(len), m_cursor(0), m_malloced(true) {
        m_data = (char*)malloc(len + 1);
        if (m_data && len) {
            memcpy(m_data, data, len);
        }
        m_data[len] = '\0';
        setIsLocal(true);
    }

    ~MemFile() {
        close();
    }

    void sweep() {
        close();
    }

    bool open(const std::string& filename, const std::string& mode) {
        assert(m_len == -1);
        const char* mode_str = mode.c_str();
        if (strchr(mode_str, '+') || strchr(mode_str, 'a') || strchr(mode_str, 'w')) {
            return false;
        }
        int len = INT_MIN;
        bool compressed = false;
        char *data = nullptr;
        Py_Initialize();
        PyObject *pName = PyUnicode_DecodeFSDefault("file_cache");
        PyObject *pModule = PyImport_Import(pName);
        Py_DECREF(pName);
        if (pModule != nullptr) {
            PyObject *pFunc = PyObject_GetAttrString(pModule, "read");
            if (PyCallable_Check(pFunc)) {
                PyObject *pArgs = PyTuple_Pack(1, PyUnicode_FromString(filename.c_str()));
                PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
                Py_DECREF(pArgs);
                if (pValue != nullptr) {
                    len = (int)PyLong_AsLong(PyTuple_GetItem(pValue, 1));
                    compressed = PyObject_IsTrue(PyTuple_GetItem(pValue, 2));
                    data = strdup(PyUnicode_AsUTF8(PyTuple_GetItem(pValue, 0)));
                    Py_DECREF(pValue);
                }
            }
            Py_DECREF(pFunc);
        }
        Py_FinalizeEx();
        if (len != INT_MIN && len != -1 && len != -2) {
            assert(len >= 0);
            if (compressed) {
                data = gzdecode(data, len);
                if (data == nullptr) {
                    raise_fatal_error("cannot unzip compressed data");
                }
                m_data = data;
                m_malloced = true;
                m_len = len;
                return true;
            }
            m_data = data;
            m_len = len;
            return true;
        }
        if (len != INT_MIN) {
            std::cerr << "Cannot open a PHP file or a directory as MemFile: " << filename << std::endl;
        }
        return false;
    }

    bool close() {
        return closeImpl();
    }

private:
    char* m_data;
    int64_t m_len;
    int64_t m_cursor;
    bool m_malloced;

    void setIsLocal(bool isLocal) {}

    bool closeImpl() {
        if (m_malloced && m_data) {
            free(m_data);
            m_data = nullptr;
        }
        return true;
    }

    void raise_fatal_error(const char* msg) {
        std::cerr << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }

    char* gzdecode(const char* data, int& len) {
        return const_cast<char*>(data); // Dummy implementation
    }
};

}