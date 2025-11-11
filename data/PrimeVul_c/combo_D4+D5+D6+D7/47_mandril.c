#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* network */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Monkey API */
#include "MKPlugin.h"
#include "mandril.h"

MONKEY_PLUGIN("mandril",  /* shortname */
              "Mandril",  /* name */
              VERSION,    /* version */
              MK_PLUGIN_STAGE_10 | MK_PLUGIN_STAGE_30); /* hooks */

struct {
    struct mk_config *a;
} g_conf;

/* Read database configuration parameters */
static int mk_security_conf(char *confdir)
{
    struct {
        int a;
        int b;
        unsigned long c;
        char *d;
        char *e;
        char *f;
    } vars = {0};

    struct mk_secure_ip_t *new_ip;
    struct mk_secure_url_t *new_url;
    struct mk_secure_deny_hotlink_t *new_deny_hotlink;

    struct mk_config_section *section;
    struct mk_config_entry *entry;
    struct mk_list *head;

    /* Read configuration */
    mk_api->str_build(&vars.d, &vars.c, "%s/mandril.conf", confdir);
    g_conf.a = mk_api->config_create(vars.d);
    section = mk_api->config_section_get(g_conf.a, "RULES");

    mk_list_foreach(head, &section->entries) {
        entry = mk_list_entry(head, struct mk_config_entry, _head);

        if (strcasecmp(entry->key, "IP") == 0) {
            new_ip = mk_api->mem_alloc(sizeof(struct mk_secure_ip_t));
            vars.a = mk_api->str_search(entry->val, "/", 1);

            if (vars.a > 0) {
                vars.e = mk_api->str_copy_substr(entry->val, 0, vars.a);
                vars.f = mk_api->str_copy_substr(entry->val,
                                                vars.a + 1,
                                                strlen(entry->val));

                if (!vars.e ||  !vars.f) {
                    mk_warn("Mandril: cannot parse entry '%s' in RULES section",
                            entry->val);
                    goto ip_next;
                }

                mk_info("network: '%s' mask: '%s'", vars.e, vars.f);

                if (inet_aton(vars.e, &new_ip->ip) == 0) {
                    mk_warn("Mandril: invalid ip address '%s' in RULES section",
                            entry->val);
                    goto ip_next;
                }

                new_ip->netmask = strtol(vars.f, (char **) NULL, 10);
                if (new_ip->netmask <= 0 || new_ip->netmask >= 32) {
                    mk_warn("Mandril: invalid mask value '%s' in RULES section",
                            entry->val);
                    goto ip_next;
                }

                new_ip->is_subnet = MK_TRUE;
                new_ip->network = MK_NET_NETWORK(new_ip->ip.s_addr, new_ip->netmask);
                new_ip->hostmin = MK_NET_HOSTMIN(new_ip->ip.s_addr, new_ip->netmask);
                new_ip->hostmax = MK_NET_HOSTMAX(new_ip->ip.s_addr, new_ip->netmask);

                mk_list_add(&new_ip->_head, &mk_secure_ip);

            ip_next:
                if (vars.e) {
                    mk_api->mem_free(vars.e);
                }
                if (vars.f) {
                    mk_api->mem_free(vars.f);
                }
            }
            else {
                if (inet_aton(entry->val, &new_ip->ip) == 0) {
                    mk_warn("Mandril: invalid ip address '%s' in RULES section",
                            entry->val);
                }
                else {
                    new_ip->is_subnet = MK_FALSE;
                    mk_list_add(&new_ip->_head, &mk_secure_ip);
                }
            }
        }
        else if (strcasecmp(entry->key, "URL") == 0) {
            new_url = mk_api->mem_alloc(sizeof(struct mk_secure_url_t));
            new_url->criteria = entry->val;

            mk_list_add(&new_url->_head, &mk_secure_url);
        }
        else if (strcasecmp(entry->key, "deny_hotlink") == 0) {
            new_deny_hotlink = mk_api->mem_alloc(sizeof(*new_deny_hotlink));
            new_deny_hotlink->criteria = entry->val;

            mk_list_add(&new_deny_hotlink->_head, &mk_secure_deny_hotlink);
        }
    }

    mk_api->mem_free(vars.d);
    return vars.b;
}

static int mk_security_check_ip(int socket)
{
    struct {
        int a;
        struct in_addr b, *c;
        socklen_t d;
    } vars = {0};

    struct mk_secure_ip_t *entry;
    struct mk_list *head;
    vars.c = &vars.b;
    vars.d = sizeof(vars.c);

    if (getpeername(socket, (struct sockaddr *)&vars.b, &vars.d) < 0) {
        return -1;
    }

    PLUGIN_TRACE("[FD %i] Mandril validating IP address", socket);
    mk_list_foreach(head, &mk_secure_ip) {
        entry = mk_list_entry(head, struct mk_secure_ip_t, _head);

        if (entry->is_subnet == MK_TRUE) {
            vars.a = MK_NET_NETWORK(vars.c->s_addr, entry->netmask);
            if (vars.a != entry->network) {
                continue;
            }

            if (vars.c->s_addr <= entry->hostmax && vars.c->s_addr >= entry->hostmin) {
                PLUGIN_TRACE("[FD %i] Mandril closing by rule in ranges", socket);
                return -1;
            }
        }
        else {
            if (vars.c->s_addr == entry->ip.s_addr) {
                PLUGIN_TRACE("[FD %i] Mandril closing by rule in IP match", socket);
                return -1;
            }
        }
    }
    return 0;
}

static int mk_security_check_url(mk_ptr_t url)
{
    struct {
        int a;
    } vars = {0};

    struct mk_list *head;
    struct mk_secure_url_t *entry;

    mk_list_foreach(head, &mk_secure_url) {
        entry = mk_list_entry(head, struct mk_secure_url_t, _head);
        vars.a = mk_api->str_search_n(url.data, entry->criteria, MK_STR_INSENSITIVE, url.len);
        if (vars.a >= 0) {
            return -1;
        }
    }

    return 0;
}

mk_ptr_t parse_referer_host(mk_ptr_t ref)
{
    struct {
        unsigned int a, b, c;
        mk_ptr_t d;
    } vars = {0};

    vars.d.data = NULL;
    vars.d.len = 0;

    for (vars.a = 0; vars.a < ref.len && !(ref.data[vars.a] == '/' && ref.data[vars.a+1] == '/'); vars.a++);
    if (vars.a == ref.len) {
        goto error;
    }
    vars.b = vars.a + 2;

    for (; vars.a < ref.len && ref.data[vars.a] != '@'; vars.a++);
    if (vars.a < ref.len) {
        vars.b = vars.a + 1;
    }

    for (vars.a = vars.b; vars.a < ref.len && ref.data[vars.a] != ':' && ref.data[vars.a] != '/'; vars.a++);
    vars.c = vars.a;

    vars.d.data = ref.data + vars.b;
    vars.d.len = vars.c - vars.b;
    return vars.d;
error:
    vars.d.data = NULL;
    vars.d.len = 0;
    return vars.d;
}

static int mk_security_check_hotlink(mk_ptr_t url, mk_ptr_t host,
        mk_ptr_t referer)
{
    struct {
        mk_ptr_t a;
        unsigned int b;
        int c;
        const char *d, *e;
    } vars = {parse_referer_host(referer), 0, 0, NULL, NULL};

    struct mk_list *head;
    struct mk_secure_deny_hotlink_t *entry;

    if (vars.a.data == NULL) {
        return 0;
    }
    else if (host.data == NULL) {
        mk_err("No host data.");
        return -1;
    }

    mk_list_foreach(head, &mk_secure_url) {
        entry = mk_list_entry(head, struct mk_secure_deny_hotlink_t, _head);
        vars.c = mk_api->str_search_n(url.data, entry->criteria, MK_STR_INSENSITIVE, url.len);
        if (vars.c >= 0) {
            break;
        }
    }
    if (vars.c < 0) {
        return 0;
    }

    vars.d = host.data + host.len;
    vars.e = vars.a.data + vars.a.len;

    while (vars.d > host.data && vars.e > vars.a.data) {
        vars.c++;
        vars.d--;
        vars.e--;

        if ((*vars.d == '.' && *vars.e == '.') ||
                vars.d == host.data || vars.e == vars.a.data) {
            if (vars.c < 1) {
                break;
            }
            else if (vars.d == host.data &&
                    !(vars.e == vars.a.data || *(vars.e - 1) == '.')) {
                break;
            }
            else if (vars.e == vars.a.data &&
                    !(vars.d == host.data || *(vars.d - 1) == '.')) {
                break;
            }
            else if (strncasecmp(vars.d, vars.e, vars.c)) {
                break;
            }
            vars.b += 1;
            vars.c = 0;
        }
    }

    return vars.b >= 2 ? 0 : -1;
}

int _mkp_init(struct plugin_api **api, char *confdir)
{
    mk_api = *api;

    mk_list_init(&mk_secure_ip);
    mk_list_init(&mk_secure_url);
    mk_list_init(&mk_secure_deny_hotlink);

    mk_security_conf(confdir);
    return 0;
}

void _mkp_exit()
{
}

int _mkp_stage_10(unsigned int socket, struct sched_connection *conx)
{
    (void) conx;

    if (mk_security_check_ip(socket) != 0) {
        PLUGIN_TRACE("[FD %i] Mandril close connection", socket);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }
    return MK_PLUGIN_RET_CONTINUE;
}

int _mkp_stage_30(struct plugin *p,
        struct client_session *cs,
        struct session_request *sr)
{
    mk_ptr_t referer;
    (void) p;
    (void) cs;

    PLUGIN_TRACE("[FD %i] Mandril validating URL", cs->socket);
    if (mk_security_check_url(sr->uri) < 0) {
        PLUGIN_TRACE("[FD %i] Close connection, blocked URL", cs->socket);
        mk_api->header_set_http_status(sr, MK_CLIENT_FORBIDDEN);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }

    PLUGIN_TRACE("[FD %d] Mandril validating hotlinking", cs->socket);
    referer = mk_api->header_get(&sr->headers_toc, "Referer", strlen("Referer"));
    if (mk_security_check_hotlink(sr->uri_processed, sr->host, referer) < 0) {
        PLUGIN_TRACE("[FD %i] Close connection, deny hotlinking.", cs->socket);
        mk_api->header_set_http_status(sr, MK_CLIENT_FORBIDDEN);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }

    return MK_PLUGIN_RET_NOT_ME;
}