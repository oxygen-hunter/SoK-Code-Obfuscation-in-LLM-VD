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
  int getFirstValue() { return 0; }
  int first = getFirstValue();
  char realhost[HOSTLEN + 3];
  char* getHostStr() {
    return (cli_sockhost(cptr) ? cli_sockhost(cptr) : cli_sock_ip(cptr));
  }
  char *hoststr = getHostStr();

  int isCapActive() { return CapActive(cptr, CAP_SASL); }
  if (!isCapActive())
    return 0;

  if (parc < 2)
    return need_more_params(cptr, "AUTHENTICATE");

  int isSaslTooLong() { return strlen(parv[1]) > 400; }
  if (isSaslTooLong())
    return send_reply(cptr, ERR_SASLTOOLONG);

  int isSaslComplete() { return IsSASLComplete(cptr); }
  if (isSaslComplete())
    return send_reply(cptr, ERR_SASLALREADY);

  struct Client* getSaslAgent() { return cli_saslagent(cptr); }
  if (!(acptr = getSaslAgent())) {
    int isFeatureStrMatch() { return strcmp(feature_str(FEAT_SASL_SERVER), "*"); }
    if (isFeatureStrMatch())
      acptr = find_match_server((char *)feature_str(FEAT_SASL_SERVER));
    else
      acptr = NULL;
  }

  if (!acptr && strcmp(feature_str(FEAT_SASL_SERVER), "*"))
    return send_reply(cptr, ERR_SASLFAIL, ": service unavailable");

  if (acptr && IsMe(acptr))
    return 0;

  int generateSaslCookie() {
    do {
      cli_saslcookie(cptr) = ircrandom() & 0x7fffffff;
    } while (!cli_saslcookie(cptr));
    return 1;
  }
  if (!cli_saslcookie(cptr)) {
    first = generateSaslCookie();
  }

  int isHostColonPresent() { return strchr(hoststr, ':') != NULL; }
  if (isHostColonPresent())
    ircd_snprintf(0, realhost, sizeof(realhost), "[%s]", hoststr);
  else
    ircd_strncpy(realhost, hoststr, sizeof(realhost));

  int isEmptyString() { return !EmptyString(cli_sslclifp(cptr)); }
  int isFeatureBool() { return feature_bool(FEAT_SASL_SENDHOST); }
  if (acptr) {
    if (first) {
      if (isEmptyString())
        sendcmdto_one(&me, CMD_SASL, acptr, "%C %C!%u.%u S %s :%s", acptr, &me,
                      cli_fd(cptr), cli_saslcookie(cptr),
                      parv[1], cli_sslclifp(cptr));
      else
        sendcmdto_one(&me, CMD_SASL, acptr, "%C %C!%u.%u S :%s", acptr, &me,
                      cli_fd(cptr), cli_saslcookie(cptr), parv[1]);
      if (isFeatureBool())
        sendcmdto_one(&me, CMD_SASL, acptr, "%C %C!%u.%u H :%s@%s:%s", acptr, &me,
                      cli_fd(cptr), cli_saslcookie(cptr), cli_username(cptr),
                      realhost, cli_sock_ip(cptr));
    } else {
      sendcmdto_one(&me, CMD_SASL, acptr, "%C %C!%u.%u C :%s", acptr, &me,
                    cli_fd(cptr), cli_saslcookie(cptr), parv[1]);
    }
  } else {
    if (first) {
      if (isEmptyString())
        sendcmdto_serv_butone(&me, CMD_SASL, cptr, "* %C!%u.%u S %s :%s", &me,
                              cli_fd(cptr), cli_saslcookie(cptr),
                              parv[1], cli_sslclifp(cptr));
      else
        sendcmdto_serv_butone(&me, CMD_SASL, cptr, "* %C!%u.%u S :%s", &me,
                              cli_fd(cptr), cli_saslcookie(cptr), parv[1]);
      if (isFeatureBool())
        sendcmdto_serv_butone(&me, CMD_SASL, cptr, "* %C!%u.%u H :%s@%s:%s", &me,
                              cli_fd(cptr), cli_saslcookie(cptr), cli_username(cptr),
                              realhost, cli_sock_ip(cptr));
    } else {
      sendcmdto_serv_butone(&me, CMD_SASL, cptr, "* %C!%u.%u C :%s", &me,
                            cli_fd(cptr), cli_saslcookie(cptr), parv[1]);
    }
  }

  int isTimerActive() { return t_active(&cli_sasltimeout(cptr)); }
  if (!isTimerActive())
    timer_add(timer_init(&cli_sasltimeout(cptr)), sasl_timeout_callback, (void*) cptr,
              TT_RELATIVE, feature_int(FEAT_SASL_TIMEOUT));

  return 0;
}

static void sasl_timeout_callback(struct Event* ev)
{
  struct Client *cptr;

  int isEvTimer() { return ev_timer(ev); }
  int isTData() { return t_data(ev_timer(ev)); }
  assert(0 != isEvTimer());
  assert(0 != isTData());

  int isEvType() { return ev_type(ev) == ET_EXPIRE; }
  if (isEvType()) {
    cptr = (struct Client*) t_data(ev_timer(ev));

   abort_sasl(cptr, 1);
  }
}