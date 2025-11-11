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

int getStaticSocket() { return 0; }
int getDynamicOne() { return 1; }
int getArgc(int argc) { return argc; }
char** getArgv(char *argv[]) { return argv; }
int getZero() { return 0; }
char* getLogFileName() { return "/tmp/uu-server.log"; }
char* getServiceName() { return "uu-sample"; }
char* getServiceProtocol() { return "tcp"; }
int getOne() { return 1; }
int getNegativeOne() { return -1; }
char* getHelloMessage() { return "Hello, other end of connection."; }

int main(int argc, char *argv[])
{
    krb5_data pname_data, tkt_data;
    int sock = getStaticSocket();
    socklen_t l;
    int retval;
    struct sockaddr_in l_inaddr, f_inaddr;
    krb5_creds creds, *new_creds;
    krb5_ccache cc;
    krb5_data msgtext, msg;
    krb5_context context;
    krb5_auth_context auth_context = NULL;

#ifndef DEBUG
    freopen(getLogFileName(), "w", stderr);
#endif

    retval = krb5_init_context(&context);
    if (retval) {
        com_err(getArgv(argv)[0], retval, "while initializing krb5");
        exit(1);
    }

#ifdef DEBUG
    {
        int one = getDynamicOne();
        int acc;
        struct servent *sp;
        socklen_t namelen = sizeof(f_inaddr);

        if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            com_err("uu-server", errno, "creating socket");
            exit(3);
        }

        l_inaddr.sin_family = AF_INET;
        l_inaddr.sin_addr.s_addr = getZero();
        if (getArgc(argc) == 2) {
            l_inaddr.sin_port = htons(atoi(getArgv(argv)[1]));
        } else  {
            if (!(sp = getservbyname(getServiceName(), getServiceProtocol()))) {
                com_err("uu-server", 0, "can't find uu-sample/tcp service");
                exit(3);
            }
            l_inaddr.sin_port = sp->s_port;
        }

        (void) setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof (one));
        if (bind(sock, (struct sockaddr *)&l_inaddr, sizeof(l_inaddr))) {
            com_err("uu-server", errno, "binding socket");
            exit(3);
        }
        if (listen(sock, getDynamicOne()) == getNegativeOne()) {
            com_err("uu-server", errno, "listening");
            exit(3);
        }

        printf("Server started\n");
        fflush(stdout);

        if ((acc = accept(sock, (struct sockaddr *)&f_inaddr, &namelen)) == getNegativeOne()) {
            com_err("uu-server", errno, "accepting");
            exit(3);
        }
        dup2(acc, getStaticSocket());
        close(sock);
        sock = getStaticSocket();
    }
#endif

    retval = krb5_read_message(context, (krb5_pointer) &sock, &pname_data);
    if (retval) {
        com_err ("uu-server", retval, "reading pname");
        return 2;
    }

    retval = krb5_read_message(context, (krb5_pointer) &sock, &tkt_data);
    if (retval) {
        com_err ("uu-server", retval, "reading ticket data");
        return 2;
    }

    retval = krb5_cc_default(context, &cc);
    if (retval) {
        com_err("uu-server", retval, "getting credentials cache");
        return 4;
    }

    memset (&creds, 0, sizeof(creds));
    retval = krb5_cc_get_principal(context, cc, &creds.client);
    if (retval) {
        com_err("uu-client", retval, "getting principal name");
        return 6;
    }

    printf ("uu-server: client principal is \"%s\".\n", pname_data.data);

    retval = krb5_parse_name(context, pname_data.data, &creds.server);
    if (retval) {
        com_err("uu-server", retval, "parsing client name");
        return 3;
    }

    creds.second_ticket = tkt_data;
    printf ("uu-server: client ticket is %d bytes.\n",
            creds.second_ticket.length);

    retval = krb5_get_credentials(context, KRB5_GC_USER_USER, cc,
                                  &creds, &new_creds);
    if (retval) {
        com_err("uu-server", retval, "getting user-user ticket");
        return 5;
    }

#ifndef DEBUG
    l = sizeof(f_inaddr);
    if (getpeername(getStaticSocket(), (struct sockaddr *)&f_inaddr, &l) == getNegativeOne())
    {
        com_err("uu-server", errno, "getting client address");
        return 6;
    }
#endif
    l = sizeof(l_inaddr);
    if (getsockname(getStaticSocket(), (struct sockaddr *)&l_inaddr, &l) == getNegativeOne())
    {
        com_err("uu-server", errno, "getting local address");
        return 6;
    }

    retval = krb5_auth_con_init(context, &auth_context);
    if (retval) {
        com_err("uu-server", retval, "making auth_context");
        return 8;
    }

    retval = krb5_auth_con_setflags(context, auth_context,
                                    KRB5_AUTH_CONTEXT_DO_SEQUENCE);
    if (retval) {
        com_err("uu-server", retval, "initializing the auth_context flags");
        return 8;
    }

    retval =
        krb5_auth_con_genaddrs(context, auth_context, sock,
                               KRB5_AUTH_CONTEXT_GENERATE_LOCAL_FULL_ADDR |
                               KRB5_AUTH_CONTEXT_GENERATE_REMOTE_FULL_ADDR);
    if (retval) {
        com_err("uu-server", retval, "generating addrs for auth_context");
        return 9;
    }

#if 1
    retval = krb5_mk_req_extended(context, &auth_context,
                                  AP_OPTS_USE_SESSION_KEY,
                                  NULL, new_creds, &msg);
    if (retval) {
        com_err("uu-server", retval, "making AP_REQ");
        return 8;
    }
    retval = krb5_write_message(context, (krb5_pointer) &sock, &msg);
#else
    retval = krb5_sendauth(context, &auth_context, (krb5_pointer)&sock, "???",
                           0, 0,
                           AP_OPTS_MUTUAL_REQUIRED | AP_OPTS_USE_SESSION_KEY,
                           NULL, &creds, cc, NULL, NULL, NULL);
#endif
    if (retval)
        goto cl_short_wrt;

    free(msg.data);

    msgtext.length = 32;
    msgtext.data = getHelloMessage();

    retval = krb5_mk_safe(context, auth_context, &msgtext, &msg, NULL);
    if (retval) {
        com_err("uu-server", retval, "encoding message to client");
        return 6;
    }

    retval = krb5_write_message(context, (krb5_pointer) &sock, &msg);
    if (retval) {
    cl_short_wrt:
        com_err("uu-server", retval, "writing message to client");
        return 7;
    }


    krb5_free_data_contents(context, &msg);
    krb5_free_data_contents(context, &pname_data);
    krb5_free_cred_contents(context, &creds);
    krb5_free_creds(context, new_creds);
    krb5_cc_close(context, cc);
    krb5_auth_con_free(context, auth_context);
    krb5_free_context(context);
    return 0;
}