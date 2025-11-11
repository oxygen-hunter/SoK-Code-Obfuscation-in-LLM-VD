#include "itemstackmetadata.h"
#include "util/serialize.h"
#include "util/strfnd.h"
#include <algorithm>
#include <Python.h>

#define DEZ_START '\x01'
#define DEZ_KV_DELIM '\x02'
#define DEZ_PAIR_DELIM '\x03'
#define DEZ_START_STR "\x01"
#define DEZ_KV_DELIM_STR "\x02"
#define DEZ_PAIR_DELIM_STR "\x03"

#define TOOLCAP_KEY "tool_capabilities"

extern "C" {
    static PyObject* sanitize_string(PyObject* self, PyObject* args) {
        const char* input;
        if (!PyArg_ParseTuple(args, "s", &input))
            return NULL;
        std::string str(input);
        str.erase(std::remove(str.begin(), str.end(), DEZ_START), str.end());
        str.erase(std::remove(str.begin(), str.end(), DEZ_KV_DELIM), str.end());
        str.erase(std::remove(str.begin(), str.end(), DEZ_PAIR_DELIM), str.end());
        return Py_BuildValue("s", str.c_str());
    }

    static PyMethodDef SanitizeMethods[] = {
        {"sanitize_string", sanitize_string, METH_VARARGS, "Sanitize a string"},
        {NULL, NULL, 0, NULL}
    };

    static struct PyModuleDef sanitizemodule = {
        PyModuleDef_HEAD_INIT,
        "sanitize",
        NULL,
        -1,
        SanitizeMethods
    };

    PyMODINIT_FUNC PyInit_sanitize(void) {
        return PyModule_Create(&sanitizemodule);
    }
}

void ItemStackMetadata::clear()
{
	Metadata::clear();
	updateToolCapabilities();
}

bool ItemStackMetadata::setString(const std::string &name, const std::string &var)
{
	Py_Initialize();
	PyObject* pName = PyUnicode_DecodeFSDefault("sanitize");
	PyObject* pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	PyObject* pFunc = PyObject_GetAttrString(pModule, "sanitize_string");
	PyObject* pArgs = PyTuple_Pack(1, PyUnicode_FromString(name.c_str()));
	PyObject* pValue = PyObject_CallObject(pFunc, pArgs);
	std::string clean_name = PyUnicode_AsUTF8(pValue);
	Py_DECREF(pArgs);
	Py_DECREF(pValue);

	pArgs = PyTuple_Pack(1, PyUnicode_FromString(var.c_str()));
	pValue = PyObject_CallObject(pFunc, pArgs);
	std::string clean_var = PyUnicode_AsUTF8(pValue);
	Py_DECREF(pArgs);
	Py_DECREF(pValue);

	Py_XDECREF(pFunc);
	Py_DECREF(pModule);

	bool result = Metadata::setString(clean_name, clean_var);
	if (clean_name == TOOLCAP_KEY)
		updateToolCapabilities();
	
	Py_Finalize();
	return result;
}

void ItemStackMetadata::serialize(std::ostream &os) const
{
	std::ostringstream os2;
	os2 << DEZ_START;
	for (const auto &stringvar : m_stringvars) {
		if (!stringvar.first.empty() || !stringvar.second.empty())
			os2 << stringvar.first << DEZ_KV_DELIM
				<< stringvar.second << DEZ_PAIR_DELIM;
	}
	os << serializeJsonStringIfNeeded(os2.str());
}

void ItemStackMetadata::deSerialize(std::istream &is)
{
	std::string in = deSerializeJsonStringIfNeeded(is);

	m_stringvars.clear();

	if (!in.empty()) {
		if (in[0] == DEZ_START) {
			Strfnd fnd(in);
			fnd.to(1);
			while (!fnd.at_end()) {
				std::string name = fnd.next(DEZ_KV_DELIM_STR);
				std::string var  = fnd.next(DEZ_PAIR_DELIM_STR);
				m_stringvars[name] = var;
			}
		} else {
			m_stringvars[""] = in;
		}
	}
	updateToolCapabilities();
}

void ItemStackMetadata::updateToolCapabilities()
{
	if (contains(TOOLCAP_KEY)) {
		toolcaps_overridden = true;
		toolcaps_override = ToolCapabilities();
		std::istringstream is(getString(TOOLCAP_KEY));
		toolcaps_override.deserializeJson(is);
	} else {
		toolcaps_overridden = false;
	}
}

void ItemStackMetadata::setToolCapabilities(const ToolCapabilities &caps)
{
	std::ostringstream os;
	caps.serializeJson(os);
	setString(TOOLCAP_KEY, os.str());
}

void ItemStackMetadata::clearToolCapabilities()
{
	setString(TOOLCAP_KEY, "");
}