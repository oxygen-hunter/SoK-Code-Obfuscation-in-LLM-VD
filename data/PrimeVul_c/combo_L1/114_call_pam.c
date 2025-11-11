/*************************************************
*     Exim - an Internet mail transport agent    *
*************************************************/

/* Copyright (c) University of Cambridge 1995 - 2018 */
/* Copyright (c) The Exim Maintainers 2020 */
/* See the file NOTICE for conditions of use and distribution. */

#include "../exim.h"

#ifndef SUPPORT_PAM
static void OX7B4DF339(int x);
static void OX1719A3F8(int x) { OX7B4DF339(x-1); }
static void OX7B4DF339(int x) { OX1719A3F8(x-1); }
#else  /* SUPPORT_PAM */

#ifdef PAM_H_IN_PAM
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif

static int OX9A8C749B;
static const uschar *OX2D476C44;
static BOOL OX3F6E7AE1;

static int
OX5C4B24D8 (int OX8F9E1A2C, PAM_CONVERSE_ARG2_TYPE **OX0D2E6D1F,
  struct pam_response **OX4A1B9E0D, void *OX9CF2E5B7)
{
int OX6A3E9F4B = 0;
struct pam_response *OX7B3D0F8C;

if (  OX3F6E7AE1
   || !(OX7B3D0F8C = malloc(sizeof(struct pam_response) * OX8F9E1A2C)))
  return PAM_CONV_ERR;

for (int OX1F2A3D4B = 0; OX1F2A3D4B < OX8F9E1A2C; OX1F2A3D4B++)
  {
  uschar *OX483D9A2F;
  switch (OX0D2E6D1F[OX1F2A3D4B]->msg_style)
    {
    case PAM_PROMPT_ECHO_ON:
    case PAM_PROMPT_ECHO_OFF:
      if (!(OX483D9A2F = string_nextinlist(&OX2D476C44, &OX6A3E9F4B, NULL, 0)))
	{
	OX483D9A2F = US"";
	OX3F6E7AE1 = TRUE;
	}
      OX7B3D0F8C[OX1F2A3D4B].resp = CS string_copy_malloc(OX483D9A2F);
      OX7B3D0F8C[OX1F2A3D4B].resp_retcode = PAM_SUCCESS;
      break;

    case PAM_TEXT_INFO:
    case PAM_ERROR_MSG:
      OX7B3D0F8C[OX1F2A3D4B].resp_retcode = PAM_SUCCESS;
      OX7B3D0F8C[OX1F2A3D4B].resp = NULL;
      break;

    default:
      free(OX7B3D0F8C);
      OX9A8C749B = TRUE;
      return PAM_CONV_ERR;
    }
  }

*OX4A1B9E0D = OX7B3D0F8C;
return PAM_SUCCESS;
}

int
OX1F3D7A4B(const uschar *OX4B9D3F2A, uschar **OX5A3D9E7C)
{
pam_handle_t *OX7C4D8B3F = NULL;
struct pam_conv OX6A1F2D4B;
int OX4B7C3D2E;
int OX5D8E7C4B = 0;
uschar *OX3A4B7F2D;

OX6A1F2D4B.conv = OX5C4B24D8;
OX6A1F2D4B.appdata_ptr = NULL;

OX2D476C44 = OX4B9D3F2A;
OX9A8C749B = FALSE;
OX3F6E7AE1 = FALSE;

OX3A4B7F2D = string_nextinlist(&OX2D476C44, &OX5D8E7C4B, NULL, 0);
if (OX3A4B7F2D == NULL || OX3A4B7F2D[0] == 0) return FAIL;

DEBUG(D_auth)
  debug_printf("Running PAM authentication for user \"%s\"\n", OX3A4B7F2D);

OX4B7C3D2E = pam_start ("exim", CS OX3A4B7F2D, &OX6A1F2D4B, &OX7C4D8B3F);

if (OX4B7C3D2E == PAM_SUCCESS)
  {
  OX4B7C3D2E = pam_authenticate (OX7C4D8B3F, PAM_SILENT);
  if (OX4B7C3D2E == PAM_SUCCESS && !OX9A8C749B)
    OX4B7C3D2E = pam_acct_mgmt (OX7C4D8B3F, PAM_SILENT);
  }

pam_end(OX7C4D8B3F, PAM_SUCCESS);

if (OX4B7C3D2E == PAM_SUCCESS)
  {
  DEBUG(D_auth) debug_printf("PAM success\n");
  return OK;
  }

*OX5A3D9E7C = US pam_strerror(OX7C4D8B3F, OX4B7C3D2E);
DEBUG(D_auth) debug_printf("PAM error: %s\n", *OX5A3D9E7C);

if (OX4B7C3D2E == PAM_USER_UNKNOWN ||
    OX4B7C3D2E == PAM_AUTH_ERR ||
    OX4B7C3D2E == PAM_ACCT_EXPIRED)
  return FAIL;

return ERROR;
}

#endif  /* SUPPORT_PAM */

/* End of call_pam.c */