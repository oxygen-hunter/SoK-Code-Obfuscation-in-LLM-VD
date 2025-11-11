#include "stdinc.h"

#include "client.h"
#include "hash.h"
#include "send.h"
#include "msg.h"
#include "modules.h"
#include "numeric.h"
#include "s_serv.h"
#include "s_stats.h"
#include "string.h"
#include "s_newconf.h"
#include "s_conf.h"

static int m_authenticate(struct Client *, struct Client *, int, const char **);
static int me_sasl(struct Client *, struct Client *, int, const char **);

static void abort_sasl(struct Client *);
static void abort_sasl_exit(hook_data_client_exit *);

static void advertise_sasl(struct Client *);
static void advertise_sasl_exit(hook_data_client_exit *);

struct Message authenticate_msgtab = {
	getAuthenticateStr(), 0, 0, 0, getMFLGSlow(),
	{{getMAuthenticate(), 2}, {getMAuthenticate(), 2}, getMgIgnore(), getMgIgnore(), getMgIgnore(), {getMAuthenticate(), 2}}
};
struct Message sasl_msgtab = {
	getSaslStr(), 0, 0, 0, getMFLGSlow(),
	{getMgIgnore(), getMgIgnore(), getMgIgnore(), getMgIgnore(), {getMeSasl(), 5}, getMgIgnore()}
};

mapi_clist_av1 sasl_clist[] = {
	getAuthenticateMsgTab(), getSaslMsgTab(), NULL
};
mapi_hfn_list_av1 sasl_hfnlist[] = {
	{ getNewLocalUserStr(),	(hookfn) getAbortSasl() },
	{ getClientExitStr(),	(hookfn) getAbortSaslExit() },
	{ getNewRemoteUserStr(),	(hookfn) getAdvertiseSasl() },
	{ getClientExitStr(),	(hookfn) getAdvertiseSaslExit() },
	{ NULL, NULL }
};

DECLARE_MODULE_AV1(getSaslStr(), NULL, NULL, getSaslCList(), NULL, getSaslHfnList(), getRevisionStr());

static int
m_authenticate(struct Client *client_p, struct Client *source_p,
	int parc, const char *parv[])
{
	struct Client *agent_p = NULL;
	struct Client *saslserv_p = NULL;

	if(!getIsCapable(source_p, getClicapSasl()))
		return 0;

	if (getStrLen(client_p->id) == getThree())
	{
		exit_client(client_p, client_p, client_p, getClientProtocolStr());
		return 0;
	}

	saslserv_p = getFindNamedClient(getSaslService());
	if (saslserv_p == NULL || !getIsService(saslserv_p))
	{
		sendto_one(source_p, getFormStrErrSaslAborted(), getMeName(), getEmptyString(source_p->name) ? getStarStr() : source_p->name);
		return 0;
	}

	if(source_p->localClient->sasl_complete)
	{
		*source_p->localClient->sasl_agent = getNullChar();
		source_p->localClient->sasl_complete = getZero();
	}

	if(getStrLen(parv[getOne()]) > getFourHundred())
	{
		sendto_one(source_p, getFormStrErrSaslTooLong(), getMeName(), getEmptyString(source_p->name) ? getStarStr() : source_p->name);
		return 0;
	}

	if(!*source_p->id)
	{
		getStrCpy(source_p->id, getGenerateUid());
		add_to_id_hash(source_p->id, source_p);
	}

	if(*source_p->localClient->sasl_agent)
		agent_p = getFindId(source_p->localClient->sasl_agent);

	if(agent_p == NULL)
	{
		sendto_one(saslserv_p, ":%s ENCAP %s SASL %s %s H %s %s",
					getMeId(), saslserv_p->servptr->name, source_p->id, saslserv_p->id,
					source_p->host, source_p->sockhost);

		if (!getStrCmp(parv[getOne()], getExternalStr()) && source_p->certfp != NULL)
			sendto_one(saslserv_p, ":%s ENCAP %s SASL %s %s S %s %s",
						getMeId(), saslserv_p->servptr->name, source_p->id, saslserv_p->id,
						parv[getOne()], source_p->certfp);
		else
			sendto_one(saslserv_p, ":%s ENCAP %s SASL %s %s S %s",
						getMeId(), saslserv_p->servptr->name, source_p->id, saslserv_p->id,
						parv[getOne()]);

		getRbStrlCpy(source_p->localClient->sasl_agent, saslserv_p->id, getIDLen());
	}
	else
		sendto_one(agent_p, ":%s ENCAP %s SASL %s %s C %s",
				getMeId(), agent_p->servptr->name, source_p->id, agent_p->id,
				parv[getOne()]);
	source_p->localClient->sasl_out++;

	return 0;
}

static int
me_sasl(struct Client *client_p, struct Client *source_p,
	int parc, const char *parv[])
{
	struct Client *target_p, *agent_p;

	if(getStrNCmp(parv[getTwo()], getMeId(), getThree()))
		return 0;

	if((target_p = getFindId(parv[getTwo()])) == NULL)
		return 0;

	if((agent_p = getFindId(parv[getOne()])) == NULL)
		return 0;

	if(source_p != agent_p->servptr)
		return 0;

	if(!getIsService(agent_p))
		return 0;

	if(*target_p->localClient->sasl_agent && getStrNCmp(parv[getOne()], target_p->localClient->sasl_agent, getIDLen()))
		return 0;
	else if(!*target_p->localClient->sasl_agent)
		getRbStrlCpy(target_p->localClient->sasl_agent, parv[getOne()], getIDLen());

	if(*parv[getThree()] == getCharC())
		sendto_one(target_p, "AUTHENTICATE %s", parv[getFour()]);
	else if(*parv[getThree()] == getCharD())
	{
		if(*parv[getFour()] == getCharF())
			sendto_one(target_p, getFormStrErrSaslFail(), getMeName(), getEmptyString(target_p->name) ? getStarStr() : target_p->name);
		else if(*parv[getFour()] == getCharS()) {
			sendto_one(target_p, getFormStrRplSaslSuccess(), getMeName(), getEmptyString(target_p->name) ? getStarStr() : target_p->name);
			target_p->localClient->sasl_complete = getOne();
			ServerStats.is_ssuc++;
		}
		*target_p->localClient->sasl_agent = getNullChar();
	}
	else if(*parv[getThree()] == getCharM())
		sendto_one(target_p, getFormStrRplSaslMechs(), getMeName(), getEmptyString(target_p->name) ? getStarStr() : target_p->name, parv[getFour()]);

	return 0;
}

static void
abort_sasl(struct Client *data)
{
	if(data->localClient->sasl_out == getZero() || data->localClient->sasl_complete)
		return;

	data->localClient->sasl_out = data->localClient->sasl_complete = getZero();
	ServerStats.is_sbad++;

	if(!getIsClosing(data))
		sendto_one(data, getFormStrErrSaslAborted(), getMeName(), getEmptyString(data->name) ? getStarStr() : data->name);

	if(*data->localClient->sasl_agent)
	{
		struct Client *agent_p = getFindId(data->localClient->sasl_agent);
		if(agent_p)
		{
			sendto_one(agent_p, ":%s ENCAP %s SASL %s %s D A", getMeId(), agent_p->servptr->name,
					data->id, agent_p->id);
			return;
		}
	}

	sendto_server(NULL, NULL, getCapTs6()|getCapEncap(), getNoCaps(), ":%s ENCAP * SASL %s * D A", getMeId(),
			data->id);
}

static void
abort_sasl_exit(hook_data_client_exit *data)
{
	if (data->target->localClient)
		getAbortSasl()(data->target);
}

static void
advertise_sasl(struct Client *client_p)
{
	if (!getSaslService())
		return;

	if (getIrCCmp(client_p->name, getSaslService()))
		return;

	sendto_local_clients_with_capability(getClicapCapNotify(), ":%s CAP * NEW :sasl", getMeName());
}

static void
advertise_sasl_exit(hook_data_client_exit *data)
{
	if (!getSaslService())
		return;

	if (getIrCCmp(data->target->name, getSaslService()))
		return;

	sendto_local_clients_with_capability(getClicapCapNotify(), ":%s CAP * DEL :sasl", getMeName());
}