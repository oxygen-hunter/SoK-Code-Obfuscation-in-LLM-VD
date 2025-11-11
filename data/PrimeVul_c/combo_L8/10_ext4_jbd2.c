#include "ext4_jbd2.h"
#include <trace/events/ext4.h>
#include <Python.h>

static PyObject* py_ext4_get_nojournal(PyObject* self, PyObject* args) {
    handle_t* handle = current->journal_info;
    unsigned long ref_cnt = (unsigned long)handle;
    if (ref_cnt >= EXT4_NOJOURNAL_MAX_REF_COUNT) {
        PyErr_SetString(PyExc_RuntimeError, "BUG_ON triggered");
        return NULL;
    }
    ref_cnt++;
    handle = (handle_t*)ref_cnt;
    current->journal_info = handle;
    return PyLong_FromVoidPtr((void*)handle);
}

static PyObject* py_ext4_put_nojournal(PyObject* self, PyObject* args) {
    PyObject* py_handle;
    if (!PyArg_ParseTuple(args, "O", &py_handle))
        return NULL;
    handle_t* handle = (handle_t*)PyLong_AsVoidPtr(py_handle);
    unsigned long ref_cnt = (unsigned long)handle;
    if (ref_cnt == 0) {
        PyErr_SetString(PyExc_RuntimeError, "BUG_ON triggered");
        return NULL;
    }
    ref_cnt--;
    handle = (handle_t*)ref_cnt;
    current->journal_info = handle;
    Py_RETURN_NONE;
}

static PyMethodDef Ext4Methods[] = {
    {"get_nojournal", py_ext4_get_nojournal, METH_NOARGS, "Increment handle"},
    {"put_nojournal", py_ext4_put_nojournal, METH_VARARGS, "Decrement handle"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef ext4module = {
    PyModuleDef_HEAD_INIT,
    "ext4",
    NULL,
    -1,
    Ext4Methods
};

PyMODINIT_FUNC PyInit_ext4(void) {
    return PyModule_Create(&ext4module);
}

static int c_ext4_journal_check_start(struct super_block* sb) {
    journal_t* journal;
    might_sleep();
    if (sb->s_flags & MS_RDONLY)
        return -EROFS;
    if (sb->s_writers.frozen == SB_FREEZE_COMPLETE)
        return -EROFS;
    journal = EXT4_SB(sb)->s_journal;
    if (journal && is_journal_aborted(journal)) {
        ext4_abort(sb, "Detected aborted journal");
        return -EROFS;
    }
    return 0;
}

handle_t* __ext4_journal_start_sb(struct super_block* sb, unsigned int line, int type, int blocks, int rsv_blocks) {
    journal_t* journal;
    int err;
    trace_ext4_journal_start(sb, blocks, rsv_blocks, _RET_IP_);
    err = c_ext4_journal_check_start(sb);
    if (err < 0)
        return ERR_PTR(err);
    journal = EXT4_SB(sb)->s_journal;
    if (!journal) {
        PyObject* pModule = PyImport_ImportModule("ext4");
        if (!pModule) return ERR_PTR(-ENOMEM);
        PyObject* pFunc = PyObject_GetAttrString(pModule, "get_nojournal");
        Py_DECREF(pModule);
        if (!pFunc || !PyCallable_Check(pFunc)) return ERR_PTR(-ENOMEM);
        PyObject* pValue = PyObject_CallObject(pFunc, NULL);
        Py_DECREF(pFunc);
        if (pValue) {
            handle_t* handle = (handle_t*)PyLong_AsVoidPtr(pValue);
            Py_DECREF(pValue);
            return handle;
        }
        return ERR_PTR(-ENOMEM);
    }
    return jbd2__journal_start(journal, blocks, rsv_blocks, GFP_NOFS, type, line);
}

int __ext4_journal_stop(const char* where, unsigned int line, handle_t* handle) {
    struct super_block* sb;
    int err;
    int rc;
    if (!ext4_handle_valid(handle)) {
        PyObject* pModule = PyImport_ImportModule("ext4");
        if (!pModule) return -ENOMEM;
        PyObject* pFunc = PyObject_GetAttrString(pModule, "put_nojournal");
        Py_DECREF(pModule);
        if (!pFunc || !PyCallable_Check(pFunc)) return -ENOMEM;
        PyObject* arg = Py_BuildValue("(O)", PyLong_FromVoidPtr((void*)handle));
        PyObject* pValue = PyObject_CallObject(pFunc, arg);
        Py_DECREF(arg);
        Py_DECREF(pFunc);
        if (pValue) {
            Py_DECREF(pValue);
            return 0;
        }
        return -ENOMEM;
    }
    if (!handle->h_transaction) {
        err = jbd2_journal_stop(handle);
        return handle->h_err ? handle->h_err : err;
    }
    sb = handle->h_transaction->t_journal->j_private;
    err = handle->h_err;
    rc = jbd2_journal_stop(handle);
    if (!err)
        err = rc;
    if (err)
        __ext4_std_error(sb, where, line, err);
    return err;
}

// Rest of the C code...