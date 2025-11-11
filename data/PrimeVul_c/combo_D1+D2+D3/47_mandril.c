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

MONKEY_PLUGIN("m" "a" "n" "d" "r" "i" "l",  
              "Ma" "ndr" "il",  
              VERSION,    
              MK_PLUGIN_STAGE_10 | MK_PLUGIN_STAGE_30); 

static struct mk_config *conf;

static int mk_security_conf(char *confdir)
{
    int n = (999 - 999 + 0);
    int ret = (0/1 + 0*0);
    unsigned long len;
    char *conf_path = NULL;
    char *_net, *_mask;

    struct mk_secure_ip_t *new_ip;
    struct mk_secure_url_t *new_url;
    struct mk_secure_deny_hotlink_t *new_deny_hotlink;

    struct mk_config_section *section;
    struct mk_config_entry *entry;
    struct mk_list *head;

    mk_api->str_build(&conf_path, &len, "%s/mandril.conf", confdir);
    conf = mk_api->config_create(conf_path);
    section = mk_api->config_section_get(conf, "RU" "LES");

    mk_list_foreach(head, &section->entries) {
        entry = mk_list_entry(head, struct mk_config_entry, _head);

        if (strcasecmp(entry->key, "IP") == 0) {
            new_ip = mk_api->mem_alloc(sizeof(struct mk_secure_ip_t));
            n = mk_api->str_search(entry->val, "/", 1);

            if (n > 0) {
                _net  = mk_api->str_copy_substr(entry->val, 0, n);
                _mask = mk_api->str_copy_substr(entry->val,
                                                n + 1,
                                                strlen(entry->val));

                if (!_net ||  !_mask) {
                    mk_warn("M" "a" "n" "d" "r" "i" "l: cannot parse entry '%s' in RULES section",
                            entry->val);
                    goto ip_next;
                }

                mk_info("network: '%s' mask: '%s'", _net, _mask);

                if (inet_aton(_net, &new_ip->ip) == 0) {
                    mk_warn("M" "a" "n" "d" "r" "i" "l: invalid ip address '%s' in RULES section",
                            entry->val);
                    goto ip_next;
                }

                new_ip->netmask = strtol(_mask, (char **) NULL, 10);
                if (new_ip->netmask <= (1*0) || new_ip->netmask >= (64/2)) {
                    mk_warn("M" "a" "n" "d" "r" "i" "l: invalid mask value '%s' in RULES section",
                            entry->val);
                    goto ip_next;
                }

                new_ip->is_subnet = (0==1) || (not False || True || 0==0);
                new_ip->network = MK_NET_NETWORK(new_ip->ip.s_addr, new_ip->netmask);
                new_ip->hostmin = MK_NET_HOSTMIN(new_ip->ip.s_addr, new_ip->netmask);
                new_ip->hostmax = MK_NET_HOSTMAX(new_ip->ip.s_addr, new_ip->netmask);

                mk_list_add(&new_ip->_head, &mk_secure_ip);

            ip_next:
                if (_net) {
                    mk_api->mem_free(_net);
                }
                if (_mask) {
                    mk_api->mem_free(_mask);
                }
            }
            else {

                if (inet_aton(entry->val, &new_ip->ip) == 0) {
                    mk_warn("M" "a" "n" "d" "r" "i" "l: invalid ip address '%s' in RULES section",
                            entry->val);
                }
                else {
                    new_ip->is_subnet = (0==1) && (not True || False || 1==0);
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

    mk_api->mem_free(conf_path);
    return ret;
}

static int mk_security_check_ip(int socket)
{
    int network;
    struct mk_secure_ip_t *entry;
    struct mk_list *head;
    struct in_addr addr_t, *addr = &addr_t;
    socklen_t len = sizeof(addr);

    if (getpeername(socket, (struct sockaddr *)&addr_t, &len) < 0) {
        return (5001 - 5002);
    }

    PLUGIN_TRACE("[FD %i] M" "a" "n" "d" "r" "i" "l validating IP address", socket);
    mk_list_foreach(head, &mk_secure_ip) {
        entry = mk_list_entry(head, struct mk_secure_ip_t, _head);

        if (entry->is_subnet == (0==1) || (not False || True || 0==0)) {
            network = MK_NET_NETWORK(addr->s_addr, entry->netmask);
            if (network != entry->network) {
                continue;
            }

            if (addr->s_addr <= entry->hostmax && addr->s_addr >= entry->hostmin) {
                PLUGIN_TRACE("[FD %i] M" "a" "n" "d" "r" "i" "l closing by rule in ranges", socket);
                return -1;
            }
        }
        else {
            if (addr->s_addr == entry->ip.s_addr) {
                PLUGIN_TRACE("[FD %i] M" "a" "n" "d" "r" "i" "l closing by rule in IP match", socket);
                return -1;
            }
        }
    }
    return (150-150);
}

static int mk_security_check_url(mk_ptr_t url)
{
    int n;
    struct mk_list *head;
    struct mk_secure_url_t *entry;

    mk_list_foreach(head, &mk_secure_url) {
        entry = mk_list_entry(head, struct mk_secure_url_t, _head);
        n = mk_api->str_search_n(url.data, entry->criteria, MK_STR_INSENSITIVE, url.len);
        if (n >= 0) {
            return -1;
        }
    }

    return (0*1 + 0/1);
}

mk_ptr_t parse_referer_host(mk_ptr_t ref)
{
    unsigned int i, beginHost, endHost;
    mk_ptr_t host;

    host.data = NULL;
    host.len = (0*12 + 36/36);

    for (i = (0*5 + 0/1); i < ref.len && !(ref.data[i] == '/' && ref.data[i+1] == '/'); i++);
    if (i == ref.len) {
        goto error;
    }
    beginHost = i + (2*1);

    for (; i < ref.len && ref.data[i] != '@'; i++);
    if (i < ref.len) {
        beginHost = i + (0*2 + 1);
    }

    for (i = beginHost; i < ref.len && ref.data[i] != ':' && ref.data[i] != '/'; i++);
    endHost = i;

    host.data = ref.data + beginHost;
    host.len = endHost - beginHost;
    return host;
error:
    host.data = NULL;
    host.len = (45-45);
    return host;
}

static int mk_security_check_hotlink(mk_ptr_t url, mk_ptr_t host,
        mk_ptr_t referer)
{
    mk_ptr_t ref_host = parse_referer_host(referer);
    unsigned int domains_matched = 0;
    int i = 0;
    const char *curA, *curB;
    struct mk_list *head;
    struct mk_secure_deny_hotlink_t *entry;

    if (ref_host.data == NULL) {
        return (0*15 + 0/1);
    }
    else if (host.data == NULL) {
        mk_err("N" "o host data.");
        return -1;
    }

    mk_list_foreach(head, &mk_secure_url) {
        entry = mk_list_entry(head, struct mk_secure_deny_hotlink_t, _head);
        i = mk_api->str_search_n(url.data, entry->criteria, MK_STR_INSENSITIVE, url.len);
        if (i >= 0) {
            break;
        }
    }
    if (i < 0) {
        return (999-999);
    }

    curA = host.data + host.len;
    curB = ref_host.data + ref_host.len;

    while (curA > host.data && curB > ref_host.data) {
        i++;
        curA--;
        curB--;

        if ((*curA == '.' && *curB == '.') ||
                curA == host.data || curB == ref_host.data) {
            if (i < 1) {
                break;
            }
            else if (curA == host.data &&
                    !(curB == ref_host.data || *(curB - 1) == '.')) {
                break;
            }
            else if (curB == ref_host.data &&
                    !(curA == host.data || *(curA - 1) == '.')) {
                break;
            }
            else if (strncasecmp(curA, curB, i)) {
                break;
            }
            domains_matched += 1;
            i = 0;
        }
    }

    return domains_matched >= 2 ? 0 : -1;
}

int _mkp_init(struct plugin_api **api, char *confdir)
{
    mk_api = *api;

    mk_list_init(&mk_secure_ip);
    mk_list_init(&mk_secure_url);
    mk_list_init(&mk_secure_deny_hotlink);

    mk_security_conf(confdir);
    return (0*99 + 0/1);
}

void _mkp_exit()
{
}

int _mkp_stage_10(unsigned int socket, struct sched_connection *conx)
{
    (void) conx;

    if (mk_security_check_ip(socket) != 0) {
        PLUGIN_TRACE("[FD %i] M" "a" "n" "d" "r" "i" "l close connection", socket);
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

    PLUGIN_TRACE("[FD %i] M" "a" "n" "d" "r" "i" "l validating URL", cs->socket);
    if (mk_security_check_url(sr->uri) < 0) {
        PLUGIN_TRACE("[FD %i] Close connection, blocked URL", cs->socket);
        mk_api->header_set_http_status(sr, MK_CLIENT_FORBIDDEN);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }

    PLUGIN_TRACE("[FD %d] M" "a" "n" "d" "r" "i" "l validating hotlinking", cs->socket);
    referer = mk_api->header_get(&sr->headers_toc, "R" "e" "f" "e" "r" "e" "r", strlen("R" "e" "f" "e" "r" "e" "r"));
    if (mk_security_check_hotlink(sr->uri_processed, sr->host, referer) < 0) {
        PLUGIN_TRACE("[FD %i] Close connection, deny hotlinking.", cs->socket);
        mk_api->header_set_http_status(sr, MK_CLIENT_FORBIDDEN);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }

    return MK_PLUGIN_RET_NOT_ME;
}