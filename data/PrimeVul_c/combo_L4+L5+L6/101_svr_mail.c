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

extern void net_close (int);
extern void svr_format_job (FILE *, job *, char *, int, char *);

extern struct server server;

extern int LOGLEVEL;

void recursive_send_mail(job *pjob, int mailpoint, int force, char *text, int step);

void svr_mailowner(job *pjob, int mailpoint, int force, char *text) {
  recursive_send_mail(pjob, mailpoint, force, text, 0);
}

void recursive_send_mail(job *pjob, int mailpoint, int force, char *text, int step) {
  static char *cmdbuf;
  static int i;
  static char *mailfrom;
  static char mailto[1024];
  static char *bodyfmt, *subjectfmt;
  static char bodyfmtbuf[1024];
  static FILE *outmail;
  static struct array_strings *pas;

  switch (step) {
    case 0:
      if ((server.sv_attr[SRV_ATR_MailDomain].at_flags & ATR_VFLAG_SET) &&
          (server.sv_attr[SRV_ATR_MailDomain].at_val.at_str != NULL) &&
          (!strcasecmp("never", server.sv_attr[SRV_ATR_MailDomain].at_val.at_str))) {
        if (LOGLEVEL >= 3) {
          log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
            PBS_EVENTCLASS_JOB,
            pjob->ji_qs.ji_jobid,
            "Not sending email: Mail domain set to 'never'\n");
        }
        return;
      }

      if (LOGLEVEL >= 3) {
        char tmpBuf[LOG_BUF_SIZE];
        snprintf(tmpBuf, LOG_BUF_SIZE, "preparing to send '%c' mail for job %s to %s (%.64s)\n",
                 (char)mailpoint,
                 pjob->ji_qs.ji_jobid,
                 pjob->ji_wattr[JOB_ATR_job_owner].at_val.at_str,
                 (text != NULL) ? text : "---");
        log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
          PBS_EVENTCLASS_JOB,
          pjob->ji_qs.ji_jobid,
          tmpBuf);
      }

      if ((force != MAIL_FORCE) ||
          (server.sv_attr[(int)SRV_ATR_NoMailForce].at_val.at_long == TRUE)) {
        if (pjob->ji_wattr[JOB_ATR_mailpnts].at_flags & ATR_VFLAG_SET) {
          if (*(pjob->ji_wattr[JOB_ATR_mailpnts].at_val.at_str) ==  MAIL_NONE) {
            log_event(PBSEVENT_JOB,
                      PBS_EVENTCLASS_JOB,
                      pjob->ji_qs.ji_jobid,
                      "Not sending email: job requested no e-mail");
            return;
          }
          if (strchr(
                pjob->ji_wattr[JOB_ATR_mailpnts].at_val.at_str,
                mailpoint) == NULL) {
            log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
              PBS_EVENTCLASS_JOB,
              pjob->ji_qs.ji_jobid,
              "Not sending email: User does not want mail of this type.\n");
            return;
          }
        } else if (mailpoint != MAIL_ABORT) {
          log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
            PBS_EVENTCLASS_JOB,
            pjob->ji_qs.ji_jobid,
            "Not sending email: Default mailpoint does not include this type.\n");
          return;
        }
      }

      if (fork()) {
        return;
      }

      rpp_terminate();
      net_close(-1);

      if ((mailfrom = server.sv_attr[SRV_ATR_mailfrom].at_val.at_str) == NULL) {
        if (LOGLEVEL >= 5) {
          char tmpBuf[LOG_BUF_SIZE];
          snprintf(tmpBuf,sizeof(tmpBuf),
            "Updated mailto from user list: '%s'\n",
            mailto);
          log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
            PBS_EVENTCLASS_JOB,
            pjob->ji_qs.ji_jobid,
            tmpBuf);
        }
        mailfrom = PBS_DEFAULT_MAIL;
      }

      *mailto = '\0';

      if (pjob->ji_wattr[JOB_ATR_mailuser].at_flags & ATR_VFLAG_SET) {
        pas = pjob->ji_wattr[JOB_ATR_mailuser].at_val.at_arst;
        if (pas != NULL) {
          for (i = 0; i < pas->as_usedptr; i++) {
            if ((strlen(mailto) + strlen(pas->as_string[i]) + 2) < sizeof(mailto)) {
              strcat(mailto, pas->as_string[i]);
              strcat(mailto, " ");
            }
          }
        }
      } else {
        if ((server.sv_attr[SRV_ATR_MailDomain].at_flags & ATR_VFLAG_SET) &&
            (server.sv_attr[SRV_ATR_MailDomain].at_val.at_str != NULL)) {
          strcpy(mailto, pjob->ji_wattr[JOB_ATR_euser].at_val.at_str);
          strcat(mailto, "@");
          strcat(mailto, server.sv_attr[SRV_ATR_MailDomain].at_val.at_str);
          if (LOGLEVEL >= 5) {
            char tmpBuf[LOG_BUF_SIZE];
            snprintf(tmpBuf,sizeof(tmpBuf),
              "Updated mailto from job owner and mail domain: '%s'\n",
              mailto);
            log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
              PBS_EVENTCLASS_JOB,
              pjob->ji_qs.ji_jobid,
              tmpBuf);
          }
        } else {
#ifdef TMAILDOMAIN
          strcpy(mailto, pjob->ji_wattr[JOB_ATR_euser].at_val.at_str);
          strcat(mailto, "@");
          strcat(mailto, TMAILDOMAIN);
#else 
          strcpy(mailto, pjob->ji_wattr[JOB_ATR_job_owner].at_val.at_str);
#endif 
          if (LOGLEVEL >= 5) {
            char tmpBuf[LOG_BUF_SIZE];
            snprintf(tmpBuf,sizeof(tmpBuf),
              "Updated mailto from job owner: '%s'\n",
              mailto);
            log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
              PBS_EVENTCLASS_JOB,
              pjob->ji_qs.ji_jobid,
              tmpBuf);
          }
        }
      }

      if ((server.sv_attr[SRV_ATR_MailSubjectFmt].at_flags & ATR_VFLAG_SET) &&
          (server.sv_attr[SRV_ATR_MailSubjectFmt].at_val.at_str != NULL)) {
        subjectfmt = server.sv_attr[SRV_ATR_MailSubjectFmt].at_val.at_str;
      } else {
        subjectfmt = "PBS JOB %i";
      }

      if ((server.sv_attr[SRV_ATR_MailBodyFmt].at_flags & ATR_VFLAG_SET) &&
          (server.sv_attr[SRV_ATR_MailBodyFmt].at_val.at_str != NULL)) {
        bodyfmt = server.sv_attr[SRV_ATR_MailBodyFmt].at_val.at_str;
      } else {
        bodyfmt =  strcpy(bodyfmtbuf, "PBS Job Id: %i\n"
                                      "Job Name:   %j\n");
        if (pjob->ji_wattr[JOB_ATR_exec_host].at_flags & ATR_VFLAG_SET) {
          strcat(bodyfmt, "Exec host:  %h\n");
        }
        strcat(bodyfmt, "%m\n");
        if (text != NULL) {
          strcat(bodyfmt, "%d\n");
        }
      }

      i = strlen(SENDMAIL_CMD) + strlen(mailfrom) + strlen(mailto) + 6;
      if ((cmdbuf = malloc(i)) == NULL) {
        char tmpBuf[LOG_BUF_SIZE];
        snprintf(tmpBuf,sizeof(tmpBuf),
          "Unable to popen() command '%s' for writing: '%s' (error %d)\n",
          cmdbuf,
          strerror(errno),
          errno);
        log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
          PBS_EVENTCLASS_JOB,
          pjob->ji_qs.ji_jobid,
          tmpBuf);
        exit(1);
      }

      sprintf(cmdbuf, "%s -f %s %s",
              SENDMAIL_CMD,
              mailfrom,
              mailto);

      outmail = (FILE *)popen(cmdbuf, "w");

      if (outmail == NULL) {
        char tmpBuf[LOG_BUF_SIZE];
        snprintf(tmpBuf,sizeof(tmpBuf),
          "Unable to popen() command '%s' for writing: '%s' (error %d)\n",
          cmdbuf,
          strerror(errno),
          errno);
        log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
          PBS_EVENTCLASS_JOB,
          pjob->ji_qs.ji_jobid,
          tmpBuf);
        exit(1);
      }

      fprintf(outmail, "To: %s\n",
              mailto);
      fprintf(outmail, "Subject: ");
      svr_format_job(outmail, pjob, subjectfmt, mailpoint, text);
      fprintf(outmail, "\n");
      fprintf(outmail, "Precedence: bulk\n\n");
      svr_format_job(outmail, pjob, bodyfmt, mailpoint, text);

      errno = 0;
      if ((i = pclose(outmail)) != 0) {
        char tmpBuf[LOG_BUF_SIZE];
        snprintf(tmpBuf,sizeof(tmpBuf),
          "Email '%c' to %s failed: Child process '%s' %s %d (errno %d:%s)\n",
          mailpoint,
          mailto,
          cmdbuf,
          ((WIFEXITED(i)) ? ("returned") : ((WIFSIGNALED(i)) ? ("killed by signal") : ("croaked"))),
          ((WIFEXITED(i)) ? (WEXITSTATUS(i)) : ((WIFSIGNALED(i)) ? (WTERMSIG(i)) : (i))),
          errno,
          strerror(errno));
        log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
          PBS_EVENTCLASS_JOB,
          pjob->ji_qs.ji_jobid,
          tmpBuf);
      } else if (LOGLEVEL >= 4) {
        log_event(PBSEVENT_ERROR | PBSEVENT_ADMIN | PBSEVENT_JOB,
          PBS_EVENTCLASS_JOB,
          pjob->ji_qs.ji_jobid,
          "Email sent successfully\n");
      }
      exit(0);
      return;
  }
}