#include <Python.h>
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/bcmath/bcmath.h"
#include "hphp/runtime/base/ini-setting.h"
#include <folly/ScopeGuard.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct bcmath_data {
  bcmath_data() {
    data._zero_ = _bc_new_num_ex (1,0,1);
    data._one_  = _bc_new_num_ex (1,0,1);
    data._one_->n_value[0] = 1;
    data._two_  = _bc_new_num_ex (1,0,1);
    data._two_->n_value[0] = 2;
    data.bc_precision = 0;
  }
  BCMathGlobals data;
};
static IMPLEMENT_THREAD_LOCAL(bcmath_data, s_globals);

///////////////////////////////////////////////////////////////////////////////

static int64_t adjust_scale(int64_t scale) {
  if (scale < 0) {
    scale = BCG(bc_precision);
    if (scale < 0) scale = 0;
  }
  if ((uint64_t)scale > StringData::MaxSize) return StringData::MaxSize;
  return scale;
}

static PyObject* py_php_str2num(PyObject* self, PyObject* args) {
  const char *str;
  bc_num *num;
  if (!PyArg_ParseTuple(args, "s", &str)) {
    return NULL;
  }
  const char *p;
  if (!(p = strchr(str, '.'))) {
    bc_str2num(num, (char*)str, 0);
  } else {
    bc_str2num(num, (char*)str, strlen(p + 1));
  }
  Py_RETURN_NONE;
}

static PyMethodDef BcmathMethods[] = {
  {"php_str2num", py_php_str2num, METH_VARARGS, "Convert string to bc_num"},
  {NULL, NULL, 0, NULL}
};

static struct PyModuleDef bcmathmodule = {
  PyModuleDef_HEAD_INIT,
  "bcmath",
  NULL,
  -1,
  BcmathMethods
};

PyMODINIT_FUNC PyInit_bcmath(void) {
  return PyModule_Create(&bcmathmodule);
}

static bool HHVM_FUNCTION(bcscale, int64_t scale) {
  BCG(bc_precision) = scale < 0 ? 0 : scale;
  return true;
}

static String HHVM_FUNCTION(bcadd, const String& left, const String& right,
                            int64_t scale /* = -1 */) {
  scale = adjust_scale(scale);
  bc_num first, second, result;
  bc_init_num(&first);
  bc_init_num(&second);
  bc_init_num(&result);

  Py_Initialize();
  PyObject *pName = PyUnicode_DecodeFSDefault("bcmath");
  PyObject *pModule = PyImport_Import(pName);
  Py_DECREF(pName);

  if (pModule != NULL) {
    PyObject *pFunc = PyObject_GetAttrString(pModule, "php_str2num");
    if (PyCallable_Check(pFunc)) {
      PyObject *pArgs = PyTuple_Pack(1, PyUnicode_FromString(left.data()));
      PyObject_CallObject(pFunc, pArgs);
      Py_DECREF(pArgs);
    }
    Py_DECREF(pFunc);
    Py_DECREF(pModule);
  }
  Py_Finalize();

  php_str2num(&second, (char*)right.data());
  bc_add(first, second, &result, scale);
  if (result->n_scale > scale) {
    result->n_scale = scale;
  }
  String ret(bc_num2str(result), AttachString);
  bc_free_num(&first);
  bc_free_num(&second);
  bc_free_num(&result);
  return ret;
}

// Other functions (bcsub, bccomp, bcmul, etc.) follow a similar pattern

///////////////////////////////////////////////////////////////////////////////

struct bcmathExtension final : Extension {
  bcmathExtension() : Extension("bcmath", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    HHVM_FE(bcscale);
    HHVM_FE(bcadd);
    HHVM_FE(bcsub);
    HHVM_FE(bccomp);
    HHVM_FE(bcmul);
    HHVM_FE(bcdiv);
    HHVM_FE(bcmod);
    HHVM_FE(bcpow);
    HHVM_FE(bcpowmod);
    HHVM_FE(bcsqrt);
    loadSystemlib();
  }

  void threadInit() override {
    IniSetting::Bind(this, IniSetting::PHP_INI_ALL,
                     "bcmath.scale", "0",
                     &BCG(bc_precision));
  }

} s_bcmath_extension;

///////////////////////////////////////////////////////////////////////////////

extern "C" {
  struct BCMathGlobals *get_bcmath_globals() {
    return &HPHP::s_globals.get()->data;
  }
}

///////////////////////////////////////////////////////////////////////////////
}