#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "MKPlugin.h"
#include "mandril.h"

MONKEY_PLUGIN("mandril", "Mandril", VERSION, MK_PLUGIN_STAGE_10 | MK_PLUGIN_STAGE_30);

static struct mk_config *OX7D4A37A8;

static int OX6C3F2B52(char *OX3F2A8B9A)
{
    int OX5C6F9B8D;
    int OX5C3C9C2B = 0;
    unsigned long OX1F3B5A8D;
    char *OX6F3E8A3F = NULL;
    char *OX4C2D7A4F, *OX5E3F8A1C;

    struct OX5A6C9F2D *OX7B3C8A4F;
    struct OX2F4E7B9D *OX8F3A5D6C;
    struct OX6E2F8B7A *OX3D4A9F5B;

    struct mk_config_section *OX2F3D8A6C;
    struct mk_config_entry *OX4F8C5A2D;
    struct mk_list *OX6D2B9A5F;

    mk_api->str_build(&OX6F3E8A3F, &OX1F3B5A8D, "%s/mandril.conf", OX3F2A8B9A);
    OX7D4A37A8 = mk_api->config_create(OX6F3E8A3F);
    OX2F3D8A6C = mk_api->config_section_get(OX7D4A37A8, "RULES");

    mk_list_foreach(OX6D2B9A5F, &OX2F3D8A6C->entries) {
        OX4F8C5A2D = mk_list_entry(OX6D2B9A5F, struct mk_config_entry, _head);

        if (strcasecmp(OX4F8C5A2D->key, "IP") == 0) {
            OX7B3C8A4F = mk_api->mem_alloc(sizeof(struct OX5A6C9F2D));
            OX5C6F9B8D = mk_api->str_search(OX4F8C5A2D->val, "/", 1);

            if (OX5C6F9B8D > 0) {
                OX4C2D7A4F  = mk_api->str_copy_substr(OX4F8C5A2D->val, 0, OX5C6F9B8D);
                OX5E3F8A1C = mk_api->str_copy_substr(OX4F8C5A2D->val, OX5C6F9B8D + 1, strlen(OX4F8C5A2D->val));

                if (!OX4C2D7A4F ||  !OX5E3F8A1C) {
                    mk_warn("Mandril: cannot parse entry '%s' in RULES section", OX4F8C5A2D->val);
                    goto OX7A4D3F2C;
                }

                mk_info("network: '%s' mask: '%s'", OX4C2D7A4F, OX5E3F8A1C);

                if (inet_aton(OX4C2D7A4F, &OX7B3C8A4F->ip) == 0) {
                    mk_warn("Mandril: invalid ip address '%s' in RULES section", OX4F8C5A2D->val);
                    goto OX7A4D3F2C;
                }

                OX7B3C8A4F->netmask = strtol(OX5E3F8A1C, (char **) NULL, 10);
                if (OX7B3C8A4F->netmask <= 0 || OX7B3C8A4F->netmask >= 32) {
                    mk_warn("Mandril: invalid mask value '%s' in RULES section", OX4F8C5A2D->val);
                    goto OX7A4D3F2C;
                }

                OX7B3C8A4F->is_subnet = MK_TRUE;
                OX7B3C8A4F->network = MK_NET_NETWORK(OX7B3C8A4F->ip.s_addr, OX7B3C8A4F->netmask);
                OX7B3C8A4F->hostmin = MK_NET_HOSTMIN(OX7B3C8A4F->ip.s_addr, OX7B3C8A4F->netmask);
                OX7B3C8A4F->hostmax = MK_NET_HOSTMAX(OX7B3C8A4F->ip.s_addr, OX7B3C8A4F->netmask);

                mk_list_add(&OX7B3C8A4F->_head, &mk_secure_ip);

            OX7A4D3F2C:
                if (OX4C2D7A4F) {
                    mk_api->mem_free(OX4C2D7A4F);
                }
                if (OX5E3F8A1C) {
                    mk_api->mem_free(OX5E3F8A1C);
                }
            }
            else {
                if (inet_aton(OX4F8C5A2D->val, &OX7B3C8A4F->ip) == 0) {
                    mk_warn("Mandril: invalid ip address '%s' in RULES section", OX4F8C5A2D->val);
                }
                else {
                    OX7B3C8A4F->is_subnet = MK_FALSE;
                    mk_list_add(&OX7B3C8A4F->_head, &mk_secure_ip);
                }
            }
        }
        else if (strcasecmp(OX4F8C5A2D->key, "URL") == 0) {
            OX8F3A5D6C = mk_api->mem_alloc(sizeof(struct OX2F4E7B9D));
            OX8F3A5D6C->criteria = OX4F8C5A2D->val;

            mk_list_add(&OX8F3A5D6C->_head, &mk_secure_url);
        }
        else if (strcasecmp(OX4F8C5A2D->key, "deny_hotlink") == 0) {
            OX3D4A9F5B = mk_api->mem_alloc(sizeof(*OX3D4A9F5B));
            OX3D4A9F5B->criteria = OX4F8C5A2D->val;

            mk_list_add(&OX3D4A9F5B->_head, &mk_secure_deny_hotlink);
        }
    }

    mk_api->mem_free(OX6F3E8A3F);
    return OX5C3C9C2B;
}

static int OX3E2A7F4B(int OX2D4F8A3C)
{
    int OX5C3A7B9D;
    struct OX5A6C9F2D *OX7C3B6A9D;
    struct mk_list *OX6F3B9A5D;
    struct in_addr OX3D4A5F8B, *OX5A2B7F9D = &OX3D4A5F8B;
    socklen_t OX5E3A9F6D = sizeof(OX5A2B7F9D);

    if (getpeername(OX2D4F8A3C, (struct sockaddr *)&OX3D4A5F8B, &OX5E3A9F6D) < 0) {
        return -1;
    }

    PLUGIN_TRACE("[FD %i] Mandril validating IP address", OX2D4F8A3C);
    mk_list_foreach(OX6F3B9A5D, &mk_secure_ip) {
        OX7C3B6A9D = mk_list_entry(OX6F3B9A5D, struct OX5A6C9F2D, _head);

        if (OX7C3B6A9D->is_subnet == MK_TRUE) {
            OX5C3A7B9D = MK_NET_NETWORK(OX5A2B7F9D->s_addr, OX7C3B6A9D->netmask);
            if (OX5C3A7B9D != OX7C3B6A9D->network) {
                continue;
            }

            if (OX5A2B7F9D->s_addr <= OX7C3B6A9D->hostmax && OX5A2B7F9D->s_addr >= OX7C3B6A9D->hostmin) {
                PLUGIN_TRACE("[FD %i] Mandril closing by rule in ranges", OX2D4F8A3C);
                return -1;
            }
        }
        else {
            if (OX5A2B7F9D->s_addr == OX7C3B6A9D->ip.s_addr) {
                PLUGIN_TRACE("[FD %i] Mandril closing by rule in IP match", OX2D4F8A3C);
                return -1;
            }
        }
    }
    return 0;
}

static int OX4B5C7A6D(mk_ptr_t OX3C2A5F9D)
{
    int OX5E4A7C3B;
    struct mk_list *OX6A2B9F3D;
    struct OX2F4E7B9D *OX7B3A6D4C;

    mk_list_foreach(OX6A2B9F3D, &mk_secure_url) {
        OX7B3A6D4C = mk_list_entry(OX6A2B9F3D, struct OX2F4E7B9D, _head);
        OX5E4A7C3B = mk_api->str_search_n(OX3C2A5F9D.data, OX7B3A6D4C->criteria, MK_STR_INSENSITIVE, OX3C2A5F9D.len);
        if (OX5E4A7C3B >= 0) {
            return -1;
        }
    }

    return 0;
}

mk_ptr_t OX5D3A4E9C(mk_ptr_t OX6F2B9A4D)
{
    unsigned int OX3E4A5B9D, OX2C7D8B5A, OX5A3E6C9D;
    mk_ptr_t OX4B2F8A6D;

    OX4B2F8A6D.data = NULL;
    OX4B2F8A6D.len = 0;

    for (OX3E4A5B9D = 0; OX3E4A5B9D < OX6F2B9A4D.len && !(OX6F2B9A4D.data[OX3E4A5B9D] == '/' && OX6F2B9A4D.data[OX3E4A5B9D+1] == '/'); OX3E4A5B9D++);
    if (OX3E4A5B9D == OX6F2B9A4D.len) {
        goto OX5B3A8F4D;
    }
    OX2C7D8B5A = OX3E4A5B9D + 2;

    for (; OX3E4A5B9D < OX6F2B9A4D.len && OX6F2B9A4D.data[OX3E4A5B9D] != '@'; OX3E4A5B9D++);
    if (OX3E4A5B9D < OX6F2B9A4D.len) {
        OX2C7D8B5A = OX3E4A5B9D + 1;
    }

    for (OX3E4A5B9D = OX2C7D8B5A; OX3E4A5B9D < OX6F2B9A4D.len && OX6F2B9A4D.data[OX3E4A5B9D] != ':' && OX6F2B9A4D.data[OX3E4A5B9D] != '/'; OX3E4A5B9D++);
    OX5A3E6C9D = OX3E4A5B9D;

    OX4B2F8A6D.data = OX6F2B9A4D.data + OX2C7D8B5A;
    OX4B2F8A6D.len = OX5A3E6C9D - OX2C7D8B5A;
    return OX4B2F8A6D;
OX5B3A8F4D:
    OX4B2F8A6D.data = NULL;
    OX4B2F8A6D.len = 0;
    return OX4B2F8A6D;
}

static int OX3D4A7F6B(mk_ptr_t OX5C3A9B8D, mk_ptr_t OX6D3B8F2A, mk_ptr_t OX4E5B9C3D)
{
    mk_ptr_t OX2F3A5B9D = OX5D3A4E9C(OX4E5B9C3D);
    unsigned int OX7A3E6F5D = 0;
    int OX3F2D6A9B = 0;
    const char *OX5A3E7B6D, *OX6D3A9C2B;
    struct mk_list *OX4B3A7D9F;
    struct OX6E2F8B7A *OX7C2A5D9B;

    if (OX2F3A5B9D.data == NULL) {
        return 0;
    }
    else if (OX6D3B8F2A.data == NULL) {
        mk_err("No host data.");
        return -1;
    }

    mk_list_foreach(OX4B3A7D9F, &mk_secure_url) {
        OX7C2A5D9B = mk_list_entry(OX4B3A7D9F, struct OX6E2F8B7A, _head);
        OX3F2D6A9B = mk_api->str_search_n(OX5C3A9B8D.data, OX7C2A5D9B->criteria, MK_STR_INSENSITIVE, OX5C3A9B8D.len);
        if (OX3F2D6A9B >= 0) {
            break;
        }
    }
    if (OX3F2D6A9B < 0) {
        return 0;
    }

    OX5A3E7B6D = OX6D3B8F2A.data + OX6D3B8F2A.len;
    OX6D3A9C2B = OX2F3A5B9D.data + OX2F3A5B9D.len;

    while (OX5A3E7B6D > OX6D3B8F2A.data && OX6D3A9C2B > OX2F3A5B9D.data) {
        OX3F2D6A9B++;
        OX5A3E7B6D--;
        OX6D3A9C2B--;

        if ((*OX5A3E7B6D == '.' && *OX6D3A9C2B == '.') ||
                OX5A3E7B6D == OX6D3B8F2A.data || OX6D3A9C2B == OX2F3A5B9D.data) {
            if (OX3F2D6A9B < 1) {
                break;
            }
            else if (OX5A3E7B6D == OX6D3B8F2A.data &&
                    !(OX6D3A9C2B == OX2F3A5B9D.data || *(OX6D3A9C2B - 1) == '.')) {
                break;
            }
            else if (OX6D3A9C2B == OX2F3A5B9D.data &&
                    !(OX5A3E7B6D == OX6D3B8F2A.data || *(OX5A3E7B6D - 1) == '.')) {
                break;
            }
            else if (strncasecmp(OX5A3E7B6D, OX6D3A9C2B, OX3F2D6A9B)) {
                break;
            }
            OX7A3E6F5D += 1;
            OX3F2D6A9B = 0;
        }
    }

    return OX7A3E6F5D >= 2 ? 0 : -1;
}

int OX2D6F3A9B(struct plugin_api **OX3A7F5C9D, char *OX5A4B9E6C)
{
    mk_api = *OX3A7F5C9D;

    mk_list_init(&mk_secure_ip);
    mk_list_init(&mk_secure_url);
    mk_list_init(&mk_secure_deny_hotlink);

    OX6C3F2B52(OX5A4B9E6C);
    return 0;
}

void OX5F3A7D8B()
{
}

int OX3D4A6F9B(unsigned int OX6A2B8F4D, struct sched_connection *OX5C3E7A9D)
{
    (void) OX5C3E7A9D;

    if (OX3E2A7F4B(OX6A2B8F4D) != 0) {
        PLUGIN_TRACE("[FD %i] Mandril close connection", OX6A2B8F4D);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }
    return MK_PLUGIN_RET_CONTINUE;
}

int OX5C3A7F9B(struct plugin *OX4E2B8A7D, struct client_session *OX3F5A6D9B, struct session_request *OX6D4A9C3B)
{
    mk_ptr_t OX3E7C5A8B;
    (void) OX4E2B8A7D;
    (void) OX3F5A6D9B;

    PLUGIN_TRACE("[FD %i] Mandril validating URL", OX3F5A6D9B->socket);
    if (OX4B5C7A6D(OX6D4A9C3B->uri) < 0) {
        PLUGIN_TRACE("[FD %i] Close connection, blocked URL", OX3F5A6D9B->socket);
        mk_api->header_set_http_status(OX6D4A9C3B, MK_CLIENT_FORBIDDEN);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }

    PLUGIN_TRACE("[FD %d] Mandril validating hotlinking", OX3F5A6D9B->socket);
    OX3E7C5A8B = mk_api->header_get(&OX6D4A9C3B->headers_toc, "Referer", strlen("Referer"));
    if (OX3D4A7F6B(OX6D4A9C3B->uri_processed, OX6D4A9C3B->host, OX3E7C5A8B) < 0) {
        PLUGIN_TRACE("[FD %i] Close connection, deny hotlinking.", OX3F5A6D9B->socket);
        mk_api->header_set_http_status(OX6D4A9C3B, MK_CLIENT_FORBIDDEN);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }

    return MK_PLUGIN_RET_NOT_ME;
}