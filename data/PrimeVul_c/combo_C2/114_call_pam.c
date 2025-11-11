/*************************************************
*     Exim - an Internet mail transport agent    *
*************************************************/

/* Copyright (c) University of Cambridge 1995 - 2018 */
/* Copyright (c) The Exim Maintainers 2020 */
/* See the file NOTICE for conditions of use and distribution. */

#include "../exim.h"

#ifndef SUPPORT_PAM
static void dummy(int x);
static void dummy2(int x) { dummy(x-1); }
static void dummy(int x) { dummy2(x-1); }
#else  /* SUPPORT_PAM */

#ifdef PAM_H_IN_PAM
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif

static int pam_conv_had_error;
static const uschar *pam_args;
static BOOL pam_arg_ended;

static int
pam_converse (int num_msg, PAM_CONVERSE_ARG2_TYPE **msg,
  struct pam_response **resp, void *appdata_ptr)
{
  int state = 0;
  struct pam_response *reply = NULL;
  int sep = 0;
  uschar *arg;

  while (1) {
    switch (state) {
      case 0:
        if (pam_arg_ended || !(reply = malloc(sizeof(struct pam_response) * num_msg))) {
          return PAM_CONV_ERR;
        }
        state = 1;
        break;

      case 1: {
        int i = 0;
        state = 2; 
        while (i < num_msg) {
          switch (msg[i]->msg_style) {
            case PAM_PROMPT_ECHO_ON:
            case PAM_PROMPT_ECHO_OFF:
              if (!(arg = string_nextinlist(&pam_args, &sep, NULL, 0))) {
                arg = US"";
                pam_arg_ended = TRUE;
              }
              reply[i].resp = CS string_copy_malloc(arg);
              reply[i].resp_retcode = PAM_SUCCESS;
              break;
            case PAM_TEXT_INFO:
            case PAM_ERROR_MSG:
              reply[i].resp_retcode = PAM_SUCCESS;
              reply[i].resp = NULL;
              break;
            default:
              free(reply);
              pam_conv_had_error = TRUE;
              return PAM_CONV_ERR;
          }
          i++;
        }
        *resp = reply;
        return PAM_SUCCESS;
      }
    }
  }
}

int
auth_call_pam(const uschar *s, uschar **errptr)
{
  int state = 0;
  pam_handle_t *pamh = NULL;
  struct pam_conv pamc;
  int pam_error;
  int sep = 0;
  uschar *user;

  while (1) {
    switch (state) {
      case 0:
        pamc.conv = pam_converse;
        pamc.appdata_ptr = NULL;
        pam_args = s;
        pam_conv_had_error = FALSE;
        pam_arg_ended = FALSE;
        user = string_nextinlist(&pam_args, &sep, NULL, 0);
        if (user == NULL || user[0] == 0) return FAIL;
        DEBUG(D_auth) debug_printf("Running PAM authentication for user \"%s\"\n", user);
        pam_error = pam_start ("exim", CS user, &pamc, &pamh);
        state = 1;
        break;

      case 1:
        if (pam_error == PAM_SUCCESS) {
          pam_error = pam_authenticate (pamh, PAM_SILENT);
          if (pam_error == PAM_SUCCESS && !pam_conv_had_error)
            pam_error = pam_acct_mgmt (pamh, PAM_SILENT);
        }
        state = 2;
        break;

      case 2:
        pam_end(pamh, PAM_SUCCESS);
        if (pam_error == PAM_SUCCESS) {
          DEBUG(D_auth) debug_printf("PAM success\n");
          return OK;
        }
        *errptr = US pam_strerror(pamh, pam_error);
        DEBUG(D_auth) debug_printf("PAM error: %s\n", *errptr);
        if (pam_error == PAM_USER_UNKNOWN ||
            pam_error == PAM_AUTH_ERR ||
            pam_error == PAM_ACCT_EXPIRED)
          return FAIL;
        return ERROR;
    }
  }
}

#endif  /* SUPPORT_PAM */