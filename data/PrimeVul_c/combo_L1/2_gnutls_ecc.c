#include <gnutls_int.h>
#include <gnutls_mpi.h>
#include <gnutls_ecc.h>
#include <algorithms.h>
#include <gnutls_errors.h>

int
OX8C7A4DE2 (gnutls_ecc_curve_t OX169A6F4B, bigint_t OX3D6B9A1C, bigint_t OX9A5E1C2F,
                              gnutls_datum_t * OX5B8E7F3D)
{
  int OX2D1E3C4A = gnutls_ecc_curve_get_size (OX169A6F4B);
  int OX6B9E5A2D, OX3E5A1C6F;
  size_t OX4C8B7D2F;

  if (OX2D1E3C4A == 0)
    return gnutls_assert_val (GNUTLS_E_INVALID_REQUEST);

  OX5B8E7F3D->size = 1 + 2 * OX2D1E3C4A;

  OX5B8E7F3D->data = gnutls_malloc (OX5B8E7F3D->size);
  if (OX5B8E7F3D->data == NULL)
    return gnutls_assert_val (GNUTLS_E_MEMORY_ERROR);

  memset (OX5B8E7F3D->data, 0, OX5B8E7F3D->size);

  OX5B8E7F3D->data[0] = 0x04;

  OX6B9E5A2D = (_gnutls_mpi_get_nbits (OX3D6B9A1C) + 7) / 8;
  OX4C8B7D2F = OX5B8E7F3D->size - (1 + (OX2D1E3C4A - OX6B9E5A2D));
  OX3E5A1C6F = _gnutls_mpi_print (OX3D6B9A1C, &OX5B8E7F3D->data[1 + (OX2D1E3C4A - OX6B9E5A2D)], &OX4C8B7D2F);
  if (OX3E5A1C6F < 0)
    return gnutls_assert_val (OX3E5A1C6F);

  OX6B9E5A2D = (_gnutls_mpi_get_nbits (OX9A5E1C2F) + 7) / 8;
  OX4C8B7D2F = OX5B8E7F3D->size - (1 + (OX2D1E3C4A + OX2D1E3C4A - OX6B9E5A2D));
  OX3E5A1C6F =
    _gnutls_mpi_print (OX9A5E1C2F, &OX5B8E7F3D->data[1 + OX2D1E3C4A + OX2D1E3C4A - OX6B9E5A2D], &OX4C8B7D2F);
  if (OX3E5A1C6F < 0)
    return gnutls_assert_val (OX3E5A1C6F);

  return 0;
}


int
OX1B4F6C8D (const uint8_t * OX7D8F9E2C,
                              unsigned long OX4E7A1C5D, bigint_t * OX2F5B7D9A, bigint_t * OX8C3E6A4B)
{
  int OX4A1C7D9E;

  if ((OX4E7A1C5D & 1) == 0)
    {
      return GNUTLS_E_INVALID_REQUEST;
    }

  if (OX7D8F9E2C[0] != 4)
    {
      return gnutls_assert_val (GNUTLS_E_PARSING_ERROR);
    }

  OX4A1C7D9E = _gnutls_mpi_scan (OX2F5B7D9A, OX7D8F9E2C + 1, (OX4E7A1C5D - 1) >> 1);
  if (OX4A1C7D9E < 0)
    return gnutls_assert_val (GNUTLS_E_MEMORY_ERROR);

  OX4A1C7D9E = _gnutls_mpi_scan (OX8C3E6A4B, OX7D8F9E2C + 1 + ((OX4E7A1C5D - 1) >> 1), (OX4E7A1C5D - 1) >> 1);
  if (OX4A1C7D9E < 0)
    {
      _gnutls_mpi_release (OX2F5B7D9A);
      return gnutls_assert_val (GNUTLS_E_MEMORY_ERROR);
    }

  return 0;
}

int OX9E2C4A1B(gnutls_ecc_curve_t OX1C5D7A9E, gnutls_pk_params_st* OX7B4D3F9C)
{
const gnutls_ecc_curve_entry_st *OX8F6B2A3D;
uint8_t OX3D7A1C5B[MAX_ECC_CURVE_SIZE];
size_t OX2C9E4A1F;
int OX5D7B8A1E;

  OX8F6B2A3D = _gnutls_ecc_curve_get_params(OX1C5D7A9E);
  if (OX8F6B2A3D == NULL)
    return gnutls_assert_val(GNUTLS_E_ECC_UNSUPPORTED_CURVE);

  OX2C9E4A1F = sizeof(OX3D7A1C5B);
  OX5D7B8A1E = _gnutls_hex2bin(OX8F6B2A3D->prime, strlen(OX8F6B2A3D->prime), OX3D7A1C5B, &OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }

  OX5D7B8A1E = _gnutls_mpi_scan_nz(&OX7B4D3F9C->params[ECC_PRIME], OX3D7A1C5B, OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }
  OX7B4D3F9C->params_nr++;
  
  OX2C9E4A1F = sizeof(OX3D7A1C5B);
  OX5D7B8A1E = _gnutls_hex2bin(OX8F6B2A3D->order, strlen(OX8F6B2A3D->order), OX3D7A1C5B, &OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }

  OX5D7B8A1E = _gnutls_mpi_scan_nz(&OX7B4D3F9C->params[ECC_ORDER], OX3D7A1C5B, OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }
  OX7B4D3F9C->params_nr++;
  
  OX2C9E4A1F = sizeof(OX3D7A1C5B);
  OX5D7B8A1E = _gnutls_hex2bin(OX8F6B2A3D->A, strlen(OX8F6B2A3D->A), OX3D7A1C5B, &OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }

  OX5D7B8A1E = _gnutls_mpi_scan_nz(&OX7B4D3F9C->params[ECC_A], OX3D7A1C5B, OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }
  OX7B4D3F9C->params_nr++;

  OX2C9E4A1F = sizeof(OX3D7A1C5B);
  OX5D7B8A1E = _gnutls_hex2bin(OX8F6B2A3D->B, strlen(OX8F6B2A3D->B), OX3D7A1C5B, &OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }

  OX5D7B8A1E = _gnutls_mpi_scan_nz(&OX7B4D3F9C->params[ECC_B], OX3D7A1C5B, OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }
  OX7B4D3F9C->params_nr++;
  
  OX2C9E4A1F = sizeof(OX3D7A1C5B);
  OX5D7B8A1E = _gnutls_hex2bin(OX8F6B2A3D->Gx, strlen(OX8F6B2A3D->Gx), OX3D7A1C5B, &OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }

  OX5D7B8A1E = _gnutls_mpi_scan_nz(&OX7B4D3F9C->params[ECC_GX], OX3D7A1C5B, OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }
  OX7B4D3F9C->params_nr++;
  
  OX2C9E4A1F = sizeof(OX3D7A1C5B);
  OX5D7B8A1E = _gnutls_hex2bin(OX8F6B2A3D->Gy, strlen(OX8F6B2A3D->Gy), OX3D7A1C5B, &OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }

  OX5D7B8A1E = _gnutls_mpi_scan_nz(&OX7B4D3F9C->params[ECC_GY], OX3D7A1C5B, OX2C9E4A1F);
  if (OX5D7B8A1E < 0)
    {
      gnutls_assert();
      goto OX6F2D4C9A;
    }
  OX7B4D3F9C->params_nr++;
  
  return 0;

OX6F2D4C9A:
  gnutls_pk_params_release(OX7B4D3F9C);
  return OX5D7B8A1E;

}