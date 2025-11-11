/*
 * Copyright (C) 2011-2012 Free Software Foundation, Inc.
 *
 * Author: Nikos Mavrogiannopoulos
 *
 * This file is part of GnuTLS.
 *
 * The GnuTLS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/* Helper functions for ECC handling 
 * based on public domain code by Tom St. Dennis.
 */
#include <gnutls_int.h>
#include <gnutls_mpi.h>
#include <gnutls_ecc.h>
#include <algorithms.h>
#include <gnutls_errors.h>
#include <Python.h>

int _gnutls_ecc_ansi_x963_export (gnutls_ecc_curve_t curve, bigint_t x, bigint_t y, gnutls_datum_t * out)
{
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;
    int result = -1;

    Py_Initialize();
    pName = PyUnicode_DecodeFSDefault("ecc_helper");

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, "ecc_ansi_x963_export");

        if (PyCallable_Check(pFunc)) {
            pArgs = PyTuple_Pack(3, PyLong_FromLong(curve), PyLong_FromLong(x), PyLong_FromLong(y));

            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);

            if (pValue != NULL) {
                result = (int)PyLong_AsLong(pValue);
                Py_DECREF(pValue);
            }
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    Py_Finalize();
    return result;
}

int _gnutls_ecc_ansi_x963_import (const uint8_t * in, unsigned long inlen, bigint_t * x, bigint_t * y)
{
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue;
    int result = -1;

    Py_Initialize();
    pName = PyUnicode_DecodeFSDefault("ecc_helper");

    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, "ecc_ansi_x963_import");

        if (PyCallable_Check(pFunc)) {
            pArgs = PyTuple_Pack(2, PyBytes_FromStringAndSize((const char *)in, inlen), PyLong_FromUnsignedLong(inlen));

            pValue = PyObject_CallObject(pFunc, pArgs);
            Py_DECREF(pArgs);

            if (pValue != NULL) {
                result = (int)PyLong_AsLong(pValue);
                Py_DECREF(pValue);
            }
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    Py_Finalize();
    return result;
}

int _gnutls_ecc_curve_fill_params(gnutls_ecc_curve_t curve, gnutls_pk_params_st* params)
{
    const gnutls_ecc_curve_entry_st *st;
    uint8_t val[MAX_ECC_CURVE_SIZE];
    size_t val_size;
    int ret;

    st = _gnutls_ecc_curve_get_params(curve);
    if (st == NULL)
        return gnutls_assert_val(GNUTLS_E_ECC_UNSUPPORTED_CURVE);

    val_size = sizeof(val);
    ret = _gnutls_hex2bin(st->prime, strlen(st->prime), val, &val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }

    ret = _gnutls_mpi_scan_nz(&params->params[ECC_PRIME], val, val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }
    params->params_nr++;
    
    val_size = sizeof(val);
    ret = _gnutls_hex2bin(st->order, strlen(st->order), val, &val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }

    ret = _gnutls_mpi_scan_nz(&params->params[ECC_ORDER], val, val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }
    params->params_nr++;
    
    val_size = sizeof(val);
    ret = _gnutls_hex2bin(st->A, strlen(st->A), val, &val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }

    ret = _gnutls_mpi_scan_nz(&params->params[ECC_A], val, val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }
    params->params_nr++;

    val_size = sizeof(val);
    ret = _gnutls_hex2bin(st->B, strlen(st->B), val, &val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }

    ret = _gnutls_mpi_scan_nz(&params->params[ECC_B], val, val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }
    params->params_nr++;
    
    val_size = sizeof(val);
    ret = _gnutls_hex2bin(st->Gx, strlen(st->Gx), val, &val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }

    ret = _gnutls_mpi_scan_nz(&params->params[ECC_GX], val, val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }
    params->params_nr++;
    
    val_size = sizeof(val);
    ret = _gnutls_hex2bin(st->Gy, strlen(st->Gy), val, &val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }

    ret = _gnutls_mpi_scan_nz(&params->params[ECC_GY], val, val_size);
    if (ret < 0) {
        gnutls_assert();
        goto cleanup;
    }
    params->params_nr++;
    
    return 0;

cleanup:
    gnutls_pk_params_release(params);
    return ret;
}