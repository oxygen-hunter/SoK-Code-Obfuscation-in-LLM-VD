#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <python3.8/Python.h>

#define EVMKEY "evm-key"
#define MAX_KEY_SIZE 128

static unsigned char evmkey[MAX_KEY_SIZE];
static int evmkey_len = MAX_KEY_SIZE;

void *crypto_lib;
typedef void (*crypto_function)(void);

void initialize_python() {
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('.')");
}

void finalize_python() {
    Py_Finalize();
}

void load_crypto_library() {
    crypto_lib = dlopen("libcrypto.so", RTLD_LAZY);
    if (!crypto_lib) {
        fprintf(stderr, "Error loading library: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
}

void unload_crypto_library() {
    if (crypto_lib) {
        dlclose(crypto_lib);
    }
}

void call_crypto_function(const char *func_name) {
    crypto_function func = (crypto_function)dlsym(crypto_lib, func_name);
    if (func) {
        func();
    } else {
        fprintf(stderr, "Function %s not found in library\n", func_name);
    }
}

void evm_init_key() {
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;

    load_crypto_library();
    initialize_python();

    pName = PyUnicode_DecodeFSDefault("evm_key_util");
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, "fetch_key");
        if (PyCallable_Check(pFunc)) {
            pArgs = PyTuple_Pack(1, PyUnicode_FromString(EVMKEY));
            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);
            if (pValue != NULL) {
                memcpy(evmkey, PyBytes_AsString(pValue), PyBytes_Size(pValue));
                Py_DECREF(pValue);
            } else {
                PyErr_Print();
            }
        } else {
            PyErr_Print();
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    } else {
        PyErr_Print();
    }

    finalize_python();
    unload_crypto_library();
}

int main() {
    evm_init_key();
    return 0;
}