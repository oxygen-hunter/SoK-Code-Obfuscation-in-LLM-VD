import asyncio
from typing import Dict, Optional, Tuple

from mitmproxy import command, ctx, exceptions, flow, http, log, master, options, platform, tcp, websocket
from mitmproxy.flow import Flow
from mitmproxy.proxy import commands, events, server_hooks
from mitmproxy.proxy import server
from mitmproxy.proxy.layers.tcp import TcpMessageInjected
from mitmproxy.proxy.layers.websocket import WebSocketMessageInjected
from mitmproxy.utils import asyncio_utils, human
from wsproto.frame_protocol import Opcode

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.registers = {}

    def load_instructions(self, instructions):
        self.instructions = instructions

    def execute(self):
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            self.execute_instruction(opcode, *args)

    def execute_instruction(self, opcode, *args):
        if opcode == 'PUSH':
            self.stack.append(args[0])
        elif opcode == 'POP':
            self.stack.pop()
        elif opcode == 'ADD':
            self.stack.append(self.stack.pop() + self.stack.pop())
        elif opcode == 'SUB':
            self.stack.append(self.stack.pop() - self.stack.pop())
        elif opcode == 'LOAD':
            reg = args[0]
            self.stack.append(self.registers.get(reg, 0))
        elif opcode == 'STORE':
            reg = args[0]
            self.registers[reg] = self.stack.pop()
        elif opcode == 'JMP':
            self.pc = args[0]
            return
        elif opcode == 'JZ':
            if self.stack.pop() == 0:
                self.pc = args[0]
                return
        self.pc += 1

class ProxyConnectionHandler(server.StreamConnectionHandler):
    master: master.Master

    def __init__(self, master, r, w, options):
        vm = VM()
        instructions = [
            ('PUSH', master),
            ('STORE', 'master'),
            ('PUSH', r),
            ('STORE', 'r'),
            ('PUSH', w),
            ('STORE', 'w'),
            ('PUSH', options),
            ('STORE', 'options'),
            ('LOAD', 'r'),
            ('LOAD', 'w'),
            ('LOAD', 'options'),
            ('LOAD', 'master'),
        ]
        vm.load_instructions(instructions)
        vm.execute()
        self.master = vm.registers['master']
        super().__init__(vm.registers['r'], vm.registers['w'], vm.registers['options'])
        self.log_prefix = f"{human.format_address(self.client.peername)}: "

    async def handle_hook(self, hook: commands.StartHook) -> None:
        vm = VM()
        vm.load_instructions([
            ('PUSH', hook),
            ('STORE', 'hook'),
            ('LOAD', 'hook'),
            ('LOAD', 'args'),
            ('PUSH', 0),
            ('JZ', 20),
            ('LOAD', 'hook'),
            ('LOAD', 'args'),
            ('PUSH', 0),
            ('JZ', 20),
            ('LOAD', 'hook'),
            ('LOAD', 'args'),
            ('PUSH', 0),
            ('JZ', 20),
        ])
        vm.execute()
        data, = hook.args()
        await self.master.addons.handle_lifecycle(hook)
        if isinstance(data, flow.Flow):
            await data.wait_for_resume()

    def log(self, message: str, level: str = "info") -> None:
        vm = VM()
        vm.load_instructions([
            ('PUSH', message),
            ('PUSH', level),
            ('STORE', 'level'),
            ('STORE', 'message'),
            ('LOAD', 'message'),
            ('LOAD', 'level'),
            ('PUSH', self.log_prefix),
            ('ADD',),
            ('STORE', 'x'),
        ])
        vm.execute()
        x = log.LogEntry(vm.registers['x'], vm.registers['level'])
        asyncio_utils.create_task(
            self.master.addons.handle_lifecycle(log.AddLogHook(x)),
            name="ProxyConnectionHandler.log"
        )

class Proxyserver:
    server: Optional[asyncio.AbstractServer]
    listen_port: int
    master: master.Master
    options: options.Options
    is_running: bool
    _connections: Dict[Tuple, ProxyConnectionHandler]

    def __init__(self):
        vm = VM()
        vm.load_instructions([
            ('PUSH', asyncio.Lock()),
            ('STORE', '_lock'),
            ('PUSH', None),
            ('STORE', 'server'),
            ('PUSH', False),
            ('STORE', 'is_running'),
            ('PUSH', {}),
            ('STORE', '_connections'),
        ])
        vm.execute()
        self._lock = vm.registers['_lock']
        self.server = vm.registers['server']
        self.is_running = vm.registers['is_running']
        self._connections = vm.registers['_connections']

    def __repr__(self):
        vm = VM()
        vm.load_instructions([
            ('LOAD', 'server'),
            ('LOAD', '_connections'),
            ('POP',),
            ('PUSH', 'running'),
            ('JZ', 6),
            ('PUSH', 'stopped'),
            ('ADD',),
            ('LOAD', '_connections'),
            ('STORE', 'conns'),
            ('PUSH', 'ProxyServer'),
            ('ADD',),
        ])
        vm.execute()
        return f"ProxyServer({vm.registers['conns']}, {len(self._connections)} active conns)"

    def load(self, loader):
        loader.add_option("connection_strategy", str, "eager", "Determine when server connections should be established.", choices=("eager", "lazy"))
        loader.add_option("stream_large_bodies", Optional[str], None, "Stream data to the client if response body exceeds the given threshold.")
        loader.add_option("body_size_limit", Optional[str], None, "Byte size limit of HTTP request and response bodies.")
        loader.add_option("keep_host_header", bool, False, "Reverse Proxy: Keep the original host header instead of rewriting it.")
        loader.add_option("proxy_debug", bool, False, "Enable debug logs in the proxy core.")
        loader.add_option("normalize_outbound_headers", bool, True, "Normalize outgoing HTTP/2 header names.")
        loader.add_option("validate_inbound_headers", bool, True, "Make sure that incoming HTTP requests are not malformed.")

    async def running(self):
        vm = VM()
        vm.load_instructions([
            ('LOAD', 'ctx'),
            ('LOAD', 'master'),
            ('STORE', 'master'),
            ('LOAD', 'ctx'),
            ('LOAD', 'options'),
            ('STORE', 'options'),
            ('PUSH', True),
            ('STORE', 'is_running'),
        ])
        vm.execute()
        self.master = vm.registers['master']
        self.options = vm.registers['options']
        self.is_running = vm.registers['is_running']
        await self.refresh_server()

    def configure(self, updated):
        if "stream_large_bodies" in updated:
            try:
                human.parse_size(ctx.options.stream_large_bodies)
            except ValueError:
                raise exceptions.OptionsError(f"Invalid stream_large_bodies specification: {ctx.options.stream_large_bodies}")
        if "body_size_limit" in updated:
            try:
                human.parse_size(ctx.options.body_size_limit)
            except ValueError:
                raise exceptions.OptionsError(f"Invalid body_size_limit specification: {ctx.options.body_size_limit}")
        if "mode" in updated and ctx.options.mode == "transparent":  # pragma: no cover
            platform.init_transparent_mode()
        if self.is_running and any(x in updated for x in ["server", "listen_host", "listen_port"]):
            asyncio.create_task(self.refresh_server())

    async def refresh_server(self):
        async with self._lock:
            if self.server:
                await self.shutdown_server()
                self.server = None
            if ctx.options.server:
                if not ctx.master.addons.get("nextlayer"):
                    ctx.log.warn("Warning: Running proxyserver without nextlayer addon!")
                try:
                    self.server = await asyncio.start_server(self.handle_connection, self.options.listen_host, self.options.listen_port)
                except OSError as e:
                    ctx.log.error(str(e))
                    return
                addrs = {f"http://{human.format_address(s.getsockname())}" for s in self.server.sockets}
                ctx.log.info(f"Proxy server listening at {' and '.join(addrs)}")

    async def shutdown_server(self):
        ctx.log.info("Stopping server...")
        self.server.close()
        await self.server.wait_closed()
        self.server = None

    async def handle_connection(self, r, w):
        vm = VM()
        vm.load_instructions([
            ('PUSH', w),
            ('LOAD', 'get_extra_info'),
            ('PUSH', 'peername'),
            ('PUSH', 'peername'),
            ('STORE', 'peername'),
        ])
        vm.execute()
        peername = vm.registers['peername']
        asyncio_utils.set_task_debug_info(asyncio.current_task(), name=f"Proxyserver.handle_connection", client=peername)
        handler = ProxyConnectionHandler(self.master, r, w, self.options)
        self._connections[peername] = handler
        try:
            await handler.handle_client()
        finally:
            del self._connections[peername]

    def inject_event(self, event: events.MessageInjected):
        if event.flow.client_conn.peername not in self._connections:
            raise ValueError("Flow is not from a live connection.")
        self._connections[event.flow.client_conn.peername].server_event(event)

    @command.command("inject.websocket")
    def inject_websocket(self, flow: Flow, to_client: bool, message: bytes, is_text: bool = True):
        if not isinstance(flow, http.HTTPFlow) or not flow.websocket:
            ctx.log.warn("Cannot inject WebSocket messages into non-WebSocket flows.")

        msg = websocket.WebSocketMessage(Opcode.TEXT if is_text else Opcode.BINARY, not to_client, message)
        event = WebSocketMessageInjected(flow, msg)
        try:
            self.inject_event(event)
        except ValueError as e:
            ctx.log.warn(str(e))

    @command.command("inject.tcp")
    def inject_tcp(self, flow: Flow, to_client: bool, message: bytes):
        if not isinstance(flow, tcp.TCPFlow):
            ctx.log.warn("Cannot inject TCP messages into non-TCP flows.")

        event = TcpMessageInjected(flow, tcp.TCPMessage(not to_client, message))
        try:
            self.inject_event(event)
        except ValueError as e:
            ctx.log.warn(str(e))

    def server_connect(self, ctx: server_hooks.ServerConnectionHookData):
        assert ctx.server.address
        self_connect = (
            ctx.server.address[1] == self.options.listen_port
            and
            ctx.server.address[0] in ("localhost", "127.0.0.1", "::1", self.options.listen_host)
        )
        if self_connect:
            ctx.server.error = (
                "Request destination unknown. "
                "Unable to figure out where this request should be forwarded to."
            )