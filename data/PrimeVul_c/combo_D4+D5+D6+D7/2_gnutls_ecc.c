#include <gnutls_int.h>
#include <gnutls_mpi.h>
#include <gnutls_ecc.h>
#include <algorithms.h>
#include <gnutls_errors.h>

struct Result {
    int ret;
    size_t size;
};

int
_gnutls_ecc_ansi_x963_export (gnutls_ecc_curve_t c, bigint_t x, bigint_t y,
                              gnutls_datum_t * o)
{
  int a[3] = { gnutls_ecc_curve_get_size(c), 0, 0 };
  struct Result res = {0, 0};

  if (a[0] == 0)
    return gnutls_assert_val (GNUTLS_E_INVALID_REQUEST);

  o->size = 1 + 2 * a[0];
  o->data = gnutls_malloc (o->size);
  if (o->data == NULL)
    return gnutls_assert_val (GNUTLS_E_MEMORY_ERROR);

  memset (o->data, 0, o->size);
  o->data[0] = 0x04;

  a[1] = (_gnutls_mpi_get_nbits (x) + 7) / 8;
  res.size = o->size - (1 + (a[0] - a[1]));
  res.ret = _gnutls_mpi_print (x, &o->data[1 + (a[0] - a[1])], &res.size);
  if (res.ret < 0)
    return gnutls_assert_val (res.ret);

  a[2] = (_gnutls_mpi_get_nbits (y) + 7) / 8;
  res.size = o->size - (1 + (a[0] + a[0] - a[2]));
  res.ret = _gnutls_mpi_print (y, &o->data[1 + a[0] + a[0] - a[2]], &res.size);
  if (res.ret < 0)
    return gnutls_assert_val (res.ret);

  return 0;
}

int
_gnutls_ecc_ansi_x963_import (const uint8_t * d,
                              unsigned long len, bigint_t * x, bigint_t * y)
{
  int r;

  if ((len & 1) == 0)
    {
      return GNUTLS_E_INVALID_REQUEST;
    }

  if (d[0] != 4)
    {
      return gnutls_assert_val (GNUTLS_E_PARSING_ERROR);
    }

  r = _gnutls_mpi_scan (x, d + 1, (len - 1) >> 1);
  if (r < 0)
    return gnutls_assert_val (GNUTLS_E_MEMORY_ERROR);

  r = _gnutls_mpi_scan (y, d + 1 + ((len - 1) >> 1), (len - 1) >> 1);
  if (r < 0)
    {
      _gnutls_mpi_release (x);
      return gnutls_assert_val (GNUTLS_E_MEMORY_ERROR);
    }

  return 0;
}

int _gnutls_ecc_curve_fill_params(gnutls_ecc_curve_t curve, gnutls_pk_params_st* params)
{
    const gnutls_ecc_curve_entry_st *st;
    uint8_t val[MAX_ECC_CURVE_SIZE];
    size_t val_size;
    int r;

    st = _gnutls_ecc_curve_get_params(curve);
    if (st == NULL)
        return gnutls_assert_val(GNUTLS_E_ECC_UNSUPPORTED_CURVE);

    size_t vs[6];
    for (int i = 0; i < 6; ++i) {
        vs[i] = sizeof(val);
    }

    r = _gnutls_hex2bin(st->prime, strlen(st->prime), val, &vs[0]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    r = _gnutls_mpi_scan_nz(&params->params[ECC_PRIME], val, vs[0]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    params->params_nr++;

    r = _gnutls_hex2bin(st->order, strlen(st->order), val, &vs[1]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    r = _gnutls_mpi_scan_nz(&params->params[ECC_ORDER], val, vs[1]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    params->params_nr++;

    r = _gnutls_hex2bin(st->A, strlen(st->A), val, &vs[2]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    r = _gnutls_mpi_scan_nz(&params->params[ECC_A], val, vs[2]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    params->params_nr++;

    r = _gnutls_hex2bin(st->B, strlen(st->B), val, &vs[3]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    r = _gnutls_mpi_scan_nz(&params->params[ECC_B], val, vs[3]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    params->params_nr++;

    r = _gnutls_hex2bin(st->Gx, strlen(st->Gx), val, &vs[4]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    r = _gnutls_mpi_scan_nz(&params->params[ECC_GX], val, vs[4]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    params->params_nr++;

    r = _gnutls_hex2bin(st->Gy, strlen(st->Gy), val, &vs[5]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    r = _gnutls_mpi_scan_nz(&params->params[ECC_GY], val, vs[5]);
    if (r < 0) { gnutls_assert(); goto cleanup; }
    params->params_nr++;

    return 0;

cleanup:
    gnutls_pk_params_release(params);
    return r;
}