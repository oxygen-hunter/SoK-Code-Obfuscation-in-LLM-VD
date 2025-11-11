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

static void OX7B4DF339(struct Event* OX4FAD5A75);

int OX5E3F3A9B(struct Client* OX1A2F9F8D, struct Client* OX3B9A6D4E, int OX6A1B5F9C, char* OX0C1E2D3B[])
{
  struct Client* OX2D3E4C5B;
  int OX4C5B6A7E = 0;
  char OX5B6A7C8D[HOSTLEN + 3];
  char *OX6B7C8D9E = (cli_sockhost(OX1A2F9F8D) ? cli_sockhost(OX1A2F9F8D) : cli_sock_ip(OX1A2F9F8D));

  if (!CapActive(OX1A2F9F8D, CAP_SASL))
    return 0;

  if (OX6A1B5F9C < 2)
    return need_more_params(OX1A2F9F8D, "AUTHENTICATE");

  if (strlen(OX0C1E2D3B[1]) > 400)
    return send_reply(OX1A2F9F8D, ERR_SASLTOOLONG);

  if (IsSASLComplete(OX1A2F9F8D))
    return send_reply(OX1A2F9F8D, ERR_SASLALREADY);

  if (!(OX2D3E4C5B = cli_saslagent(OX1A2F9F8D))) {
    if (strcmp(feature_str(FEAT_SASL_SERVER), "*"))
      OX2D3E4C5B = find_match_server((char *)feature_str(FEAT_SASL_SERVER));
    else
      OX2D3E4C5B = NULL;
  }

  if (!OX2D3E4C5B && strcmp(feature_str(FEAT_SASL_SERVER), "*"))
    return send_reply(OX1A2F9F8D, ERR_SASLFAIL, ": service unavailable");

  if (OX2D3E4C5B && IsMe(OX2D3E4C5B))
    return 0;

  if (!cli_saslcookie(OX1A2F9F8D)) {
    do {
      cli_saslcookie(OX1A2F9F8D) = ircrandom() & 0x7fffffff;
    } while (!cli_saslcookie(OX1A2F9F8D));
    OX4C5B6A7E = 1;
  }

  if (strchr(OX6B7C8D9E, ':') != NULL)
    ircd_snprintf(0, OX5B6A7C8D, sizeof(OX5B6A7C8D), "[%s]", OX6B7C8D9E);
  else
    ircd_strncpy(OX5B6A7C8D, OX6B7C8D9E, sizeof(OX5B6A7C8D));

  if (OX2D3E4C5B) {
    if (OX4C5B6A7E) {
      if (!EmptyString(cli_sslclifp(OX1A2F9F8D)))
        sendcmdto_one(&me, CMD_SASL, OX2D3E4C5B, "%C %C!%u.%u S %s :%s", OX2D3E4C5B, &me,
                      cli_fd(OX1A2F9F8D), cli_saslcookie(OX1A2F9F8D),
                      OX0C1E2D3B[1], cli_sslclifp(OX1A2F9F8D));
      else
        sendcmdto_one(&me, CMD_SASL, OX2D3E4C5B, "%C %C!%u.%u S :%s", OX2D3E4C5B, &me,
                      cli_fd(OX1A2F9F8D), cli_saslcookie(OX1A2F9F8D), OX0C1E2D3B[1]);
      if (feature_bool(FEAT_SASL_SENDHOST))
        sendcmdto_one(&me, CMD_SASL, OX2D3E4C5B, "%C %C!%u.%u H :%s@%s:%s", OX2D3E4C5B, &me,
                      cli_fd(OX1A2F9F8D), cli_saslcookie(OX1A2F9F8D), cli_username(OX1A2F9F8D),
                      OX5B6A7C8D, cli_sock_ip(OX1A2F9F8D));
    } else {
      sendcmdto_one(&me, CMD_SASL, OX2D3E4C5B, "%C %C!%u.%u C :%s", OX2D3E4C5B, &me,
                    cli_fd(OX1A2F9F8D), cli_saslcookie(OX1A2F9F8D), OX0C1E2D3B[1]);
    }
  } else {
    if (OX4C5B6A7E) {
      if (!EmptyString(cli_sslclifp(OX1A2F9F8D)))
        sendcmdto_serv_butone(&me, CMD_SASL, OX1A2F9F8D, "* %C!%u.%u S %s :%s", &me,
                              cli_fd(OX1A2F9F8D), cli_saslcookie(OX1A2F9F8D),
                              OX0C1E2D3B[1], cli_sslclifp(OX1A2F9F8D));
      else
        sendcmdto_serv_butone(&me, CMD_SASL, OX1A2F9F8D, "* %C!%u.%u S :%s", &me,
                              cli_fd(OX1A2F9F8D), cli_saslcookie(OX1A2F9F8D), OX0C1E2D3B[1]);
      if (feature_bool(FEAT_SASL_SENDHOST))
        sendcmdto_serv_butone(&me, CMD_SASL, OX1A2F9F8D, "* %C!%u.%u H :%s@%s:%s", &me,
                              cli_fd(OX1A2F9F8D), cli_saslcookie(OX1A2F9F8D), cli_username(OX1A2F9F8D),
                              OX5B6A7C8D, cli_sock_ip(OX1A2F9F8D));
    } else {
      sendcmdto_serv_butone(&me, CMD_SASL, OX1A2F9F8D, "* %C!%u.%u C :%s", &me,
                            cli_fd(OX1A2F9F8D), cli_saslcookie(OX1A2F9F8D), OX0C1E2D3B[1]);
    }
  }

  if (!t_active(&cli_sasltimeout(OX1A2F9F8D)))
    timer_add(timer_init(&cli_sasltimeout(OX1A2F9F8D)), OX7B4DF339, (void*) OX1A2F9F8D,
              TT_RELATIVE, feature_int(FEAT_SASL_TIMEOUT));

  return 0;
}

static void OX7B4DF339(struct Event* OX4FAD5A75)
{
  struct Client *OX2B3E4C5D;

  assert(0 != ev_timer(OX4FAD5A75));
  assert(0 != t_data(ev_timer(OX4FAD5A75)));

  if (ev_type(OX4FAD5A75) == ET_EXPIRE) {
    OX2B3E4C5D = (struct Client*) t_data(ev_timer(OX4FAD5A75));

   abort_sasl(OX2B3E4C5D, 1);
  }
}