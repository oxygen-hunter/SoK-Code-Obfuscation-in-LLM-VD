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
  struct {
    int a;
    char b[HOSTLEN + 3];
  } data;
  data.a = 0;
  char *hoststr = (cli_sockhost(cptr) ? cli_sockhost(cptr) : cli_sock_ip(cptr));

  if (!CapActive(cptr, CAP_SASL))
    return 0;

  if (parc < 2)
    return need_more_params(cptr, "AUTHENTICATE");

  if (strlen(parv[1]) > 400)
    return send_reply(cptr, ERR_SASLTOOLONG);

  if (IsSASLComplete(cptr))
    return send_reply(cptr, ERR_SASLALREADY);

  if (!(acptr = cli_saslagent(cptr))) {
    if (strcmp(feature_str(FEAT_SASL_SERVER), "*"))
      acptr = find_match_server((char *)feature_str(FEAT_SASL_SERVER));
    else
      acptr = NULL;
  }

  if (!acptr && strcmp(feature_str(FEAT_SASL_SERVER), "*"))
    return send_reply(cptr, ERR_SASLFAIL, ": service unavailable");

  if (acptr && IsMe(acptr))
    return 0;

  if (!cli_saslcookie(cptr)) {
    do {
      cli_saslcookie(cptr) = ircrandom() & 0x7fffffff;
    } while (!cli_saslcookie(cptr));
    data.a = 1;
  }

  if (strchr(hoststr, ':') != NULL)
    ircd_snprintf(0, data.b, sizeof(data.b), "[%s]", hoststr);
  else
    ircd_strncpy(data.b, hoststr, sizeof(data.b));

  if (acptr) {
    if (data.a) {
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
                      data.b, cli_sock_ip(cptr));
    } else {
      sendcmdto_one(&me, CMD_SASL, acptr, "%C %C!%u.%u C :%s", acptr, &me,
                    cli_fd(cptr), cli_saslcookie(cptr), parv[1]);
    }
  } else {
    if (data.a) {
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
                              data.b, cli_sock_ip(cptr));
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

   abort_sasl(cptr, 1);
  }
}