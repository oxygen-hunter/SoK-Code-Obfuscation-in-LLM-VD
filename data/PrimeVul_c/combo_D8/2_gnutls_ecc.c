#include <gnutls_int.h>
#include <gnutls_mpi.h>
#include <gnutls_ecc.h>
#include <algorithms.h>
#include <gnutls_errors.h>

int _gnutls_ecc_ansi_x963_export(gnutls_ecc_curve_t curve, bigint_t x, bigint_t y, gnutls_datum_t *out) {
  int numlen = gnutls_ecc_curve_get_size(curve);
  int byte_size, ret;
  size_t size;

  if (numlen == 0)
    return gnutls_assert_val(GNUTLS_E_INVALID_REQUEST);

  out->size = getOutSize(numlen);

  out->data = getOutData(out->size);
  if (out->data == NULL)
    return gnutls_assert_val(GNUTLS_E_MEMORY_ERROR);

  memset(out->data, 0, out->size);

  out->data[0] = getInitialByte();

  byte_size = getByteSizeX(x);
  size = getSizeX(out->size, numlen, byte_size);
  ret = _gnutls_mpi_print(x, &out->data[1 + (numlen - byte_size)], &size);
  if (ret < 0)
    return gnutls_assert_val(ret);

  byte_size = getByteSizeY(y);
  size = getSizeY(out->size, numlen, byte_size);
  ret = _gnutls_mpi_print(y, &out->data[1 + numlen + numlen - byte_size], &size);
  if (ret < 0)
    return gnutls_assert_val(ret);

  return 0;
}

int _gnutls_ecc_ansi_x963_import(const uint8_t *in, unsigned long inlen, bigint_t *x, bigint_t *y) {
  int ret;

  if (isInvalidLength(inlen))
    return GNUTLS_E_INVALID_REQUEST;

  if (!isValidInitialByte(in[0]))
    return gnutls_assert_val(GNUTLS_E_PARSING_ERROR);

  ret = _gnutls_mpi_scan(x, in + 1, getScanLength(inlen));
  if (ret < 0)
    return gnutls_assert_val(GNUTLS_E_MEMORY_ERROR);

  ret = _gnutls_mpi_scan(y, in + 1 + getScanOffset(inlen), getScanLength(inlen));
  if (ret < 0) {
    _gnutls_mpi_release(x);
    return gnutls_assert_val(GNUTLS_E_MEMORY_ERROR);
  }

  return 0;
}

int _gnutls_ecc_curve_fill_params(gnutls_ecc_curve_t curve, gnutls_pk_params_st *params) {
  const gnutls_ecc_curve_entry_st *st;
  uint8_t val[MAX_ECC_CURVE_SIZE];
  size_t val_size;
  int ret;

  st = _gnutls_ecc_curve_get_params(curve);
  if (!isValidCurve(st))
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
  incrementParamsNr(params);

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
  incrementParamsNr(params);

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
  incrementParamsNr(params);

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
  incrementParamsNr(params);

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
  incrementParamsNr(params);

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
  incrementParamsNr(params);

  return 0;

cleanup:
  gnutls_pk_params_release(params);
  return ret;
}

int getOutSize(int numlen) {
  return 1 + 2 * numlen;
}

uint8_t *getOutData(size_t size) {
  return gnutls_malloc(size);
}

int getInitialByte() {
  return 0x04;
}

int getByteSizeX(bigint_t x) {
  return (_gnutls_mpi_get_nbits(x) + 7) / 8;
}

size_t getSizeX(size_t out_size, int numlen, int byte_size) {
  return out_size - (1 + (numlen - byte_size));
}

int getByteSizeY(bigint_t y) {
  return (_gnutls_mpi_get_nbits(y) + 7) / 8;
}

size_t getSizeY(size_t out_size, int numlen, int byte_size) {
  return out_size - (1 + (numlen + numlen - byte_size));
}

int isInvalidLength(unsigned long inlen) {
  return (inlen & 1) == 0;
}

int isValidInitialByte(uint8_t byte) {
  return byte == 4;
}

unsigned long getScanLength(unsigned long inlen) {
  return (inlen - 1) >> 1;
}

unsigned long getScanOffset(unsigned long inlen) {
  return ((inlen - 1) >> 1);
}

int isValidCurve(const gnutls_ecc_curve_entry_st *st) {
  return st != NULL;
}

void incrementParamsNr(gnutls_pk_params_st *params) {
  params->params_nr++;
}