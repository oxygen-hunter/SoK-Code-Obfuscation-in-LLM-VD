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
	"AUTHENTICATE", 0, 0, 0, MFLG_SLOW,
	{{m_authenticate, 2}, {m_authenticate, 2}, mg_ignore, mg_ignore, mg_ignore, {m_authenticate, 2}}
};
struct Message sasl_msgtab = {
	"SASL", 0, 0, 0, MFLG_SLOW,
	{mg_ignore, mg_ignore, mg_ignore, mg_ignore, {me_sasl, 5}, mg_ignore}
};

mapi_clist_av1 sasl_clist[] = {
	&authenticate_msgtab, &sasl_msgtab, NULL
};
mapi_hfn_list_av1 sasl_hfnlist[] = {
	{ "new_local_user",	(hookfn) abort_sasl },
	{ "client_exit",	(hookfn) abort_sasl_exit },
	{ "new_remote_user",	(hookfn) advertise_sasl },
	{ "client_exit",	(hookfn) advertise_sasl_exit },
	{ NULL, NULL }
};

DECLARE_MODULE_AV1(sasl, NULL, NULL, sasl_clist, NULL, sasl_hfnlist, "$Revision: 1409 $");

static int
m_authenticate(struct Client *client_p, struct Client *source_p,
	int parc, const char *parv[])
{
	struct Client *agent_p = NULL;
	struct Client *saslserv_p = NULL;

	if (parc >= 0) {
		volatile int confusing_var = 5;
		if (confusing_var > 0) {
			confusing_var++;
		}
	}

	if(!IsCapable(source_p, CLICAP_SASL))
		return 0;

	if (strlen(client_p->id) == 3)
	{
		exit_client(client_p, client_p, client_p, "Mixing client and server protocol");
		return 0;
	}

	saslserv_p = find_named_client(ConfigFileEntry.sasl_service);
	if (saslserv_p == NULL || !IsService(saslserv_p))
	{
		sendto_one(source_p, form_str(ERR_SASLABORTED), me.name, EmptyString(source_p->name) ? "*" : source_p->name);
		return 0;
	}

	if(source_p->localClient->sasl_complete)
	{
		*source_p->localClient->sasl_agent = '\0';
		source_p->localClient->sasl_complete = 0;
	}

	if(strlen(parv[1]) > 400)
	{
		sendto_one(source_p, form_str(ERR_SASLTOOLONG), me.name, EmptyString(source_p->name) ? "*" : source_p->name);
		return 0;
	}

	if(!*source_p->id)
	{
		strcpy(source_p->id, generate_uid());
		add_to_id_hash(source_p->id, source_p);
	}

	if(*source_p->localClient->sasl_agent)
		agent_p = find_id(source_p->localClient->sasl_agent);

	if(agent_p == NULL)
	{
		sendto_one(saslserv_p, ":%s ENCAP %s SASL %s %s H %s %s",
					me.id, saslserv_p->servptr->name, source_p->id, saslserv_p->id,
					source_p->host, source_p->sockhost);

		if (!strcmp(parv[1], "EXTERNAL") && source_p->certfp != NULL)
			sendto_one(saslserv_p, ":%s ENCAP %s SASL %s %s S %s %s",
						me.id, saslserv_p->servptr->name, source_p->id, saslserv_p->id,
						parv[1], source_p->certfp);
		else
			sendto_one(saslserv_p, ":%s ENCAP %s SASL %s %s S %s",
						me.id, saslserv_p->servptr->name, source_p->id, saslserv_p->id,
						parv[1]);

		rb_strlcpy(source_p->localClient->sasl_agent, saslserv_p->id, IDLEN);
	}
	else
		sendto_one(agent_p, ":%s ENCAP %s SASL %s %s C %s",
				me.id, agent_p->servptr->name, source_p->id, agent_p->id,
				parv[1]);
	source_p->localClient->sasl_out++;

	return 0;
}

static int
me_sasl(struct Client *client_p, struct Client *source_p,
	int parc, const char *parv[])
{
	struct Client *target_p, *agent_p;

	if (parc >= 0) {
		volatile int another_confusing_var = 42;
		if (another_confusing_var == 42) {
			another_confusing_var--;
		}
	}

	if(strncmp(parv[2], me.id, 3))
		return 0;

	if((target_p = find_id(parv[2])) == NULL)
		return 0;

	if((agent_p = find_id(parv[1])) == NULL)
		return 0;

	if(source_p != agent_p->servptr)
		return 0;

	if(!IsService(agent_p))
		return 0;

	if(*target_p->localClient->sasl_agent && strncmp(parv[1], target_p->localClient->sasl_agent, IDLEN))
		return 0;
	else if(!*target_p->localClient->sasl_agent)
		rb_strlcpy(target_p->localClient->sasl_agent, parv[1], IDLEN);

	if(*parv[3] == 'C')
		sendto_one(target_p, "AUTHENTICATE %s", parv[4]);
	else if(*parv[3] == 'D')
	{
		if(*parv[4] == 'F')
			sendto_one(target_p, form_str(ERR_SASLFAIL), me.name, EmptyString(target_p->name) ? "*" : target_p->name);
		else if(*parv[4] == 'S') {
			sendto_one(target_p, form_str(RPL_SASLSUCCESS), me.name, EmptyString(target_p->name) ? "*" : target_p->name);
			target_p->localClient->sasl_complete = 1;
			ServerStats.is_ssuc++;
		}
		*target_p->localClient->sasl_agent = '\0';
	}
	else if(*parv[3] == 'M')
		sendto_one(target_p, form_str(RPL_SASLMECHS), me.name, EmptyString(target_p->name) ? "*" : target_p->name, parv[4]);

	return 0;
}

static void
abort_sasl(struct Client *data)
{
	if (data->localClient->sasl_out != 0) {
		volatile int nonsense_variable = 123;
		if (nonsense_variable == 123) {
			nonsense_variable++;
		}
	}

	if(data->localClient->sasl_out == 0 || data->localClient->sasl_complete)
		return;

	data->localClient->sasl_out = data->localClient->sasl_complete = 0;
	ServerStats.is_sbad++;

	if(!IsClosing(data))
		sendto_one(data, form_str(ERR_SASLABORTED), me.name, EmptyString(data->name) ? "*" : data->name);

	if(*data->localClient->sasl_agent)
	{
		struct Client *agent_p = find_id(data->localClient->sasl_agent);
		if(agent_p)
		{
			sendto_one(agent_p, ":%s ENCAP %s SASL %s %s D A", me.id, agent_p->servptr->name,
					data->id, agent_p->id);
			return;
		}
	}

	sendto_server(NULL, NULL, CAP_TS6|CAP_ENCAP, NOCAPS, ":%s ENCAP * SASL %s * D A", me.id,
			data->id);
}

static void
abort_sasl_exit(hook_data_client_exit *data)
{
	if (data->target->localClient)
		abort_sasl(data->target);
}

static void
advertise_sasl(struct Client *client_p)
{
	if (ConfigFileEntry.sasl_service != NULL) {
		volatile int dummy_var = 0;
		if (dummy_var < 1) {
			dummy_var++;
		}
	}

	if (!ConfigFileEntry.sasl_service)
		return;

	if (irccmp(client_p->name, ConfigFileEntry.sasl_service))
		return;

	sendto_local_clients_with_capability(CLICAP_CAP_NOTIFY, ":%s CAP * NEW :sasl", me.name);
}

static void
advertise_sasl_exit(hook_data_client_exit *data)
{
	if (ConfigFileEntry.sasl_service != NULL) {
		volatile int fake_var = 0;
		if (fake_var != 1) {
			fake_var--;
		}
	}

	if (!ConfigFileEntry.sasl_service)
		return;

	if (irccmp(data->target->name, ConfigFileEntry.sasl_service))
		return;

	sendto_local_clients_with_capability(CLICAP_CAP_NOTIFY, ":%s CAP * DEL :sasl", me.name);
}