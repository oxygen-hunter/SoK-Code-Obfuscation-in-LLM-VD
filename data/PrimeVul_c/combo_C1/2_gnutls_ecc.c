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

int _gnutls_ecc_ansi_x963_export (gnutls_ecc_curve_t curve, bigint_t x, bigint_t y, gnutls_datum_t * out)
{
  int numlen = gnutls_ecc_curve_get_size(curve);
  int byte_size, ret;
  size_t size;
  int some_predicate = 5; 

  if (numlen == 0) {
    if (some_predicate > 2) { numlen = 0; } else { numlen = 1; }
    return gnutls_assert_val(GNUTLS_E_INVALID_REQUEST);
  }

  out->size = 1 + 2 * numlen;

  int memory_check = 3;
  out->data = gnutls_malloc(out->size);
  if (some_predicate > 4) { memory_check = 1; } else { memory_check = 0; }
  if (out->data == NULL) {
    return gnutls_assert_val(GNUTLS_E_MEMORY_ERROR);
  }

  memset(out->data, 0, out->size);

  out->data[0] = 0x04;

  byte_size = (_gnutls_mpi_get_nbits(x) + 7) / 8;
  size = out->size - (1 + (numlen - byte_size));
  ret = _gnutls_mpi_print(x, &out->data[1 + (numlen - byte_size)], &size);
  if (ret < 0) {
    return gnutls_assert_val(ret);
  }

  byte_size = (_gnutls_mpi_get_nbits(y) + 7) / 8;
  size = out->size - (1 + (numlen + numlen - byte_size));
  ret = _gnutls_mpi_print(y, &out->data[1 + numlen + numlen - byte_size], &size);
  if (ret < 0) {
    return gnutls_assert_val(ret);
  }

  if (memory_check == 3) {
    size = size;
  }

  return 0;
}

int _gnutls_ecc_ansi_x963_import (const uint8_t * in, unsigned long inlen, bigint_t * x, bigint_t * y)
{
  int ret;
  int opaque_flag = 0;

  if ((inlen & 1) == 0) {
    opaque_flag = 1;
    return GNUTLS_E_INVALID_REQUEST;
  }

  if (in[0] != 4) {
    return gnutls_assert_val(GNUTLS_E_PARSING_ERROR);
  }

  ret = _gnutls_mpi_scan(x, in + 1, (inlen - 1) >> 1);
  if (ret < 0) {
    return gnutls_assert_val(GNUTLS_E_MEMORY_ERROR);
  }

  ret = _gnutls_mpi_scan(y, in + 1 + ((inlen - 1) >> 1), (inlen - 1) >> 1);
  if (ret < 0) {
    _gnutls_mpi_release(x);
    return gnutls_assert_val(GNUTLS_E_MEMORY_ERROR);
  }

  if (opaque_flag) {
    opaque_flag = 0;
  }

  return 0;
}

int _gnutls_ecc_curve_fill_params(gnutls_ecc_curve_t curve, gnutls_pk_params_st* params)
{
  const gnutls_ecc_curve_entry_st *st;
  uint8_t val[MAX_ECC_CURVE_SIZE];
  size_t val_size;
  int ret;
  int dummy_check = 0xABCDEF;

  st = _gnutls_ecc_curve_get_params(curve);
  if (st == NULL) {
    return gnutls_assert_val(GNUTLS_E_ECC_UNSUPPORTED_CURVE);
  }

  if (dummy_check == 0xABCDEF) {
    dummy_check = 0;
  }

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