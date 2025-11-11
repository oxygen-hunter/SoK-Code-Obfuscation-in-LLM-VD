#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <vector>
#include <asyncio.h>
#include <mitmproxy.h>
#include <human.h>
#include <flow.h>
using namespace std;

class ProxyConnectionHandler : public server::StreamConnectionHandler {
public:
    master::Master* master;
    ProxyConnectionHandler(master::Master* master, stream &r, stream &w, options::Options* options)
        : master(master), server::StreamConnectionHandler(r, w, options) {
        log_prefix = human::format_address(client.peername) + ": ";
    }

    async void handle_hook(commands::StartHook* hook) {
        timeout_watchdog.disarm();
        auto data = hook->args()[0];
        master->addons.handle_lifecycle(hook);
        if (dynamic_cast<flow::Flow*>(data)) {
            data->wait_for_resume();
        }
    }

    void log(const string &message, const string &level = "info") {
        log::LogEntry x(log_prefix + message, level);
        asyncio_utils::create_task(
            master->addons.handle_lifecycle(log::AddLogHook(x)),
            "ProxyConnectionHandler.log"
        );
    }

private:
    string log_prefix;
};

class Proxyserver {
public:
    optional<asyncio::AbstractServer> server;
    int listen_port;
    master::Master* master;
    options::Options* options;
    bool is_running;
    map<tuple<string, int>, ProxyConnectionHandler*> _connections;

    Proxyserver() : is_running(false) {}

    string repr() {
        return "ProxyServer(" + (server.has_value() ? "running" : "stopped") + ", " +
               to_string(_connections.size()) + " active conns)";
    }

    void load(loader::Loader* loader) {
        loader->add_option("connection_strategy", "eager");
        loader->add_option("stream_large_bodies", optional<string>{});
        loader->add_option("body_size_limit", optional<string>{});
        loader->add_option("keep_host_header", false);
        loader->add_option("proxy_debug", false);
        loader->add_option("normalize_outbound_headers", true);
        loader->add_option("validate_inbound_headers", true);
    }

    async void running() {
        master = ctx::master;
        options = ctx::options;
        is_running = true;
        refresh_server();
    }

    void configure(vector<string> updated) {
        if (find(updated.begin(), updated.end(), "stream_large_bodies") != updated.end()) {
            try {
                human::parse_size(ctx::options.stream_large_bodies);
            } catch (const invalid_argument&) {
                throw exceptions::OptionsError("Invalid stream_large_bodies specification: " +
                                               ctx::options.stream_large_bodies);
            }
        }
        if (find(updated.begin(), updated.end(), "body_size_limit") != updated.end()) {
            try {
                human::parse_size(ctx::options.body_size_limit);
            } catch (const invalid_argument&) {
                throw exceptions::OptionsError("Invalid body_size_limit specification: " +
                                               ctx::options.body_size_limit);
            }
        }
        if (is_running && (find(updated.begin(), updated.end(), "server") != updated.end() ||
                           find(updated.begin(), updated.end(), "listen_host") != updated.end() ||
                           find(updated.begin(), updated.end(), "listen_port") != updated.end())) {
            asyncio::create_task(refresh_server());
        }
    }

    async void refresh_server() {
        _lock.lock();
        if (server.has_value()) {
            shutdown_server();
            server.reset();
        }
        if (ctx::options.server) {
            if (!ctx::master->addons.get("nextlayer")) {
                ctx::log.warn("Warning: Running proxyserver without nextlayer addon!");
            }
            try {
                server = asyncio::start_server(
                    bind(&Proxyserver::handle_connection, this),
                    options->listen_host,
                    options->listen_port
                );
            } catch (const system_error& e) {
                ctx::log.error(e.what());
                return;
            }
            auto addrs = { "http://" + human::format_address(server->sockets[0].getsockname()) };
            ctx::log.info("Proxy server listening at " + addrs);
        }
        _lock.unlock();
    }

    async void shutdown_server() {
        ctx::log.info("Stopping server...");
        server->close();
        server->wait_closed();
        server.reset();
    }

    async void handle_connection(stream &r, stream &w) {
        auto peername = w.get_extra_info("peername");
        asyncio_utils::set_task_debug_info(
            asyncio::current_task(),
            "Proxyserver.handle_connection",
            peername
        );
        ProxyConnectionHandler* handler = new ProxyConnectionHandler(master, r, w, options);
        _connections[peername] = handler;
        try {
            handler->handle_client();
        } finally {
            _connections.erase(peername);
        }
    }

    void inject_event(events::MessageInjected* event) {
        if (_connections.find(event->flow.client_conn.peername) == _connections.end()) {
            throw invalid_argument("Flow is not from a live connection.");
        }
        _connections[event->flow.client_conn.peername]->server_event(event);
    }

    command::command("inject.websocket")
    void inject_websocket(flow::Flow* flow, bool to_client, const bytes& message, bool is_text = true) {
        if (!dynamic_cast<http::HTTPFlow*>(flow) || !flow->websocket) {
            ctx::log.warn("Cannot inject WebSocket messages into non-WebSocket flows.");
        }
        websocket::WebSocketMessage msg(
            is_text ? Opcode::TEXT : Opcode::BINARY,
            !to_client,
            message
        );
        WebSocketMessageInjected event(flow, msg);
        try {
            inject_event(&event);
        } catch (const invalid_argument& e) {
            ctx::log.warn(e.what());
        }
    }

    command::command("inject.tcp")
    void inject_tcp(flow::Flow* flow, bool to_client, const bytes& message) {
        if (!dynamic_cast<tcp::TCPFlow*>(flow)) {
            ctx::log.warn("Cannot inject TCP messages into non-TCP flows.");
        }
        TcpMessageInjected event(flow, tcp::TCPMessage(!to_client, message));
        try {
            inject_event(&event);
        } catch (const invalid_argument& e) {
            ctx::log.warn(e.what());
        }
    }

    void server_connect(server_hooks::ServerConnectionHookData* ctx) {
        assert(ctx->server.address);
        bool self_connect = (ctx->server.address[1] == options->listen_port &&
                             (ctx->server.address[0] == "localhost" ||
                              ctx->server.address[0] == "127.0.0.1" ||
                              ctx->server.address[0] == "::1" ||
                              ctx->server.address[0] == options->listen_host));
        if (self_connect) {
            ctx->server.error = "Request destination unknown. Unable to figure out where this request should be forwarded to.";
        }
    }

private:
    asyncio::Lock _lock;
};