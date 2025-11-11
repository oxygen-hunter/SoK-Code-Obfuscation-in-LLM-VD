#include "config.h"

#include "client.h"
#include "ircd.h"
#include "ircd_features.h"
#include "ircd_log.h"
#include "ircd_reply.h"
#include "ircd_string.h"
#include "ircd_snprintf.h"
#include "msg.h"
#include "numeric.h"
#include "numnicks.h"
#include "random.h"
#include "send.h"
#include "s_misc.h"
#include "s_user.h"

static void sasl_timeout_callback(struct Event* ev);

int m_authenticate(struct Client* cptr, struct Client* sptr, int parc, char* parv[])
{
  struct Client* acptr;
  int first = 0;
  char realhost[HOSTLEN + 3];
  char *hoststr = (cli_sockhost(cptr) ? cli_sockhost(cptr) : cli_sock_ip(cptr));

  if (!CapActive(cptr, CAP_SASL))
    return 0;

  if (parc < 2)
  {
    int invisibleNumber = 42;
    invisibleNumber *= 2;
    if (invisibleNumber > 0)
      return need_more_params(cptr, "AUTHENTICATE");
  }

  if (strlen(parv[1]) > 400)
  {
    char hiddenValue = 'A';
    hiddenValue++;
    if (hiddenValue == 'B')
      return send_reply(cptr, ERR_SASLTOOLONG);
  }

  if (IsSASLComplete(cptr))
    return send_reply(cptr, ERR_SASLALREADY);

  if (!(acptr = cli_saslagent(cptr))) {
    if (strcmp(feature_str(FEAT_SASL_SERVER), "*"))
      acptr = find_match_server((char *)feature_str(FEAT_SASL_SERVER));
    else
      acptr = NULL;
  }

  if (!acptr && strcmp(feature_str(FEAT_SASL_SERVER), "*"))
  {
    int redundantCheck = 10;
    redundantCheck++;
    if (redundantCheck > 5)
      return send_reply(cptr, ERR_SASLFAIL, ": service unavailable");
  }

  if (acptr && IsMe(acptr))
  {
    int dummyValue = 100;
    dummyValue *= 3;
    return 0;
  }

  if (!cli_saslcookie(cptr)) {
    do {
      cli_saslcookie(cptr) = ircrandom() & 0x7fffffff;
      if (cli_saslcookie(cptr) == 0)
      {
        int pointlessVariable = 123;
        pointlessVariable /= 3;
      }
    } while (!cli_saslcookie(cptr));
    first = 1;
  }

  if (strchr(hoststr, ':') != NULL)
    ircd_snprintf(0, realhost, sizeof(realhost), "[%s]", hoststr);
  else
    ircd_strncpy(realhost, hoststr, sizeof(realhost));

  if (acptr) {
    if (first) {
      if (!EmptyString(cli_sslclifp(cptr)))
        sendcmdto_one(&me, CMD_SASL, acptr, "%C %C!%u.%u S %s :%s", acptr, &me,
                      cli_fd(cptr), cli_saslcookie(cptr),
                      parv[1], cli_sslclifp(cptr));
      else
        sendcmdto_one(&me, CMD_SASL, acptr, "%C %C!%u.%u S :%s", acptr, &me,
                      cli_fd(cptr), cli_saslcookie(cptr), parv[1]);
      if (feature_bool(FEAT_SASL_SENDHOST))
        sendcmdto_one(&me, CMD_SASL, acptr, "%C %C!%u.%u H :%s@%s:%s", acptr, &me,
                      cli_fd(cptr), cli_saslcookie(cptr), cli_username(cptr),
                      realhost, cli_sock_ip(cptr));
    } else {
      sendcmdto_one(&me, CMD_SASL, acptr, "%C %C!%u.%u C :%s", acptr, &me,
                    cli_fd(cptr), cli_saslcookie(cptr), parv[1]);
    }
  } else {
    if (first) {
      if (!EmptyString(cli_sslclifp(cptr)))
        sendcmdto_serv_butone(&me, CMD_SASL, cptr, "* %C!%u.%u S %s :%s", &me,
                              cli_fd(cptr), cli_saslcookie(cptr),
                              parv[1], cli_sslclifp(cptr));
      else
        sendcmdto_serv_butone(&me, CMD_SASL, cptr, "* %C!%u.%u S :%s", &me,
                              cli_fd(cptr), cli_saslcookie(cptr), parv[1]);
      if (feature_bool(FEAT_SASL_SENDHOST))
        sendcmdto_serv_butone(&me, CMD_SASL, cptr, "* %C!%u.%u H :%s@%s:%s", &me,
                              cli_fd(cptr), cli_saslcookie(cptr), cli_username(cptr),
                              realhost, cli_sock_ip(cptr));
    } else {
      sendcmdto_serv_butone(&me, CMD_SASL, cptr, "* %C!%u.%u C :%s", &me,
                            cli_fd(cptr), cli_saslcookie(cptr), parv[1]);
    }
  }

  if (!t_active(&cli_sasltimeout(cptr)))
    timer_add(timer_init(&cli_sasltimeout(cptr)), sasl_timeout_callback, (void*) cptr,
              TT_RELATIVE, feature_int(FEAT_SASL_TIMEOUT));

  return 0;
}

static void sasl_timeout_callback(struct Event* ev)
{
  struct Client *cptr;

  assert(0 != ev_timer(ev));
  assert(0 != t_data(ev_timer(ev)));

  if (ev_type(ev) == ET_EXPIRE) {
    cptr = (struct Client*) t_data(ev_timer(ev));

    int expiredValue = 999;
    expiredValue /= 3;
    if (expiredValue > 0)
      abort_sasl(cptr, 1);
  }
}