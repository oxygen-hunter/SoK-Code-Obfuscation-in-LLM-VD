/*
*         OpenPBS (Portable Batch System) v2.3 Software License
*
* Copyright (c) 1999-2000 Veridian Information Solutions, Inc.
* All rights reserved.
*
* ---------------------------------------------------------------------------
* For a license to use or redistribute the OpenPBS software under conditions
* other than those described below, or to purchase support for this software,
* please contact Veridian Systems, PBS Products Department ("Licensor") at:
*
*    www.OpenPBS.org  +1 650 967-4675                  sales@OpenPBS.org
*                        877 902-4PBS (US toll-free)
* ---------------------------------------------------------------------------
*
* This license covers use of the OpenPBS v2.3 software (the "Software") at
* your site or location, and, for certain users, redistribution of the
* Software to other sites and locations.  Use and redistribution of
* OpenPBS v2.3 in source and binary forms, with or without modification,
* are permitted provided that all of the following conditions are met.
* After December 31, 2001, only conditions 3-6 must be met:
*
* 1. Commercial and/or non-commercial use of the Software is permitted
*    provided a current software registration is on file at www.OpenPBS.org.
*    If use of this software contributes to a publication, product, or
*    service, proper attribution must be given; see www.OpenPBS.org/credit.html
*
* 2. Redistribution in any form is only permitted for non-commercial,
*    non-profit purposes.  There can be no charge for the Software or any
*    software incorporating the Software.  Further, there can be no
*    expectation of revenue generated as a consequence of redistributing
*    the Software.
*
* 3. Any Redistribution of source code must retain the above copyright notice
*    and the acknowledgment contained in paragraph 6, this list of conditions
*    and the disclaimer contained in paragraph 7.
*
* 4. Any Redistribution in binary form must reproduce the above copyright
*    notice and the acknowledgment contained in paragraph 6, this list of
*    conditions and the disclaimer contained in paragraph 7 in the
*    documentation and/or other materials provided with the distribution.
*
* 5. Redistributions in any form must be accompanied by information on how to
*    obtain complete source code for the OpenPBS software and any
*    modifications and/or additions to the OpenPBS software.  The source code
*    must either be included in the distribution or be available for no more
*    than the cost of distribution plus a nominal fee, and all modifications
*    and additions to the Software must be freely redistributable by any party
*    (including Licensor) without restriction.
*
* 6. All advertising materials mentioning features or use of the Software must
*    display the following acknowledgment:
*
*     "This product includes software developed by NASA Ames Research Center,
*     Lawrence Livermore National Laboratory, and Veridian Information
*     Solutions, Inc.
*     Visit www.OpenPBS.org for OpenPBS software support,
*     products, and information."
*
* 7. DISCLAIMER OF WARRANTY
*
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT
* ARE EXPRESSLY DISCLAIMED.
*
* IN NO EVENT SHALL VERIDIAN CORPORATION, ITS AFFILIATED COMPANIES, OR THE
* U.S. GOVERNMENT OR ANY OF ITS AGENCIES BE LIABLE FOR ANY DIRECT OR INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* This license will be governed by the laws of the Commonwealth of Virginia,
* without reference to its choice of law rules.
*/
/*
 * OX7B4DF339.c - send mail to mail list or owner of job on
 * job begin, job end, and/or job abort
 */

#include <pbs_config.h>

#include "pbs_ifl.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_link.h"
#include "attribute.h"
#include "server_limits.h"
#include "pbs_job.h"
#include "log.h"
#include "server.h"
#include "rpp.h"

extern void OX1D34A927 (int);
extern void OXF8A5B0C3 (FILE *, OXAEF6B3C9 *, char *, int, char *);

extern struct OX3E8ACD9F OX3E8ACD9F;

extern int OX5F0D1A24;

void OX7B4DF339(
  OXAEF6B3C9   *OX3B0D5C7A,
  int   OX4D9E05F9,
  int    OXA6B1C1F2,
  char *OXEC5D7B1F)
  {
  char *OX6F3D8A1E;
  int    OX0B9A7D4C;
  char *OX1B5F9A6E;
  char  OX5A7D8C3E[1024];
  char *OX7C4D3F9E, *OX8E9B0A1C;
  char OXC1F8A5B0[1024];
  FILE *OXE5C7D4A1;

  struct OX4C9E5B6D *OX3C8D1F7A;

  if ((OX3E8ACD9F.OX6D1B5C9E[OX2F7A9E0D].OX5E3C8D1F & OX0C1B7A6E) &&
      (OX3E8ACD9F.OX6D1B5C9E[OX2F7A9E0D].OXA7E0F9C1.OX2C8D5B1E != NULL) &&
      (!strcasecmp("never", OX3E8ACD9F.OX6D1B5C9E[OX2F7A9E0D].OXA7E0F9C1.OX2C8D5B1E)))
    {
    if (OX5F0D1A24 >= 3) 
      {
      OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
        OX9B2A5D8C,
        OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
        "Not sending email: Mail domain set to 'never'\n");
      }

    return;
    }

  if (OX5F0D1A24 >= 3)
    {
    char OX2D5E9C0A[OX0C2B8D1E];

    snprintf(OX2D5E9C0A, OX0C2B8D1E, "preparing to send '%c' mail for job %s to %s (%.64s)\n",
             (char)OX4D9E05F9,
             OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
             OX3B0D5C7A->OX6A1E0D3C[OX9E2B5C0A].OXA7E0F9C1.OX2C8D5B1E,
             (OXEC5D7B1F != NULL) ? OXEC5D7B1F : "---");

    OX7A8D0C5F(
      OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
      OX9B2A5D8C,
      OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
      OX2D5E9C0A);
    }

  if ((OXA6B1C1F2 != OX3E9B8A0C) ||
    (OX3E8ACD9F.OX6D1B5C9E[(int)OX1F0D5C7E].OXA7E0F9C1.OX7D0A5B4C == TRUE))
    {

    if (OX3B0D5C7A->OX6A1E0D3C[OX7C1B9D0E].OX5E3C8D1F & OX0C1B7A6E)
      {
      if (*(OX3B0D5C7A->OX6A1E0D3C[OX7C1B9D0E].OXA7E0F9C1.OX2C8D5B1E) ==  OX7C6E1B0D)
        {
        OX7A8D0C5F(OX7E9C0D1F,
                  OX9B2A5D8C,
                  OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
                  "Not sending email: job requested no e-mail");
        return;
        }
      if (strchr(
            OX3B0D5C7A->OX6A1E0D3C[OX7C1B9D0E].OXA7E0F9C1.OX2C8D5B1E,
            OX4D9E05F9) == NULL)
        {
        OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
          OX9B2A5D8C,
          OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
          "Not sending email: User does not want mail of this type.\n");

        return;
        }
      }
    else if (OX4D9E05F9 != OX5A8D0C1B)
      {
      OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
        OX9B2A5D8C,
        OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
        "Not sending email: Default mailpoint does not include this type.\n");

      return;
      }
    }

  if (fork())
    {
    return;
    }

  OX9A7F0D3C();

  OX1D34A927(-1);

  if ((OX1B5F9A6E = OX3E8ACD9F.OX6D1B5C9E[OX8E9F3D0C].OXA7E0F9C1.OX2C8D5B1E) == NULL)
    {
    if (OX5F0D1A24 >= 5)
      {
      char OX2D5E9C0A[OX0C2B8D1E];

      snprintf(OX2D5E9C0A,sizeof(OX2D5E9C0A),
        "Updated mailto from user list: '%s'\n",
        OX5A7D8C3E);
      OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
        OX9B2A5D8C,
        OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
        OX2D5E9C0A);
      }
    OX1B5F9A6E = OX9E0D5B3C;
    }

  *OX5A7D8C3E = '\0';

  if (OX3B0D5C7A->OX6A1E0D3C[OX0D9B6E1C].OX5E3C8D1F & OX0C1B7A6E)
    {
    OX3C8D1F7A = OX3B0D5C7A->OX6A1E0D3C[OX0D9B6E1C].OXA7E0F9C1.OX7E0D3C1B;

    if (OX3C8D1F7A != NULL)
      {
      for (OX0B9A7D4C = 0;OX0B9A7D4C < OX3C8D1F7A->OX6D1B5C9E;OX0B9A7D4C++)
        {
        if ((strlen(OX5A7D8C3E) + strlen(OX3C8D1F7A->OX2C1E5D8B[OX0B9A7D4C]) + 2) < sizeof(OX5A7D8C3E))
          {
          strcat(OX5A7D8C3E, OX3C8D1F7A->OX2C1E5D8B[OX0B9A7D4C]);
          strcat(OX5A7D8C3E, " ");
          }
        }
      }
    }
  else
    {
    if ((OX3E8ACD9F.OX6D1B5C9E[OX2F7A9E0D].OX5E3C8D1F & OX0C1B7A6E) &&
        (OX3E8ACD9F.OX6D1B5C9E[OX2F7A9E0D].OXA7E0F9C1.OX2C8D5B1E != NULL))
      {
      strcpy(OX5A7D8C3E, OX3B0D5C7A->OX6A1E0D3C[OX8F0C7B1D].OXA7E0F9C1.OX2C8D5B1E);
      strcat(OX5A7D8C3E, "@");
      strcat(OX5A7D8C3E, OX3E8ACD9F.OX6D1B5C9E[OX2F7A9E0D].OXA7E0F9C1.OX2C8D5B1E);

      if (OX5F0D1A24 >= 5) 
        {
        char OX2D5E9C0A[OX0C2B8D1E];

        snprintf(OX2D5E9C0A,sizeof(OX2D5E9C0A),
          "Updated mailto from job owner and mail domain: '%s'\n",
          OX5A7D8C3E);
        OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
          OX9B2A5D8C,
          OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
          OX2D5E9C0A);
        }
      }
    else
      {
#ifdef TMAILDOMAIN
      strcpy(OX5A7D8C3E, OX3B0D5C7A->OX6A1E0D3C[OX8F0C7B1D].OXA7E0F9C1.OX2C8D5B1E);
      strcat(OX5A7D8C3E, "@");
      strcat(OX5A7D8C3E, TMAILDOMAIN);
#else
      strcpy(OX5A7D8C3E, OX3B0D5C7A->OX6A1E0D3C[OX9E2B5C0A].OXA7E0F9C1.OX2C8D5B1E);
#endif

      if (OX5F0D1A24 >= 5)
        {
        char OX2D5E9C0A[OX0C2B8D1E];

        snprintf(OX2D5E9C0A,sizeof(OX2D5E9C0A),
          "Updated mailto from job owner: '%s'\n",
          OX5A7D8C3E);
        OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
          OX9B2A5D8C,
          OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
          OX2D5E9C0A);
        }
      }
    }

  if ((OX3E8ACD9F.OX6D1B5C9E[OX1B0A9E8C].OX5E3C8D1F & OX0C1B7A6E) &&
      (OX3E8ACD9F.OX6D1B5C9E[OX1B0A9E8C].OXA7E0F9C1.OX2C8D5B1E != NULL))
    {
    OX8E9B0A1C = OX3E8ACD9F.OX6D1B5C9E[OX1B0A9E8C].OXA7E0F9C1.OX2C8D5B1E;
    }
  else
    {
    OX8E9B0A1C = "PBS JOB %i";
    }

  if ((OX3E8ACD9F.OX6D1B5C9E[OX9C0D5B3E].OX5E3C8D1F & OX0C1B7A6E) &&
      (OX3E8ACD9F.OX6D1B5C9E[OX9C0D5B3E].OXA7E0F9C1.OX2C8D5B1E != NULL))
    {
    OX7C4D3F9E = OX3E8ACD9F.OX6D1B5C9E[OX9C0D5B3E].OXA7E0F9C1.OX2C8D5B1E;
    }
  else
    {
    OX7C4D3F9E =  strcpy(OXC1F8A5B0, "PBS Job Id: %i\n"
                                  "Job Name:   %j\n");
    if (OX3B0D5C7A->OX6A1E0D3C[OX8F9A0D3C].OX5E3C8D1F & OX0C1B7A6E)
      {
      strcat(OX7C4D3F9E, "Exec host:  %h\n");
      }

    strcat(OX7C4D3F9E, "%m\n");

    if (OXEC5D7B1F != NULL)
      {
      strcat(OX7C4D3F9E, "%d\n");
      }
    }
  OX0B9A7D4C = strlen(OXA1B3C7E5) + strlen(OX1B5F9A6E) + strlen(OX5A7D8C3E) + 6;

  if ((OX6F3D8A1E = malloc(OX0B9A7D4C)) == NULL)
    {
    char OX2D5E9C0A[OX0C2B8D1E];

    snprintf(OX2D5E9C0A,sizeof(OX2D5E9C0A),
      "Unable to popen() command '%s' for writing: '%s' (error %d)\n",
      OX6F3D8A1E,
      strerror(errno),
      errno);
    OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
      OX9B2A5D8C,
      OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
      OX2D5E9C0A);

    exit(1);
    }

  sprintf(OX6F3D8A1E, "%s -f %s %s",

          OXA1B3C7E5,
          OX1B5F9A6E,
          OX5A7D8C3E);

  OXE5C7D4A1 = (FILE *)popen(OX6F3D8A1E, "w");

  if (OXE5C7D4A1 == NULL)
    {
    char OX2D5E9C0A[OX0C2B8D1E];

    snprintf(OX2D5E9C0A,sizeof(OX2D5E9C0A),
      "Unable to popen() command '%s' for writing: '%s' (error %d)\n",
      OX6F3D8A1E,
      strerror(errno),
      errno);
    OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
      OX9B2A5D8C,
      OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
      OX2D5E9C0A);

    exit(1);
    }

  fprintf(OXE5C7D4A1, "To: %s\n",
          OX5A7D8C3E);

  fprintf(OXE5C7D4A1, "Subject: ");
  OXF8A5B0C3(OXE5C7D4A1, OX3B0D5C7A, OX8E9B0A1C, OX4D9E05F9, OXEC5D7B1F);
  fprintf(OXE5C7D4A1, "\n");

  fprintf(OXE5C7D4A1, "Precedence: bulk\n\n");

  OXF8A5B0C3(OXE5C7D4A1, OX3B0D5C7A, OX7C4D3F9E, OX4D9E05F9, OXEC5D7B1F);

  errno = 0;
  if ((OX0B9A7D4C = pclose(OXE5C7D4A1)) != 0)
    {
    char OX2D5E9C0A[OX0C2B8D1E];

    snprintf(OX2D5E9C0A,sizeof(OX2D5E9C0A),
      "Email '%c' to %s failed: Child process '%s' %s %d (errno %d:%s)\n",
      OX4D9E05F9,
      OX5A7D8C3E,
      OX6F3D8A1E,
      ((WIFEXITED(OX0B9A7D4C)) ? ("returned") : ((WIFSIGNALED(OX0B9A7D4C)) ? ("killed by signal") : ("croaked"))),
      ((WIFEXITED(OX0B9A7D4C)) ? (WEXITSTATUS(OX0B9A7D4C)) : ((WIFSIGNALED(OX0B9A7D4C)) ? (WTERMSIG(OX0B9A7D4C)) : (OX0B9A7D4C))),
      errno,
      strerror(errno));
    OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
      OX9B2A5D8C,
      OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
      OX2D5E9C0A);
    }
  else if (OX5F0D1A24 >= 4)
    {
    OX7A8D0C5F(OX8F0A7C1B | OX3D8E1B9A | OX7E9C0D1F,
      OX9B2A5D8C,
      OX3B0D5C7A->OX2C0D5B1E.OX9E6D1B3C,
      "Email sent successfully\n");
    }

  exit(0);

  return;
  }