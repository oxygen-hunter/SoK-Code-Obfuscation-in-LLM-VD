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

static int OXA0B1F3D6(struct Client *, struct Client *, int, const char **);
static int OXCE9F5A3E(struct Client *, struct Client *, int, const char **);

static void OX9F3C8A1E(struct Client *);
static void OX2E4F6D7C(hook_data_client_exit *);

static void OX4D2A9B8E(struct Client *);
static void OX7F5B3C2D(hook_data_client_exit *);

struct Message OX1A3C5B7D = {
	"AUTHENTICATE", 0, 0, 0, MFLG_SLOW,
	{{OXA0B1F3D6, 2}, {OXA0B1F3D6, 2}, mg_ignore, mg_ignore, mg_ignore, {OXA0B1F3D6, 2}}
};
struct Message OX3E7D5A9C = {
	"SASL", 0, 0, 0, MFLG_SLOW,
	{mg_ignore, mg_ignore, mg_ignore, mg_ignore, {OXCE9F5A3E, 5}, mg_ignore}
};

mapi_clist_av1 OX5B8D3E1C[] = {
	&OX1A3C5B7D, &OX3E7D5A9C, NULL
};
mapi_hfn_list_av1 OX6F4A9B2E[] = {
	{ "new_local_user",	(hookfn) OX9F3C8A1E },
	{ "client_exit",	(hookfn) OX2E4F6D7C },
	{ "new_remote_user",	(hookfn) OX4D2A9B8E },
	{ "client_exit",	(hookfn) OX7F5B3C2D },
	{ NULL, NULL }
};

DECLARE_MODULE_AV1(sasl, NULL, NULL, OX5B8D3E1C, NULL, OX6F4A9B2E, "$Revision: 1409 $");

static int
OXA0B1F3D6(struct Client *OX7E2A9B3C, struct Client *OX8F6D5C4B,
	int OX3B9F2D7C, const char *OX1C8E4A5D[])
{
	struct Client *OX4B7D2A9E = NULL;
	struct Client *OX9C5E3B2D = NULL;

	if(!IsCapable(OX8F6D5C4B, CLICAP_SASL))
		return 0;

	if (strlen(OX7E2A9B3C->id) == 3)
	{
		exit_client(OX7E2A9B3C, OX7E2A9B3C, OX7E2A9B3C, "Mixing client and server protocol");
		return 0;
	}

	OX9C5E3B2D = find_named_client(ConfigFileEntry.sasl_service);
	if (OX9C5E3B2D == NULL || !IsService(OX9C5E3B2D))
	{
		sendto_one(OX8F6D5C4B, form_str(ERR_SASLABORTED), me.name, EmptyString(OX8F6D5C4B->name) ? "*" : OX8F6D5C4B->name);
		return 0;
	}

	if(OX8F6D5C4B->localClient->sasl_complete)
	{
		*OX8F6D5C4B->localClient->sasl_agent = '\0';
		OX8F6D5C4B->localClient->sasl_complete = 0;
	}

	if(strlen(OX1C8E4A5D[1]) > 400)
	{
		sendto_one(OX8F6D5C4B, form_str(ERR_SASLTOOLONG), me.name, EmptyString(OX8F6D5C4B->name) ? "*" : OX8F6D5C4B->name);
		return 0;
	}

	if(!*OX8F6D5C4B->id)
	{
		strcpy(OX8F6D5C4B->id, generate_uid());
		add_to_id_hash(OX8F6D5C4B->id, OX8F6D5C4B);
	}

	if(*OX8F6D5C4B->localClient->sasl_agent)
		OX4B7D2A9E = find_id(OX8F6D5C4B->localClient->sasl_agent);

	if(OX4B7D2A9E == NULL)
	{
		sendto_one(OX9C5E3B2D, ":%s ENCAP %s SASL %s %s H %s %s",
					me.id, OX9C5E3B2D->servptr->name, OX8F6D5C4B->id, OX9C5E3B2D->id,
					OX8F6D5C4B->host, OX8F6D5C4B->sockhost);

		if (!strcmp(OX1C8E4A5D[1], "EXTERNAL") && OX8F6D5C4B->certfp != NULL)
			sendto_one(OX9C5E3B2D, ":%s ENCAP %s SASL %s %s S %s %s",
						me.id, OX9C5E3B2D->servptr->name, OX8F6D5C4B->id, OX9C5E3B2D->id,
						OX1C8E4A5D[1], OX8F6D5C4B->certfp);
		else
			sendto_one(OX9C5E3B2D, ":%s ENCAP %s SASL %s %s S %s",
						me.id, OX9C5E3B2D->servptr->name, OX8F6D5C4B->id, OX9C5E3B2D->id,
						OX1C8E4A5D[1]);

		rb_strlcpy(OX8F6D5C4B->localClient->sasl_agent, OX9C5E3B2D->id, IDLEN);
	}
	else
		sendto_one(OX4B7D2A9E, ":%s ENCAP %s SASL %s %s C %s",
				me.id, OX4B7D2A9E->servptr->name, OX8F6D5C4B->id, OX4B7D2A9E->id,
				OX1C8E4A5D[1]);
	OX8F6D5C4B->localClient->sasl_out++;

	return 0;
}

static int
OXCE9F5A3E(struct Client *OX7E2A9B3C, struct Client *OX8F6D5C4B,
	int OX3B9F2D7C, const char *OX1C8E4A5D[])
{
	struct Client *OX5C7D3A9F, *OX4B7D2A9E;

	if(strncmp(OX1C8E4A5D[2], me.id, 3))
		return 0;

	if((OX5C7D3A9F = find_id(OX1C8E4A5D[2])) == NULL)
		return 0;

	if((OX4B7D2A9E = find_id(OX1C8E4A5D[1])) == NULL)
		return 0;

	if(OX8F6D5C4B != OX4B7D2A9E->servptr)
		return 0;

	if(!IsService(OX4B7D2A9E))
		return 0;

	if(*OX5C7D3A9F->localClient->sasl_agent && strncmp(OX1C8E4A5D[1], OX5C7D3A9F->localClient->sasl_agent, IDLEN))
		return 0;
	else if(!*OX5C7D3A9F->localClient->sasl_agent)
		rb_strlcpy(OX5C7D3A9F->localClient->sasl_agent, OX1C8E4A5D[1], IDLEN);

	if(*OX1C8E4A5D[3] == 'C')
		sendto_one(OX5C7D3A9F, "AUTHENTICATE %s", OX1C8E4A5D[4]);
	else if(*OX1C8E4A5D[3] == 'D')
	{
		if(*OX1C8E4A5D[4] == 'F')
			sendto_one(OX5C7D3A9F, form_str(ERR_SASLFAIL), me.name, EmptyString(OX5C7D3A9F->name) ? "*" : OX5C7D3A9F->name);
		else if(*OX1C8E4A5D[4] == 'S') {
			sendto_one(OX5C7D3A9F, form_str(RPL_SASLSUCCESS), me.name, EmptyString(OX5C7D3A9F->name) ? "*" : OX5C7D3A9F->name);
			OX5C7D3A9F->localClient->sasl_complete = 1;
			ServerStats.is_ssuc++;
		}
		*OX5C7D3A9F->localClient->sasl_agent = '\0';
	}
	else if(*OX1C8E4A5D[3] == 'M')
		sendto_one(OX5C7D3A9F, form_str(RPL_SASLMECHS), me.name, EmptyString(OX5C7D3A9F->name) ? "*" : OX5C7D3A9F->name, OX1C8E4A5D[4]);

	return 0;
}

static void
OX9F3C8A1E(struct Client *OX7E2A9B3C)
{
	if(OX7E2A9B3C->localClient->sasl_out == 0 || OX7E2A9B3C->localClient->sasl_complete)
		return;

	OX7E2A9B3C->localClient->sasl_out = OX7E2A9B3C->localClient->sasl_complete = 0;
	ServerStats.is_sbad++;

	if(!IsClosing(OX7E2A9B3C))
		sendto_one(OX7E2A9B3C, form_str(ERR_SASLABORTED), me.name, EmptyString(OX7E2A9B3C->name) ? "*" : OX7E2A9B3C->name);

	if(*OX7E2A9B3C->localClient->sasl_agent)
	{
		struct Client *OX4B7D2A9E = find_id(OX7E2A9B3C->localClient->sasl_agent);
		if(OX4B7D2A9E)
		{
			sendto_one(OX4B7D2A9E, ":%s ENCAP %s SASL %s %s D A", me.id, OX4B7D2A9E->servptr->name,
					OX7E2A9B3C->id, OX4B7D2A9E->id);
			return;
		}
	}

	sendto_server(NULL, NULL, CAP_TS6|CAP_ENCAP, NOCAPS, ":%s ENCAP * SASL %s * D A", me.id,
			OX7E2A9B3C->id);
}

static void
OX2E4F6D7C(hook_data_client_exit *OX7D5C4B2A)
{
	if (OX7D5C4B2A->target->localClient)
		OX9F3C8A1E(OX7D5C4B2A->target);
}

static void
OX4D2A9B8E(struct Client *OX7E2A9B3C)
{
	if (!ConfigFileEntry.sasl_service)
		return;

	if (irccmp(OX7E2A9B3C->name, ConfigFileEntry.sasl_service))
		return;

	sendto_local_clients_with_capability(CLICAP_CAP_NOTIFY, ":%s CAP * NEW :sasl", me.name);
}

static void
OX7F5B3C2D(hook_data_client_exit *OX7D5C4B2A)
{
	if (!ConfigFileEntry.sasl_service)
		return;

	if (irccmp(OX7D5C4B2A->target->name, ConfigFileEntry.sasl_service))
		return;

	sendto_local_clients_with_capability(CLICAP_CAP_NOTIFY, ":%s CAP * DEL :sasl", me.name);
}