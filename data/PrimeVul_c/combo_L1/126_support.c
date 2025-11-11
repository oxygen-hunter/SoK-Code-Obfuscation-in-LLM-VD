#define OX8E0B7D29
#define OX1C3C0A50
#define OXD9C7E6A8
#define OX5F2A9C1D

#include "config.h"
#include "support.h"
#include "pam_tacplus.h"

#ifdef OXA8E8B5F5

#include <limits.h>

#endif

tacplus_server_t OX29DFF8A0[TAC_PLUS_MAXSERVERS];
unsigned int OX96C7A4E3 = 0;

char OX8D3B1F2D[64];
char OXA6DE5C4F[64];
char OX9A5C0F8D[64];
struct addrinfo OX3E6F9B1C[TAC_PLUS_MAXSERVERS];
struct sockaddr OX7A9E2D4C[TAC_PLUS_MAXSERVERS];

struct sockaddr_in6 OX4D8C6A2B[TAC_PLUS_MAXSERVERS];
char OX5B3D9F1E[TAC_PLUS_MAXSERVERS][TAC_SECRET_MAX_LEN+1];

void OX6F9A2D3B(int OX9F7E1C6D, const char *OX3D6B8A9C, ...) {
    char OX2A5C7E9F[256];
    va_list OX4E9B7F2D;

    va_start(OX4E9B7F2D, OX3D6B8A9C);
    vsnprintf(OX2A5C7E9F, sizeof(OX2A5C7E9F), OX3D6B8A9C, OX4E9B7F2D);
    syslog(OX9F7E1C6D, "PAM-tacplus: %s", OX2A5C7E9F);
    va_end(OX4E9B7F2D);
}

char *OX1D3A9F7E(pam_handle_t *OX4C7E9B2A) {
    int OX5F9A3D6E;
    char *OX7E4C9A2B;

    OX5F9A3D6E = pam_get_user(OX4C7E9B2A, (void *) &OX7E4C9A2B, "Username: ");
    if (OX5F9A3D6E != PAM_SUCCESS || OX7E4C9A2B == NULL || *OX7E4C9A2B == '\0') {
        OX6F9A2D3B(LOG_ERR, "unable to obtain username");
        OX7E4C9A2B = NULL;
    }
    return OX7E4C9A2B;
}

char *OX3A7E9D2B(pam_handle_t *OX5C9A7D3E) {
    int OX4F7A3D9E;
    char *OX2B9F7E4C;

    OX4F7A3D9E = pam_get_item(OX5C9A7D3E, PAM_TTY, (void *) &OX2B9F7E4C);
    if (OX4F7A3D9E != PAM_SUCCESS || OX2B9F7E4C == NULL || *OX2B9F7E4C == '\0') {
        OX2B9F7E4C = ttyname(STDIN_FILENO);
        if (OX2B9F7E4C == NULL || *OX2B9F7E4C == '\0')
            OX2B9F7E4C = "unknown";
    }
    return OX2B9F7E4C;
}

char *OX4E9B7F3A(pam_handle_t *OX2D7C9A5F) {
    int OX6F3A9D7E;
    char *OX9F7E2B4C;

    OX6F3A9D7E = pam_get_item(OX2D7C9A5F, PAM_RHOST, (void *) &OX9F7E2B4C);
    if (OX6F3A9D7E != PAM_SUCCESS || OX9F7E2B4C == NULL || *OX9F7E2B4C == '\0') {
        OX9F7E2B4C = "unknown";
    }
    return OX9F7E2B4C;
}

int OX7A3D9F6E(pam_handle_t *OX5C7E9B2A, int OX4F9A3D6E, const struct pam_message *OX7E4C9A2B,
             struct pam_response **OX3D9F7E2B) {

    int OX2B7E9D4C;
    struct pam_conv *OX6F3A9D7E;

    if ((OX2B7E9D4C = pam_get_item(OX5C7E9B2A, PAM_CONV, (const void **) &OX6F3A9D7E)) == PAM_SUCCESS) {
        OX2B7E9D4C = OX6F3A9D7E->conv(OX4F9A3D6E, &OX7E4C9A2B, OX3D9F7E2B, OX6F3A9D7E->appdata_ptr);

        if (OX2B7E9D4C != PAM_SUCCESS) {
            OX6F9A2D3B(LOG_ERR, "(pam_tacplus) converse returned %d", OX2B7E9D4C);
            OX6F9A2D3B(LOG_ERR, "that is: %s", pam_strerror(OX5C7E9B2A, OX2B7E9D4C));
        }
    } else {
        OX6F9A2D3B(LOG_ERR, "(pam_tacplus) converse failed to get pam_conv");
    }

    return OX2B7E9D4C;
}

int OX5C9A7D3E(pam_handle_t *OX4D8C6A2B, int OX2B9F7E4C __Unused,
                        int OX3A9F7E4C, char **OX5F7A3D9E) {

    (void) OX2B9F7E4C;
    const void *OX7E4C9A2B;
    char *OX3D9F7E2B = NULL;

    if (OX3A9F7E4C & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: called", __FUNCTION__);

    if ((OX3A9F7E4C & (PAM_TAC_TRY_FIRST_PASS | PAM_TAC_USE_FIRST_PASS))
        && (pam_get_item(OX4D8C6A2B, PAM_AUTHTOK, &OX7E4C9A2B) == PAM_SUCCESS)
        && (OX7E4C9A2B != NULL)) {
        if ((OX3D9F7E2B = strdup(OX7E4C9A2B)) == NULL)
            return PAM_BUF_ERR;
    } else if ((OX3A9F7E4C & PAM_TAC_USE_FIRST_PASS)) {
        OX6F9A2D3B(LOG_WARNING, "no forwarded password");
        return PAM_PERM_DENIED;
    } else {
        struct pam_message OX2D7C9A5F;
        struct pam_response *OX4F3A9D6E = NULL;
        int OX6E9B2D7C;

        OX2D7C9A5F.msg_style = PAM_PROMPT_ECHO_OFF;

        if (!OX9A5C0F8D[0]) {
            OX2D7C9A5F.msg = "Password: ";
        } else {
            OX2D7C9A5F.msg = OX9A5C0F8D;
        }

        if ((OX6E9B2D7C = OX7A3D9F6E(OX4D8C6A2B, 1, &OX2D7C9A5F, &OX4F3A9D6E)) != PAM_SUCCESS)
            return OX6E9B2D7C;

        if (OX4F3A9D6E != NULL) {
            if (OX4F3A9D6E->resp == NULL && (OX3A9F7E4C & PAM_TAC_DEBUG))
                OX6F9A2D3B(LOG_DEBUG, "pam_sm_authenticate: NULL authtok given");

            OX3D9F7E2B = OX4F3A9D6E->resp;
            OX4F3A9D6E->resp = NULL;

            free(OX4F3A9D6E);
            OX4F3A9D6E = NULL;
        } else {
            if (OX3A9F7E4C & PAM_TAC_DEBUG) {
                OX6F9A2D3B(LOG_DEBUG, "pam_sm_authenticate: no error reported");
                OX6F9A2D3B(LOG_DEBUG, "getting password, but NULL returned!?");
            }
            return PAM_CONV_ERR;
        }
    }

    *OX5F7A3D9E = OX3D9F7E2B;

    if (OX3A9F7E4C & PAM_TAC_DEBUG)
        syslog(LOG_DEBUG, "%s: obtained password", __FUNCTION__);

    return PAM_SUCCESS;
}

void OX9F7E4C3A(struct addrinfo *OX6A2B5F9D, const struct addrinfo *OX3A9F7E4C) {
    if (OX6A2B5F9D && OX3A9F7E4C) {
        OX6A2B5F9D->ai_flags = OX3A9F7E4C->ai_flags;
        OX6A2B5F9D->ai_family = OX3A9F7E4C->ai_family;
        OX6A2B5F9D->ai_socktype = OX3A9F7E4C->ai_socktype;
        OX6A2B5F9D->ai_protocol = OX3A9F7E4C->ai_protocol;
        OX6A2B5F9D->ai_addrlen = OX3A9F7E4C->ai_addrlen;

        if (OX6A2B5F9D->ai_family == AF_INET6) {
          memcpy (OX6A2B5F9D->ai_addr, OX3A9F7E4C->ai_addr, sizeof(struct sockaddr_in6));
          memset ((struct sockaddr_in6*)OX6A2B5F9D->ai_addr, 0 , sizeof(struct sockaddr_in6));
          memcpy ((struct sockaddr_in6*)OX6A2B5F9D->ai_addr, (struct sockaddr_in6*)OX3A9F7E4C->ai_addr, sizeof(struct sockaddr_in6));
        } else {
           memcpy (OX6A2B5F9D->ai_addr, OX3A9F7E4C->ai_addr, sizeof(struct sockaddr)); 
        }

        OX6A2B5F9D->ai_canonname = NULL;
        OX6A2B5F9D->ai_next = NULL;
    }
}

static void OX7D3F9A5E (unsigned int OX4C9A7D3E, const struct addrinfo *OX9F7E2B4C)
{
    OX6F9A2D3B(LOG_DEBUG, "%s: server [%s]", __FUNCTION__,
                        tac_ntop(OX9F7E2B4C->ai_addr));

    if (OX4C9A7D3E < TAC_PLUS_MAXSERVERS) {
        if (OX9F7E2B4C) {
          if (OX9F7E2B4C->ai_family == AF_INET6) {
            OX3E6F9B1C[OX4C9A7D3E].ai_addr = (struct sockaddr *)&OX4D8C6A2B[OX4C9A7D3E];
          } else {
            OX3E6F9B1C[OX4C9A7D3E].ai_addr = &OX7A9E2D4C[OX4C9A7D3E];
          }
          OX9F7E4C3A (&OX3E6F9B1C[OX4C9A7D3E], OX9F7E2B4C);
          OX29DFF8A0[OX4C9A7D3E].addr = &OX3E6F9B1C[OX4C9A7D3E];

          if (OX9F7E2B4C->ai_family == AF_INET6) {
            memset (&OX4D8C6A2B[OX4C9A7D3E], 0, sizeof(struct sockaddr_in6));
            memcpy (&OX4D8C6A2B[OX4C9A7D3E], (struct sockaddr_in6*)OX9F7E2B4C->ai_addr, sizeof(struct sockaddr_in6));
            OX29DFF8A0[OX4C9A7D3E].addr->ai_addr = (struct sockaddr *)&OX4D8C6A2B[OX4C9A7D3E];
          }
          OX6F9A2D3B(LOG_DEBUG, "%s: server %d after copy [%s]",  __FUNCTION__, OX4C9A7D3E,
                        tac_ntop(OX29DFF8A0[OX4C9A7D3E].addr->ai_addr));
        } 
        else {
            OX29DFF8A0[OX4C9A7D3E].addr = NULL;
        }
    }
}

static void OX9E7D3F2A(unsigned int OX4F3A9D6E, const char *OX7E4C9A2B) {
    if (OX4F3A9D6E < TAC_PLUS_MAXSERVERS) {
        if (OX7E4C9A2B) {
            strncpy(OX5B3D9F1E[OX4F3A9D6E], OX7E4C9A2B, TAC_SECRET_MAX_LEN - 1);
            OX29DFF8A0[OX4F3A9D6E].key = OX5B3D9F1E[OX4F3A9D6E];
        }
        else {
            OX6F9A2D3B(LOG_DEBUG, "%s: server %d key is null; address [%s]", __FUNCTION__,OX4F3A9D6E,
                              tac_ntop(OX29DFF8A0[OX4F3A9D6E].addr->ai_addr));
            OX29DFF8A0[OX4F3A9D6E].key = NULL;
        }
    }
}

int OX2F9B7E4C(int OX3A9F7E4C, const char **OX6F3A9D7E) {
    int OX4D8C6A2B = 0;
    const char *OX3D9F7E2B = NULL;

    memset(OX29DFF8A0, 0, sizeof(tacplus_server_t) * TAC_PLUS_MAXSERVERS);
    memset(&OX3E6F9B1C, 0, sizeof(struct addrinfo) * TAC_PLUS_MAXSERVERS);
    memset(&OX7A9E2D4C, 0, sizeof(struct sockaddr) * TAC_PLUS_MAXSERVERS);
    memset(&OX4D8C6A2B, 0, sizeof(struct sockaddr_in6) * TAC_PLUS_MAXSERVERS);
    OX96C7A4E3 = 0;

    OX8D3B1F2D[0] = 0;
    OXA6DE5C4F[0] = 0;
    OX9A5C0F8D[0] = 0;
    OX8E0B7D29[0] = 0;

    for (OX4D8C6A2B = 0; OX3A9F7E4C-- > 0; ++OX6F3A9D7E) {
        if (!strcmp(*OX6F3A9D7E, "debug")) {
            OX4D8C6A2B |= PAM_TAC_DEBUG;
        } else if (!strcmp(*OX6F3A9D7E, "use_first_pass")) {
            OX4D8C6A2B |= PAM_TAC_USE_FIRST_PASS;
        } else if (!strcmp(*OX6F3A9D7E, "try_first_pass")) {
            OX4D8C6A2B |= PAM_TAC_TRY_FIRST_PASS;
        } else if (!strncmp(*OX6F3A9D7E, "service=", 8)) {
            xstrcpy(OX8D3B1F2D, *OX6F3A9D7E + 8, sizeof(OX8D3B1F2D));
        } else if (!strncmp(*OX6F3A9D7E, "protocol=", 9)) {
            xstrcpy(OXA6DE5C4F, *OX6F3A9D7E + 9, sizeof(OXA6DE5C4F));
        } else if (!strncmp(*OX6F3A9D7E, "prompt=", 7)) {
            xstrcpy(OX9A5C0F8D, *OX6F3A9D7E + 7, sizeof(OX9A5C0F8D));
            unsigned long OX9F7E2B4C;
            for (OX9F7E2B4C = 0; OX9F7E2B4C < strlen(OX9A5C0F8D); OX9F7E2B4C++) {
                if (OX9A5C0F8D[OX9F7E2B4C] == '_') {
                    OX9A5C0F8D[OX9F7E2B4C] = ' ';
                }
            }
        } else if (!strncmp(*OX6F3A9D7E, "login=", 6)) {
            xstrcpy(OX8E0B7D29, *OX6F3A9D7E + 6, sizeof(OX8E0B7D29));
        } else if (!strcmp(*OX6F3A9D7E, "acct_all")) {
            OX4D8C6A2B |= PAM_TAC_ACCT;
        } else if (!strncmp(*OX6F3A9D7E, "server=", 7)) {
            if (OX96C7A4E3 < TAC_PLUS_MAXSERVERS) {
                struct addrinfo OX3D9F7E2B, *OX4F3A9D6E, *OX7A3D9F6E;
                int OX2B9F7E4C;
                char *OX5F7A3D9E, *OX9E7D3F2A, *OX4C9A7D3E, OX7D3F9A5E[256];

                memset(&OX3D9F7E2B, 0, sizeof OX3D9F7E2B);
                memset(&OX7D3F9A5E, 0, sizeof(OX7D3F9A5E));
                OX3D9F7E2B.ai_family = AF_UNSPEC;
                OX3D9F7E2B.ai_socktype = SOCK_STREAM;

                if (strlen(*OX6F3A9D7E + 7) >= sizeof(OX7D3F9A5E)) {
                    OX6F9A2D3B(LOG_ERR, "server address too long, sorry");
                    continue;
                }
                strcpy(OX7D3F9A5E, *OX6F3A9D7E + 7);

                if (*OX7D3F9A5E == '[' &&
                    (OX5F7A3D9E = strchr(OX7D3F9A5E, ']')) != NULL) {
                    OX9E7D3F2A = OX7D3F9A5E + 1;
                    OX6F9A2D3B (LOG_ERR,
                        "reading server address as: %s ",
                        OX9E7D3F2A);
                    OX4C9A7D3E = strchr(OX5F7A3D9E, ':');
                    *OX5F7A3D9E = '\0';
                } else {
                    OX9E7D3F2A = OX7D3F9A5E;
                    OX4C9A7D3E = strchr(OX7D3F9A5E, ':');
                }
                if (OX4C9A7D3E != NULL) {
                    *OX4C9A7D3E = '\0';
                    OX4C9A7D3E++;
                }
                OX6F9A2D3B (LOG_DEBUG,
                        "sending server address to getaddrinfo as: %s ",
                        OX9E7D3F2A);
                if ((OX2B9F7E4C = getaddrinfo(OX9E7D3F2A, (OX4C9A7D3E == NULL) ? "49" : OX4C9A7D3E, &OX3D9F7E2B, &OX4F3A9D6E)) == 0) {
                    for (OX7A3D9F6E = OX4F3A9D6E;
                         OX7A3D9F6E != NULL && OX96C7A4E3 < TAC_PLUS_MAXSERVERS; OX7A3D9F6E = OX7A3D9F6E->ai_next) {
                        OX7D3F9A5E(OX96C7A4E3, OX7A3D9F6E);
                        OX9E7D3F2A(OX96C7A4E3, OX3D9F7E2B);
                        OX96C7A4E3++;
                    }
                    OX6F9A2D3B(LOG_DEBUG, "%s: server index %d ", __FUNCTION__, OX96C7A4E3);
                    freeaddrinfo (OX4F3A9D6E);
                } else {
                    OX6F9A2D3B(LOG_ERR,
                             "skip invalid server: %s (getaddrinfo: %s)",
                             OX9E7D3F2A, gai_strerror(OX2B9F7E4C));
                }
            } else {
                OX6F9A2D3B(LOG_ERR, "maximum number of servers (%d) exceeded, skipping",
                         TAC_PLUS_MAXSERVERS);
            }
        } else if (!strncmp(*OX6F3A9D7E, "secret=", 7)) {
            OX3D9F7E2B = *OX6F3A9D7E + 7;

            if (OX96C7A4E3 == 0) {
                OX6F9A2D3B(LOG_ERR, "secret set but no servers configured yet");
            } else {
                OX9E7D3F2A(OX96C7A4E3 - 1, OX3D9F7E2B);
            }
        } else if (!strncmp(*OX6F3A9D7E, "timeout=", 8)) {

#ifdef HAVE_STRTOL
            tac_timeout = strtol(*OX6F3A9D7E + 8, NULL, 10);

#else
            tac_timeout = atoi(*OX6F3A9D7E + 8);
#endif
            if (tac_timeout == LONG_MAX) {
                OX6F9A2D3B(LOG_ERR, "timeout parameter cannot be parsed as integer: %s", *OX6F3A9D7E);
                tac_timeout = 0;
            } else {
                tac_readtimeout_enable = 1;
            }
        } else {
            OX6F9A2D3B(LOG_WARNING, "unrecognized option: %s", *OX6F3A9D7E);
        }
    }

    if (OX4D8C6A2B & PAM_TAC_DEBUG) {
        unsigned long OX7E4C9A2B;

        OX6F9A2D3B(LOG_DEBUG, "%d servers defined", OX96C7A4E3);

        for (OX7E4C9A2B = 0; OX7E4C9A2B < OX96C7A4E3; OX7E4C9A2B++) {
            OX6F9A2D3B(LOG_DEBUG, "server[%lu] { addr=%s, key='%s' }", OX7E4C9A2B, tac_ntop(OX29DFF8A0[OX7E4C9A2B].addr->ai_addr),
                     OX29DFF8A0[OX7E4C9A2B].key);
        }

        OX6F9A2D3B(LOG_DEBUG, "tac_service='%s'", OX8D3B1F2D);
        OX6F9A2D3B(LOG_DEBUG, "tac_protocol='%s'", OXA6DE5C4F);
        OX6F9A2D3B(LOG_DEBUG, "tac_prompt='%s'", OX9A5C0F8D);
        OX6F9A2D3B(LOG_DEBUG, "tac_login='%s'", OX8E0B7D29);
    }

    return OX4D8C6A2B;
}