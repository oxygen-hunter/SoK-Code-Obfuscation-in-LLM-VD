/*
 * Audio and Video frame extraction
 * Copyright (c) 2003 Fabrice Bellard
 * Copyright (c) 2003 Michael Niedermayer
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <Python.h>
#include "parser.h"
#include "aac_ac3_parser.h"
#include "aacadtsdec.h"
#include "get_bits.h"
#include "mpeg4audio.h"

static int aac_sync(uint64_t state, AACAC3ParseContext *hdr_info,
        int *need_next_header, int *new_frame_start)
{
    GetBitContext bits;
    AACADTSHeaderInfo hdr;
    int size;
    union {
        uint64_t u64;
        uint8_t  u8[8];
    } tmp;

    tmp.u64 = av_be2ne64(state);
    init_get_bits(&bits, tmp.u8+8-AAC_ADTS_HEADER_SIZE, AAC_ADTS_HEADER_SIZE * 8);

    if ((size = avpriv_aac_parse_header(&bits, &hdr)) < 0)
        return 0;
    *need_next_header = 0;
    *new_frame_start  = 1;
    hdr_info->sample_rate = hdr.sample_rate;
    hdr_info->channels    = ff_mpeg4audio_channels[hdr.chan_config];
    hdr_info->samples     = hdr.samples;
    hdr_info->bit_rate    = hdr.bit_rate;
    return size;
}

static av_cold int aac_parse_init(AVCodecParserContext *s1)
{
    AACAC3ParseContext *s = s1->priv_data;
    s->header_size = AAC_ADTS_HEADER_SIZE;
    s->sync = aac_sync;
    return 0;
}

static PyObject* wrap_aac_sync(PyObject* self, PyObject* args) {
    uint64_t state;
    PyObject *hdr_info_obj;
    int need_next_header, new_frame_start;
    
    if (!PyArg_ParseTuple(args, "KOii", &state, &hdr_info_obj, &need_next_header, &new_frame_start))
        return NULL;
    
    AACAC3ParseContext hdr_info;
    if (aac_sync(state, &hdr_info, &need_next_header, &new_frame_start) > 0) {
        PyObject* dict = PyDict_New();
        PyDict_SetItemString(dict, "sample_rate", PyLong_FromLong(hdr_info.sample_rate));
        PyDict_SetItemString(dict, "channels", PyLong_FromLong(hdr_info.channels));
        PyDict_SetItemString(dict, "samples", PyLong_FromLong(hdr_info.samples));
        PyDict_SetItemString(dict, "bit_rate", PyLong_FromLong(hdr_info.bit_rate));
        return dict;
    }
    Py_RETURN_NONE;
}

static PyMethodDef AacMethods[] = {
    {"aac_sync", wrap_aac_sync, METH_VARARGS, "Synchronize AAC header"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef aacmodule = {
    PyModuleDef_HEAD_INIT,
    "aac",
    NULL,
    -1,
    AacMethods
};

PyMODINIT_FUNC PyInit_aac(void) {
    return PyModule_Create(&aacmodule);
}

AVCodecParser ff_aac_parser = {
    .codec_ids      = { AV_CODEC_ID_AAC },
    .priv_data_size = sizeof(AACAC3ParseContext),
    .parser_init    = aac_parse_init,
    .parser_parse   = ff_aac_ac3_parse,
    .parser_close   = ff_parse_close,
};