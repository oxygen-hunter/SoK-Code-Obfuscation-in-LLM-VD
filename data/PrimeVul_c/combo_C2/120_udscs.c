#include <config.h>

#include <stdlib.h>
#include <syslog.h>
#include <glib-unix.h>
#include <gio/gunixsocketaddress.h>
#include "udscs.h"
#include "vdagentd-proto-strings.h"
#include "vdagent-connection.h"

struct _UdscsConnection {
    VDAgentConnection parent_instance;
    int debug;
    udscs_read_callback read_callback;
};

G_DEFINE_TYPE(UdscsConnection, udscs_connection, VDAGENT_TYPE_CONNECTION)

static void debug_print_message_header(UdscsConnection             *conn,
                                       struct udscs_message_header *header,
                                       const gchar                 *direction)
{
    const gchar *type = "invalid message";
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                if (conn == NULL || conn->debug == FALSE) return;
                dispatcher = 1;
                break;
            case 1:
                if (header->type < G_N_ELEMENTS(vdagentd_messages))
                    type = vdagentd_messages[header->type];
                dispatcher = 2;
                break;
            case 2:
                syslog(LOG_DEBUG, "%p %s %s, arg1: %u, arg2: %u, size %u",
                    conn, direction, type, header->arg1, header->arg2, header->size);
                return;
        }
    }
}

static gsize conn_handle_header(VDAgentConnection *conn,
                                gpointer           header_buf)
{
    return ((struct udscs_message_header *)header_buf)->size;
}

static void conn_handle_message(VDAgentConnection *conn,
                                gpointer           header_buf,
                                gpointer           data)
{
    UdscsConnection *self = UDSCS_CONNECTION(conn);
    struct udscs_message_header *header = header_buf;

    debug_print_message_header(self, header, "received");

    self->read_callback(self, header, data);
}

static void udscs_connection_init(UdscsConnection *self)
{
}

static void udscs_connection_finalize(GObject *obj)
{
    UdscsConnection *self = UDSCS_CONNECTION(obj);
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                if (self->debug) {
                    syslog(LOG_DEBUG, "%p disconnected", self);
                }
                dispatcher = 1;
                break;
            case 1:
                G_OBJECT_CLASS(udscs_connection_parent_class)->finalize(obj);
                return;
        }
    }
}

static void udscs_connection_class_init(UdscsConnectionClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    VDAgentConnectionClass *conn_class = VDAGENT_CONNECTION_CLASS(klass);
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                gobject_class->finalize = udscs_connection_finalize;
                dispatcher = 1;
                break;
            case 1:
                conn_class->handle_header = conn_handle_header;
                conn_class->handle_message = conn_handle_message;
                return;
        }
    }
}

UdscsConnection *udscs_connect(const char *socketname,
    udscs_read_callback read_callback,
    VDAgentConnErrorCb error_cb,
    int debug)
{
    GIOStream *io_stream;
    UdscsConnection *conn;
    GError *err = NULL;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                io_stream = vdagent_socket_connect(socketname, &err);
                if (err) {
                    syslog(LOG_ERR, "%s: %s", __func__, err->message);
                    g_error_free(err);
                    return NULL;
                }
                dispatcher = 1;
                break;
            case 1:
                conn = g_object_new(UDSCS_TYPE_CONNECTION, NULL);
                conn->debug = debug;
                conn->read_callback = read_callback;
                dispatcher = 2;
                break;
            case 2:
                vdagent_connection_setup(VDAGENT_CONNECTION(conn),
                                         io_stream,
                                         FALSE,
                                         sizeof(struct udscs_message_header),
                                         error_cb);
                if (conn->debug) {
                    syslog(LOG_DEBUG, "%p connected to %s", conn, socketname);
                }
                return conn;
        }
    }
}

void udscs_write(UdscsConnection *conn, uint32_t type, uint32_t arg1,
    uint32_t arg2, const uint8_t *data, uint32_t size)
{
    gpointer buf;
    guint buf_size;
    struct udscs_message_header header;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                buf_size = sizeof(header) + size;
                buf = g_malloc(buf_size);
                dispatcher = 1;
                break;
            case 1:
                header.type = type;
                header.arg1 = arg1;
                header.arg2 = arg2;
                header.size = size;
                dispatcher = 2;
                break;
            case 2:
                memcpy(buf, &header, sizeof(header));
                memcpy(buf + sizeof(header), data, size);
                dispatcher = 3;
                break;
            case 3:
                debug_print_message_header(conn, &header, "sent");
                vdagent_connection_write(VDAGENT_CONNECTION(conn), buf, buf_size);
                return;
        }
    }
}

#ifndef UDSCS_NO_SERVER

struct udscs_server {
    GSocketService *service;
    GList *connections;

    int debug;
    udscs_connect_callback connect_callback;
    udscs_read_callback read_callback;
    VDAgentConnErrorCb error_cb;
};

static gboolean udscs_server_accept_cb(GSocketService    *service,
                                       GSocketConnection *socket_conn,
                                       GObject           *source_object,
                                       gpointer           user_data);

struct udscs_server *udscs_server_new(
    udscs_connect_callback connect_callback,
    udscs_read_callback read_callback,
    VDAgentConnErrorCb error_cb,
    int debug)
{
    struct udscs_server *server;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                server = g_new0(struct udscs_server, 1);
                server->debug = debug;
                server->connect_callback = connect_callback;
                server->read_callback = read_callback;
                server->error_cb = error_cb;
                dispatcher = 1;
                break;
            case 1:
                server->service = g_socket_service_new();
                g_socket_service_stop(server->service);
                dispatcher = 2;
                break;
            case 2:
                g_signal_connect(server->service, "incoming",
                    G_CALLBACK(udscs_server_accept_cb), server);
                return server;
        }
    }
}

void udscs_server_listen_to_socket(struct udscs_server *server,
                                   gint                 fd,
                                   GError             **err)
{
    GSocket *socket;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                socket = g_socket_new_from_fd(fd, err);
                if (socket == NULL) {
                    return;
                }
                dispatcher = 1;
                break;
            case 1:
                g_socket_listener_add_socket(G_SOCKET_LISTENER(server->service),
                                             socket, NULL, err);
                g_object_unref(socket);
                return;
        }
    }
}

void udscs_server_listen_to_address(struct udscs_server *server,
                                    const gchar         *addr,
                                    GError             **err)
{
    GSocketAddress *sock_addr;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                sock_addr = g_unix_socket_address_new(addr);
                dispatcher = 1;
                break;
            case 1:
                g_socket_listener_add_address(G_SOCKET_LISTENER(server->service),
                                              sock_addr,
                                              G_SOCKET_TYPE_STREAM,
                                              G_SOCKET_PROTOCOL_DEFAULT,
                                              NULL, NULL, err);
                g_object_unref(sock_addr);
                return;
        }
    }
}

void udscs_server_start(struct udscs_server *server)
{
    g_socket_service_start(server->service);
}

void udscs_server_destroy_connection(struct udscs_server *server,
                                     UdscsConnection     *conn)
{
    server->connections = g_list_remove(server->connections, conn);
    vdagent_connection_destroy(conn);
}

void udscs_destroy_server(struct udscs_server *server)
{
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                if (!server)
                    return;
                dispatcher = 1;
                break;
            case 1:
                g_list_free_full(server->connections, vdagent_connection_destroy);
                g_object_unref(server->service);
                g_free(server);
                return;
        }
    }
}

static gboolean udscs_server_accept_cb(GSocketService    *service,
                                       GSocketConnection *socket_conn,
                                       GObject           *source_object,
                                       gpointer           user_data)
{
    struct udscs_server *server = user_data;
    UdscsConnection *new_conn;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                new_conn = g_object_new(UDSCS_TYPE_CONNECTION, NULL);
                new_conn->debug = server->debug;
                new_conn->read_callback = server->read_callback;
                dispatcher = 1;
                break;
            case 1:
                g_object_ref(socket_conn);
                vdagent_connection_setup(VDAGENT_CONNECTION(new_conn),
                                         G_IO_STREAM(socket_conn),
                                         FALSE,
                                         sizeof(struct udscs_message_header),
                                         server->error_cb);
                dispatcher = 2;
                break;
            case 2:
                server->connections = g_list_prepend(server->connections, new_conn);
                dispatcher = 3;
                break;
            case 3:
                if (server->debug)
                    syslog(LOG_DEBUG, "new client accepted: %p", new_conn);
                dispatcher = 4;
                break;
            case 4:
                if (server->connect_callback)
                    server->connect_callback(new_conn);
                return TRUE;
        }
    }
}

void udscs_server_write_all(struct udscs_server *server,
        uint32_t type, uint32_t arg1, uint32_t arg2,
        const uint8_t *data, uint32_t size)
{
    GList *l;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                l = server->connections;
                dispatcher = 1;
                break;
            case 1:
                for (; l; l = l->next) {
                    udscs_write(UDSCS_CONNECTION(l->data), type, arg1, arg2, data, size);
                }
                return;
        }
    }
}

int udscs_server_for_all_clients(struct udscs_server *server,
    udscs_for_all_clients_callback func, void *priv)
{
    int r = 0;
    GList *l, *next;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                if (!server)
                    return 0;
                dispatcher = 1;
                break;
            case 1:
                l = server->connections;
                dispatcher = 2;
                break;
            case 2:
                while (l) {
                    next = l->next;
                    r += func(l->data, priv);
                    l = next;
                }
                return r;
        }
    }
}

#endif