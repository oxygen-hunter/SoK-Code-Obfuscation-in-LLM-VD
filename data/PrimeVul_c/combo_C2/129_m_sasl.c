static int
m_authenticate(struct Client *client_p, struct Client *source_p,
	int parc, const char *parv[])
{
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0: {
				struct Client *agent_p = NULL;
				struct Client *saslserv_p = NULL;

				if(!IsCapable(source_p, CLICAP_SASL)) {
					dispatcher = 1;
					break;
				}

				if (strlen(client_p->id) == 3) {
					exit_client(client_p, client_p, client_p, "Mixing client and server protocol");
					dispatcher = 1;
					break;
				}

				saslserv_p = find_named_client(ConfigFileEntry.sasl_service);
				if (saslserv_p == NULL || !IsService(saslserv_p)) {
					sendto_one(source_p, form_str(ERR_SASLABORTED), me.name, EmptyString(source_p->name) ? "*" : source_p->name);
					dispatcher = 1;
					break;
				}

				if(source_p->localClient->sasl_complete) {
					*source_p->localClient->sasl_agent = '\0';
					source_p->localClient->sasl_complete = 0;
				}

				if(strlen(parv[1]) > 400) {
					sendto_one(source_p, form_str(ERR_SASLTOOLONG), me.name, EmptyString(source_p->name) ? "*" : source_p->name);
					dispatcher = 1;
					break;
				}

				if(!*source_p->id) {
					strcpy(source_p->id, generate_uid());
					add_to_id_hash(source_p->id, source_p);
				}

				if(*source_p->localClient->sasl_agent)
					agent_p = find_id(source_p->localClient->sasl_agent);

				if(agent_p == NULL) {
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
				} else
					sendto_one(agent_p, ":%s ENCAP %s SASL %s %s C %s",
							me.id, agent_p->servptr->name, source_p->id, agent_p->id,
							parv[1]);
				source_p->localClient->sasl_out++;
				dispatcher = 1;
				break;
			}
			case 1:
				return 0;
		}
	}
}

static int
me_sasl(struct Client *client_p, struct Client *source_p,
	int parc, const char *parv[])
{
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0: {
				struct Client *target_p, *agent_p;

				if(strncmp(parv[2], me.id, 3)) {
					dispatcher = 1;
					break;
				}

				if((target_p = find_id(parv[2])) == NULL) {
					dispatcher = 1;
					break;
				}

				if((agent_p = find_id(parv[1])) == NULL) {
					dispatcher = 1;
					break;
				}

				if(source_p != agent_p->servptr) {
					dispatcher = 1;
					break;
				}

				if(!IsService(agent_p)) {
					dispatcher = 1;
					break;
				}

				if(*target_p->localClient->sasl_agent && strncmp(parv[1], target_p->localClient->sasl_agent, IDLEN)) {
					dispatcher = 1;
					break;
				} else if(!*target_p->localClient->sasl_agent)
					rb_strlcpy(target_p->localClient->sasl_agent, parv[1], IDLEN);

				if(*parv[3] == 'C')
					sendto_one(target_p, "AUTHENTICATE %s", parv[4]);
				else if(*parv[3] == 'D') {
					if(*parv[4] == 'F')
						sendto_one(target_p, form_str(ERR_SASLFAIL), me.name, EmptyString(target_p->name) ? "*" : target_p->name);
					else if(*parv[4] == 'S') {
						sendto_one(target_p, form_str(RPL_SASLSUCCESS), me.name, EmptyString(target_p->name) ? "*" : target_p->name);
						target_p->localClient->sasl_complete = 1;
						ServerStats.is_ssuc++;
					}
					*target_p->localClient->sasl_agent = '\0';
				} else if(*parv[3] == 'M')
					sendto_one(target_p, form_str(RPL_SASLMECHS), me.name, EmptyString(target_p->name) ? "*" : target_p->name, parv[4]);
				dispatcher = 1;
				break;
			}
			case 1:
				return 0;
		}
	}
}

static void
abort_sasl(struct Client *data)
{
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if(data->localClient->sasl_out == 0 || data->localClient->sasl_complete) {
					dispatcher = 1;
					break;
				}

				data->localClient->sasl_out = data->localClient->sasl_complete = 0;
				ServerStats.is_sbad++;

				if(!IsClosing(data))
					sendto_one(data, form_str(ERR_SASLABORTED), me.name, EmptyString(data->name) ? "*" : data->name);

				if(*data->localClient->sasl_agent) {
					struct Client *agent_p = find_id(data->localClient->sasl_agent);
					if(agent_p) {
						sendto_one(agent_p, ":%s ENCAP %s SASL %s %s D A", me.id, agent_p->servptr->name,
								data->id, agent_p->id);
						dispatcher = 1;
						break;
					}
				}

				sendto_server(NULL, NULL, CAP_TS6|CAP_ENCAP, NOCAPS, ":%s ENCAP * SASL %s * D A", me.id,
						data->id);
				dispatcher = 1;
				break;
			case 1:
				return;
		}
	}
}

static void
abort_sasl_exit(hook_data_client_exit *data)
{
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if (data->target->localClient)
					abort_sasl(data->target);
				dispatcher = 1;
				break;
			case 1:
				return;
		}
	}
}

static void
advertise_sasl(struct Client *client_p)
{
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if (!ConfigFileEntry.sasl_service) {
					dispatcher = 1;
					break;
				}

				if (irccmp(client_p->name, ConfigFileEntry.sasl_service)) {
					dispatcher = 1;
					break;
				}

				sendto_local_clients_with_capability(CLICAP_CAP_NOTIFY, ":%s CAP * NEW :sasl", me.name);
				dispatcher = 1;
				break;
			case 1:
				return;
		}
	}
}

static void
advertise_sasl_exit(hook_data_client_exit *data)
{
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if (!ConfigFileEntry.sasl_service) {
					dispatcher = 1;
					break;
				}

				if (irccmp(data->target->name, ConfigFileEntry.sasl_service)) {
					dispatcher = 1;
					break;
				}

				sendto_local_clients_with_capability(CLICAP_CAP_NOTIFY, ":%s CAP * DEL :sasl", me.name);
				dispatcher = 1;
				break;
			case 1:
				return;
		}
	}
}