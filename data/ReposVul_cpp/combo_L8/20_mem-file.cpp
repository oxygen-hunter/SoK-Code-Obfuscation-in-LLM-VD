extern "C" {
#include <Python.h>
#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"
}

namespace HPHP {

struct PyMemFile {
  PyObject_HEAD
  MemFile* memFile;
};

static PyObject* PyMemFile_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
  PyMemFile* self;
  self = (PyMemFile*)type->tp_alloc(type, 0);
  if (self != nullptr) {
    self->memFile = nullptr;
  }
  return (PyObject*)self;
}

static int PyMemFile_init(PyMemFile* self, PyObject* args, PyObject* kwds) {
  const char* wrapper;
  const char* stream;
  if (!PyArg_ParseTuple(args, "ss", &wrapper, &stream)) {
    return -1;
  }
  self->memFile = new MemFile(wrapper, stream);
  return 0;
}

static void PyMemFile_dealloc(PyMemFile* self) {
  delete self->memFile;
  Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyMethodDef PyMemFile_methods[] = {
  {nullptr}  /* Sentinel */
};

static PyTypeObject PyMemFileType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "hphp.PyMemFile",             /* tp_name */
  sizeof(PyMemFile), /* tp_basicsize */
  0,                         /* tp_itemsize */
  (destructor)PyMemFile_dealloc, /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_reserved */
  0,                         /* tp_repr */
  0,                         /* tp_as_number */
  0,                         /* tp_as_sequence */
  0,                         /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,                         /* tp_str */
  0,                         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,        /* tp_flags */
  "MemFile objects",           /* tp_doc */
  0,		               /* tp_traverse */
  0,		               /* tp_clear */
  0,		               /* tp_richcompare */
  0,		               /* tp_weaklistoffset */
  0,		               /* tp_iter */
  0,		               /* tp_iternext */
  PyMemFile_methods,             /* tp_methods */
  0,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)PyMemFile_init,      /* tp_init */
  0,                         /* tp_alloc */
  PyMemFile_new,                 /* tp_new */
};

static PyModuleDef hphpmodule = {
  PyModuleDef_HEAD_INIT,
  "hphp",
  nullptr,
  -1,
  nullptr, nullptr, nullptr, nullptr, nullptr
};

PyMODINIT_FUNC PyInit_hphp(void) {
  PyObject* m;

  if (PyType_Ready(&PyMemFileType) < 0)
    return nullptr;

  m = PyModule_Create(&hphpmodule);
  if (m == nullptr)
    return nullptr;

  Py_INCREF(&PyMemFileType);
  PyModule_AddObject(m, "PyMemFile", (PyObject*)&PyMemFileType);
  return m;
}

///////////////////////////////////////////////////////////////////////////////
}