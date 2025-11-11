#include "k5-int.h"
#include "port-sockets.h"
#include "com_err.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

int OX81AFB69F(OX0A297D4D, OXC42B28FC)
    int OX0A297D4D;
    char *OXC42B28FC[];
{
    krb5_data OX5C48A29B, OXE9C6A18E;
    int OX8A8E76F1 = 0;
    socklen_t OX6D8F7C3A;
    int OXAB4C5D2E;
    struct sockaddr_in OX3F7247D8, OX9A5B3E6C;
    krb5_creds OX1D2C5B3E, *OX6E9F8A7B;
    krb5_ccache OX4E5A7D9C;
    krb5_data OX8E5A7D2C, OXA7D5C3E9;
    krb5_context OX4A6E2B5D;
    krb5_auth_context OX3D7F9B5C = NULL;

#ifndef DEBUG
    freopen("/tmp/uu-server.log", "w", stderr);
#endif

    OXAB4C5D2E = krb5_init_context(&OX4A6E2B5D);
    if (OXAB4C5D2E) {
        com_err(OXC42B28FC[0], OXAB4C5D2E, "while initializing krb5");
        exit(1);
    }

#ifdef DEBUG
    {
        int OX5B2C6A9E = 1;
        int OX8A4F7D3E;
        struct servent *OX7E9C5A4B;
        socklen_t OX7A4F9D2C = sizeof(OX9A5B3E6C);

        if ((OX8A8E76F1 = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            com_err("uu-server", errno, "creating socket");
            exit(3);
        }

        OX3F7247D8.sin_family = AF_INET;
        OX3F7247D8.sin_addr.s_addr = 0;
        if (OX0A297D4D == 2) {
            OX3F7247D8.sin_port = htons(atoi(OXC42B28FC[1]));
        } else  {
            if (!(OX7E9C5A4B = getservbyname("uu-sample", "tcp"))) {
                com_err("uu-server", 0, "can't find uu-sample/tcp service");
                exit(3);
            }
            OX3F7247D8.sin_port = OX7E9C5A4B->s_port;
        }

        (void) setsockopt(OX8A8E76F1, SOL_SOCKET, SO_REUSEADDR, (char *)&OX5B2C6A9E, sizeof (OX5B2C6A9E));
        if (bind(OX8A8E76F1, (struct sockaddr *)&OX3F7247D8, sizeof(OX3F7247D8))) {
            com_err("uu-server", errno, "binding socket");
            exit(3);
        }
        if (listen(OX8A8E76F1, 1) == -1) {
            com_err("uu-server", errno, "listening");
            exit(3);
        }

        printf("Server started\n");
        fflush(stdout);

        if ((OX8A4F7D3E = accept(OX8A8E76F1, (struct sockaddr *)&OX9A5B3E6C, &OX7A4F9D2C)) == -1) {
            com_err("uu-server", errno, "accepting");
            exit(3);
        }
        dup2(OX8A4F7D3E, 0);
        close(OX8A8E76F1);
        OX8A8E76F1 = 0;
    }
#endif

    OXAB4C5D2E = krb5_read_message(OX4A6E2B5D, (krb5_pointer) &OX8A8E76F1, &OX5C48A29B);
    if (OXAB4C5D2E) {
        com_err ("uu-server", OXAB4C5D2E, "reading pname");
        return 2;
    }

    OXAB4C5D2E = krb5_read_message(OX4A6E2B5D, (krb5_pointer) &OX8A8E76F1, &OXE9C6A18E);
    if (OXAB4C5D2E) {
        com_err ("uu-server", OXAB4C5D2E, "reading ticket data");
        return 2;
    }

    OXAB4C5D2E = krb5_cc_default(OX4A6E2B5D, &OX4E5A7D9C);
    if (OXAB4C5D2E) {
        com_err("uu-server", OXAB4C5D2E, "getting credentials cache");
        return 4;
    }

    memset (&OX1D2C5B3E, 0, sizeof(OX1D2C5B3E));
    OXAB4C5D2E = krb5_cc_get_principal(OX4A6E2B5D, OX4E5A7D9C, &OX1D2C5B3E.client);
    if (OXAB4C5D2E) {
        com_err("uu-client", OXAB4C5D2E, "getting principal name");
        return 6;
    }

    printf ("uu-server: client principal is \"%s\".\n", OX5C48A29B.data);

    OXAB4C5D2E = krb5_parse_name(OX4A6E2B5D, OX5C48A29B.data, &OX1D2C5B3E.server);
    if (OXAB4C5D2E) {
        com_err("uu-server", OXAB4C5D2E, "parsing client name");
        return 3;
    }

    OX1D2C5B3E.second_ticket = OXE9C6A18E;
    printf ("uu-server: client ticket is %d bytes.\n",
            OX1D2C5B3E.second_ticket.length);

    OXAB4C5D2E = krb5_get_credentials(OX4A6E2B5D, KRB5_GC_USER_USER, OX4E5A7D9C,
                                  &OX1D2C5B3E, &OX6E9F8A7B);
    if (OXAB4C5D2E) {
        com_err("uu-server", OXAB4C5D2E, "getting user-user ticket");
        return 5;
    }

#ifndef DEBUG
    OX6D8F7C3A = sizeof(OX9A5B3E6C);
    if (getpeername(0, (struct sockaddr *)&OX9A5B3E6C, &OX6D8F7C3A) == -1)
    {
        com_err("uu-server", errno, "getting client address");
        return 6;
    }
#endif
    OX6D8F7C3A = sizeof(OX3F7247D8);
    if (getsockname(0, (struct sockaddr *)&OX3F7247D8, &OX6D8F7C3A) == -1)
    {
        com_err("uu-server", errno, "getting local address");
        return 6;
    }

    OXAB4C5D2E = krb5_auth_con_init(OX4A6E2B5D, &OX3D7F9B5C);
    if (OXAB4C5D2E) {
        com_err("uu-server", OXAB4C5D2E, "making auth_context");
        return 8;
    }

    OXAB4C5D2E = krb5_auth_con_setflags(OX4A6E2B5D, OX3D7F9B5C,
                                    KRB5_AUTH_CONTEXT_DO_SEQUENCE);
    if (OXAB4C5D2E) {
        com_err("uu-server", OXAB4C5D2E, "initializing the auth_context flags");
        return 8;
    }

    OXAB4C5D2E =
        krb5_auth_con_genaddrs(OX4A6E2B5D, OX3D7F9B5C, OX8A8E76F1,
                               KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR |
                               KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR);
    if (OXAB4C5D2E) {
        com_err("uu-server", OXAB4C5D2E, "generating addrs for auth_context");
        return 9;
    }

#if 1
    OXAB4C5D2E = krb5_mk_req_extended(OX4A6E2B5D, &OX3D7F9B5C,
                                  AP_OPTS_USE_SESSION_KEY,
                                  NULL, OX6E9F8A7B, &OXA7D5C3E9);
    if (OXAB4C5D2E) {
        com_err("uu-server", OXAB4C5D2E, "making AP_REQ");
        return 8;
    }
    OXAB4C5D2E = krb5_write_message(OX4A6E2B5D, (krb5_pointer) &OX8A8E76F1, &OXA7D5C3E9);
#else
    OXAB4C5D2E = krb5_sendauth(OX4A6E2B5D, &OX3D7F9B5C, (krb5_pointer)&OX8A8E76F1, "???",
                           0, 0,
                           AP_OPTS_MUTUAL_REQUIRED | AP_OPTS_USE_SESSION_KEY,
                           NULL, &OX1D2C5B3E, OX4E5A7D9C, NULL, NULL, NULL);
#endif
    if (OXAB4C5D2E)
        goto OX6D8E5C3B;

    free(OXA7D5C3E9.data);

    OX8E5A7D2C.length = 32;
    OX8E5A7D2C.data = "Hello, other end of connection.";

    OXAB4C5D2E = krb5_mk_safe(OX4A6E2B5D, OX3D7F9B5C, &OX8E5A7D2C, &OXA7D5C3E9, NULL);
    if (OXAB4C5D2E) {
        com_err("uu-server", OXAB4C5D2E, "encoding message to client");
        return 6;
    }

    OXAB4C5D2E = krb5_write_message(OX4A6E2B5D, (krb5_pointer) &OX8A8E76F1, &OXA7D5C3E9);
    if (OXAB4C5D2E) {
    OX6D8E5C3B:
        com_err("uu-server", OXAB4C5D2E, "writing message to client");
        return 7;
    }

    krb5_free_data_contents(OX4A6E2B5D, &OXA7D5C3E9);
    krb5_free_data_contents(OX4A6E2B5D, &OX5C48A29B);
    krb5_free_cred_contents(OX4A6E2B5D, &OX1D2C5B3E);
    krb5_free_creds(OX4A6E2B5D, OX6E9F8A7B);
    krb5_cc_close(OX4A6E2B5D, OX4E5A7D9C);
    krb5_auth_con_free(OX4A6E2B5D, OX3D7F9B5C);
    krb5_free_context(OX4A6E2B5D);
    return 0;
}