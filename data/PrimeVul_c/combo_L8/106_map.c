#include <Python.h>
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

static PyObject* py_read_map_image(PyObject *self, PyObject *args)
{
    const ImageInfo *image_info;
    ExceptionInfo *exception;
    if (!PyArg_ParseTuple(args, "OO", &image_info, &exception))
        return NULL;
    return PyLong_FromVoidPtr(ReadMAPImage(image_info, exception));
}

static PyObject* py_register_map_image(PyObject *self, PyObject *args)
{
    return PyLong_FromSize_t(RegisterMAPImage());
}

static PyObject* py_unregister_map_image(PyObject *self, PyObject *args)
{
    UnregisterMAPImage();
    Py_RETURN_NONE;
}

static PyObject* py_write_map_image(PyObject *self, PyObject *args)
{
    const ImageInfo *image_info;
    Image *image;
    ExceptionInfo *exception;
    if (!PyArg_ParseTuple(args, "OOO", &image_info, &image, &exception))
        return NULL;
    return PyBool_FromLong(WriteMAPImage(image_info, image, exception));
}

static PyMethodDef MapMethods[] = {
    {"read_map_image", py_read_map_image, METH_VARARGS, "Read MAP image"},
    {"register_map_image", py_register_map_image, METH_NOARGS, "Register MAP image"},
    {"unregister_map_image", py_unregister_map_image, METH_NOARGS, "Unregister MAP image"},
    {"write_map_image", py_write_map_image, METH_VARARGS, "Write MAP image"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mapmodule = {
    PyModuleDef_HEAD_INIT,
    "map",
    NULL,
    -1,
    MapMethods
};

PyMODINIT_FUNC PyInit_map(void)
{
    return PyModule_Create(&mapmodule);
}