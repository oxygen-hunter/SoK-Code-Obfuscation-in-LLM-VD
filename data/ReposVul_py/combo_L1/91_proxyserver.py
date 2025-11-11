import asyncio
from typing import Dict, Optional, Tuple

from mitmproxy import command as OX7EF8B61F, ctx as OX1F4D9E9E, exceptions as OX7D7B1184, flow as OX3E5B2567, http as OX4C1F2F8A, log as OX5F8B3C2E, master as OX6A7E4F1B, options as OX160C7B2E, platform as OX7BA5C4D7, tcp as OX2E4B8F6C, websocket as OX8DB1A3F4
from mitmproxy.flow import Flow as OX6C2F5D3B
from mitmproxy.proxy import commands as OX5AF2D8C1, events as OX3D7A4B2F, server_hooks as OX4C1F2F8B
from mitmproxy.proxy import server as OX5D2A8B1E
from mitmproxy.proxy.layers.tcp import TcpMessageInjected as OX7C1E9D63
from mitmproxy.proxy.layers.websocket import WebSocketMessageInjected as OX5B7F1D3A
from mitmproxy.utils import asyncio_utils as OX7D8E4C2A, human as OX3F1B8D6C
from wsproto.frame_protocol import Opcode as OX4A6D3F2B


class OX1D8F3E2C(OX5D2A8B1E.StreamConnectionHandler):
    OX6A7E4F1B: OX6A7E4F1B.Master

    def __init__(self, OX6A7E4F1B, OX2F6B4E1C, OX7B4D9E3F, OX160C7B2E):
        self.OX6A7E4F1B = OX6A7E4F1B
        super().__init__(OX2F6B4E1C, OX7B4D9E3F, OX160C7B2E)
        self.OX5E6C7B2A = f"{OX3F1B8D6C.format_address(self.client.peername)}: "

    async def OX4B7F6C8D(self, OX5AF2D8C1: OX5AF2D8C1.StartHook) -> None:
        with self.timeout_watchdog.disarm():
            OX3E5B2567, = OX5AF2D8C1.args()
            await self.OX6A7E4F1B.addons.handle_lifecycle(OX5AF2D8C1)
            if isinstance(OX3E5B2567, OX3E5B2567.Flow):
                await OX3E5B2567.wait_for_resume()

    def OX7C6D2F1B(self, OX2E4B5F8C: str, OX1B3D8F6C: str = "info") -> None:
        OX7B8F3D2A = OX5F8B3C2E.LogEntry(self.OX5E6C7B2A + OX2E4B5F8C, OX1B3D8F6C)
        OX7D8E4C2A.create_task(
            self.OX6A7E4F1B.addons.handle_lifecycle(OX5F8B3C2E.AddLogHook(OX7B8F3D2A)),
            name="ProxyConnectionHandler.log"
        )


class OX4A7E3F2B:
    OX5D2A8B1E: Optional[asyncio.AbstractServer]
    OX3C5A8D6B: int
    OX6A7E4F1B: OX6A7E4F1B.Master
    OX160C7B2E: OX160C7B2E.Options
    OX7E1F3B9C: bool
    OX7C2F5B8A: Dict[Tuple, OX1D8F3E2C]

    def __init__(self):
        self.OX4B2D9E1C = asyncio.Lock()
        self.OX5D2A8B1E = None
        self.OX7E1F3B9C = False
        self.OX7C2F5B8A = {}

    def __repr__(self):
        return f"ProxyServer({'running' if self.OX5D2A8B1E else 'stopped'}, {len(self.OX7C2F5B8A)} active conns)"

    def OX3C8B7E2A(self, OX4A5D6F3B):
        OX4A5D6F3B.add_option(
            "connection_strategy", str, "eager",
            "Determine when server connections should be established. When set to lazy, mitmproxy "
            "tries to defer establishing an upstream connection as long as possible. This makes it possible to "
            "use server replay while being offline. When set to eager, mitmproxy can detect protocols with "
            "server-side greetings, as well as accurately mirror TLS ALPN negotiation.",
            choices=("eager", "lazy")
        )
        OX4A5D6F3B.add_option(
            "stream_large_bodies", Optional[str], None,
            """
            Stream data to the client if response body exceeds the given
            threshold. If streamed, the body will not be stored in any way.
            Understands k/m/g suffixes, i.e. 3m for 3 megabytes.
            """
        )
        OX4A5D6F3B.add_option(
            "body_size_limit", Optional[str], None,
            """
            Byte size limit of HTTP request and response bodies. Understands
            k/m/g suffixes, i.e. 3m for 3 megabytes.
            """
        )
        OX4A5D6F3B.add_option(
            "keep_host_header", bool, False,
            """
            Reverse Proxy: Keep the original host header instead of rewriting it
            to the reverse proxy target.
            """
        )
        OX4A5D6F3B.add_option(
            "proxy_debug", bool, False,
            "Enable debug logs in the proxy core.",
        )
        OX4A5D6F3B.add_option(
            "normalize_outbound_headers", bool, True,
            """
            Normalize outgoing HTTP/2 header names, but emit a warning when doing so.
            HTTP/2 does not allow uppercase header names. This option makes sure that HTTP/2 headers set
            in custom scripts are lowercased before they are sent.
            """,
        )
        OX4A5D6F3B.add_option(
            "validate_inbound_headers", bool, True,
            """
            Make sure that incoming HTTP requests are not malformed.
            Disabling this option makes mitmproxy vulnerable to HTTP smuggling attacks.
            """,
        )

    async def OX5A9F7D3C(self):
        self.OX6A7E4F1B = OX1F4D9E9E.master
        self.OX160C7B2E = OX1F4D9E9E.options
        self.OX7E1F3B9C = True
        await self.OX2F8A7B1C()

    def OX6C8D3F2A(self, OX7E3C1F8B):
        if "stream_large_bodies" in OX7E3C1F8B:
            try:
                OX3F1B8D6C.parse_size(OX1F4D9E9E.options.stream_large_bodies)
            except ValueError:
                raise OX7D7B1184.OptionsError(f"Invalid stream_large_bodies specification: "
                                              f"{OX1F4D9E9E.options.stream_large_bodies}")
        if "body_size_limit" in OX7E3C1F8B:
            try:
                OX3F1B8D6C.parse_size(OX1F4D9E9E.options.body_size_limit)
            except ValueError:
                raise OX7D7B1184.OptionsError(f"Invalid body_size_limit specification: "
                                              f"{OX1F4D9E9E.options.body_size_limit}")
        if "mode" in OX7E3C1F8B and OX1F4D9E9E.options.mode == "transparent":  # pragma: no cover
            OX7BA5C4D7.init_transparent_mode()
        if self.OX7E1F3B9C and any(OX1D3C5F8A in OX7E3C1F8B for OX1D3C5F8A in ["server", "listen_host", "listen_port"]):
            asyncio.create_task(self.OX2F8A7B1C())

    async def OX2F8A7B1C(self):
        async with self.OX4B2D9E1C:
            if self.OX5D2A8B1E:
                await self.OX7D3F1B2A()
                self.OX5D2A8B1E = None
            if OX1F4D9E9E.options.server:
                if not OX1F4D9E9E.master.addons.get("nextlayer"):
                    OX1F4D9E9E.log.warn("Warning: Running proxyserver without nextlayer addon!")
                try:
                    self.OX5D2A8B1E = await asyncio.start_server(
                        self.OX8C3E5F2A,
                        self.OX160C7B2E.listen_host,
                        self.OX160C7B2E.listen_port,
                    )
                except OSError as OX1A3E5C7B:
                    OX1F4D9E9E.log.error(str(OX1A3E5C7B))
                    return
                OX3F2A7B1C = {f"http://{OX3F1B8D6C.format_address(OX1B4D7E6F.getsockname())}" for OX1B4D7E6F in self.OX5D2A8B1E.sockets}
                OX1F4D9E9E.log.info(f"Proxy server listening at {' and '.join(OX3F2A7B1C)}")

    async def OX7D3F1B2A(self):
        OX1F4D9E9E.log.info("Stopping server...")
        self.OX5D2A8B1E.close()
        await self.OX5D2A8B1E.wait_closed()
        self.OX5D2A8B1E = None

    async def OX8C3E5F2A(self, OX2F6B4E1C, OX7B4D9E3F):
        OX4F7B1D6A = OX7B4D9E3F.get_extra_info('peername')
        OX7D8E4C2A.set_task_debug_info(
            asyncio.current_task(),
            name=f"Proxyserver.handle_connection",
            client=OX4F7B1D6A,
        )
        OX1D8F3E2C_instance = OX1D8F3E2C(
            self.OX6A7E4F1B,
            OX2F6B4E1C,
            OX7B4D9E3F,
            self.OX160C7B2E
        )
        self.OX7C2F5B8A[OX4F7B1D6A] = OX1D8F3E2C_instance
        try:
            await OX1D8F3E2C_instance.handle_client()
        finally:
            del self.OX7C2F5B8A[OX4F7B1D6A]

    def OX3D6A8B2F(self, OX3D7A4B2F: OX3D7A4B2F.MessageInjected):
        if OX3D7A4B2F.flow.client_conn.peername not in self.OX7C2F5B8A:
            raise ValueError("Flow is not from a live connection.")
        self.OX7C2F5B8A[OX3D7A4B2F.flow.client_conn.peername].server_event(OX3D7A4B2F)

    @OX7EF8B61F.command("inject.websocket")
    def OX1B7E3F6A(self, OX6C2F5D3B: OX6C2F5D3B, OX3E7C9B1A: bool, OX2D8C7F6B: bytes, OX6A3B4F1E: bool = True):
        if not isinstance(OX6C2F5D3B, OX4C1F2F8A.HTTPFlow) or not OX6C2F5D3B.websocket:
            OX1F4D9E9E.log.warn("Cannot inject WebSocket messages into non-WebSocket flows.")

        OX7A3F8B2C = OX8DB1A3F4.WebSocketMessage(
            OX4A6D3F2B.TEXT if OX6A3B4F1E else OX4A6D3F2B.BINARY,
            not OX3E7C9B1A,
            OX2D8C7F6B
        )
        OX3D7A4B2F_instance = OX5B7F1D3A(OX6C2F5D3B, OX7A3F8B2C)
        try:
            self.OX3D6A8B2F(OX3D7A4B2F_instance)
        except ValueError as OX1A3E5C7B:
            OX1F4D9E9E.log.warn(str(OX1A3E5C7B))

    @OX7EF8B61F.command("inject.tcp")
    def OX2A9F7B3E(self, OX6C2F5D3B: OX6C2F5D3B, OX3E7C9B1A: bool, OX2D8C7F6B: bytes):
        if not isinstance(OX6C2F5D3B, OX2E4B8F6C.TCPFlow):
            OX1F4D9E9E.log.warn("Cannot inject TCP messages into non-TCP flows.")

        OX3D7A4B2F_instance = OX7C1E9D63(OX6C2F5D3B, OX2E4B8F6C.TCPMessage(not OX3E7C9B1A, OX2D8C7F6B))
        try:
            self.OX3D6A8B2F(OX3D7A4B2F_instance)
        except ValueError as OX1A3E5C7B:
            OX1F4D9E9E.log.warn(str(OX1A3E5C7B))

    def OX9F7C2A3B(self, OX4C1F2F8B: OX4C1F2F8B.ServerConnectionHookData):
        assert OX4C1F2F8B.server.address
        OX8F7A3D2B = (
            OX4C1F2F8B.server.address[1] == self.OX160C7B2E.listen_port
            and
            OX4C1F2F8B.server.address[0] in ("localhost", "127.0.0.1", "::1", self.OX160C7B2E.listen_host)
        )
        if OX8F7A3D2B:
            OX4C1F2F8B.server.error = (
                "Request destination unknown. "
                "Unable to figure out where this request should be forwarded to."
            )