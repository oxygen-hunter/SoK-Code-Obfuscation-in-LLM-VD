////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//                Copyright (c) 1998 - 2019 David Bryant.                 //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// wave64.c

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>

#include "wavpack.h"
#include "utils.h"
#include "md5.h"

typedef struct {
    char ckID [16];
    int64_t ckSize;
    char formType [16];
} Wave64FileHeader;

typedef struct {
    char ckID [16];
    int64_t ckSize;
} Wave64ChunkHeader;

#define Wave64ChunkHeaderFormat "88D"

static const unsigned char riff_guid [16] = { 'r','i','f','f', 0x2e,0x91,0xcf,0x11,0xa5,0xd6,0x28,0xdb,0x04,0xc1,0x00,0x00 };
static const unsigned char wave_guid [16] = { 'w','a','v','e', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };
static const unsigned char  fmt_guid [16] = { 'f','m','t',' ', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };
static const unsigned char data_guid [16] = { 'd','a','t','a', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };

#define WAVPACK_NO_ERROR    0
#define WAVPACK_SOFT_ERROR  1
#define WAVPACK_HARD_ERROR  2

extern int debug_logging_mode;

extern "C" {
    int ParseWave64HeaderConfig (FILE *infile, char *infilename, char *fourcc, WavpackContext *wpc, WavpackConfig *config);
    int WriteWave64Header (FILE *outfile, WavpackContext *wpc, int64_t total_samples, int qmode);
}

#include <Python.h>

PyObject* py_ParseWave64HeaderConfig(PyObject* self, PyObject* args) {
    FILE *infile;
    char *infilename, *fourcc;
    WavpackContext *wpc;
    WavpackConfig *config;
    int result;

    if (!PyArg_ParseTuple(args, "sssOO", &infilename, &fourcc, &wpc, &config))
        return NULL;

    infile = fopen(infilename, "rb");

    if (!infile) {
        PyErr_SetString(PyExc_FileNotFoundError, "Cannot open input file");
        return NULL;
    }

    result = ParseWave64HeaderConfig(infile, infilename, fourcc, wpc, config);
    fclose(infile);

    return PyLong_FromLong(result);
}

PyObject* py_WriteWave64Header(PyObject* self, PyObject* args) {
    FILE *outfile;
    char *outfilename;
    WavpackContext *wpc;
    int64_t total_samples;
    int qmode;
    int result;

    if (!PyArg_ParseTuple(args, "sOll", &outfilename, &wpc, &total_samples, &qmode))
        return NULL;

    outfile = fopen(outfilename, "wb");

    if (!outfile) {
        PyErr_SetString(PyExc_FileNotFoundError, "Cannot open output file");
        return NULL;
    }

    result = WriteWave64Header(outfile, wpc, total_samples, qmode);
    fclose(outfile);

    return PyLong_FromLong(result);
}

static PyMethodDef Wave64Methods[] = {
    {"ParseWave64HeaderConfig", py_ParseWave64HeaderConfig, METH_VARARGS, "Parse Wave64 header"},
    {"WriteWave64Header", py_WriteWave64Header, METH_VARARGS, "Write Wave64 header"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef wave64module = {
    PyModuleDef_HEAD_INIT,
    "wave64",
    NULL,
    -1,
    Wave64Methods
};

PyMODINIT_FUNC PyInit_wave64(void) {
    return PyModule_Create(&wave64module);
}