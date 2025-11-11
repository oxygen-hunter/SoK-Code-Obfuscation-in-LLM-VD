#include <Python.h>
#include <stdio.h>

static PyObject* c_module_ldsem_atomic_update(PyObject* self, PyObject* args) {
    long delta;
    PyObject* py_sem;
    if (!PyArg_ParseTuple(args, "lO", &delta, &py_sem))
        return NULL;

    PyObject* py_count = PyObject_GetAttrString(py_sem, "count");
    if (!py_count)
        return NULL;

    long count = PyLong_AsLong(py_count);
    count += delta;
    PyObject_SetAttrString(py_sem, "count", PyLong_FromLong(count));

    Py_DECREF(py_count);

    return Py_BuildValue("l", count);
}

static PyObject* c_module_ldsem_cmpxchg(PyObject* self, PyObject* args) {
    long old_val, new_val;
    PyObject* py_sem;
    if (!PyArg_ParseTuple(args, "llO", &old_val, &new_val, &py_sem))
        return NULL;

    PyObject* py_count = PyObject_GetAttrString(py_sem, "count");
    if (!py_count)
        return NULL;

    long count = PyLong_AsLong(py_count);
    if (count == old_val) {
        PyObject_SetAttrString(py_sem, "count", PyLong_FromLong(new_val));
        old_val = new_val; 
    }

    Py_DECREF(py_count);

    return Py_BuildValue("i", (old_val == count));
}

static PyMethodDef LdsemMethods[] = {
    {"atomic_update", c_module_ldsem_atomic_update, METH_VARARGS, "Update the semaphore atomically"},
    {"cmpxchg", c_module_ldsem_cmpxchg, METH_VARARGS, "Perform compare and exchange"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef ldsemmodule = {
    PyModuleDef_HEAD_INIT,
    "ldsem",
    NULL,
    -1,
    LdsemMethods
};

PyMODINIT_FUNC PyInit_ldsem(void) {
    return PyModule_Create(&ldsemmodule);
}