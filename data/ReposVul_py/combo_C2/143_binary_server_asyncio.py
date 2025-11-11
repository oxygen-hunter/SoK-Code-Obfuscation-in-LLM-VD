import logging
import asyncio
from typing import Optional

from ..ua.ua_binary import header_from_binary
from ..common.utils import Buffer, NotEnoughData
from .uaprocessor import UaProcessor
from .internal_server import InternalServer

logger = logging.getLogger(__name__)


class OPCUAProtocol(asyncio.Protocol):

    def __init__(self, iserver: InternalServer, policies, clients, closing_tasks):
        self.peer_name = None
        self.transport = None
        self.processor = None
        self._buffer = b''
        self.iserver: InternalServer = iserver
        self.policies = policies
        self.clients = clients
        self.closing_tasks = closing_tasks
        self.messages = asyncio.Queue()
        self._task = None

    def __str__(self):
        return f'OPCUAProtocol({self.peer_name}, {self.processor.session})'

    __repr__ = __str__

    def connection_made(self, transport):
        control_flow = 0
        while True:
            if control_flow == 0:
                self.peer_name = transport.get_extra_info('peername')
                logger.info('New connection from %s', self.peer_name)
                self.transport = transport
                self.processor = UaProcessor(self.iserver, self.transport)
                self.processor.set_policies(self.policies)
                control_flow = 1
            elif control_flow == 1:
                self.iserver.asyncio_transports.append(transport)
                self.clients.append(self)
                self._task = asyncio.create_task(self._process_received_message_loop())
                break

    def connection_lost(self, ex):
        control_flow = 0
        while True:
            if control_flow == 0:
                logger.info('Lost connection from %s, %s', self.peer_name, ex)
                self.transport.close()
                self.iserver.asyncio_transports.remove(self.transport)
                closing_task = asyncio.create_task(self.processor.close())
                self.closing_tasks.append(closing_task)
                control_flow = 1
            elif control_flow == 1:
                if self in self.clients:
                    self.clients.remove(self)
                self.messages.put_nowait((None, None))
                self._task.cancel()
                break

    def data_received(self, data):
        control_flow = 0
        self._buffer += data
        while self._buffer:
            try:
                if control_flow == 0:
                    buf = Buffer(self._buffer)
                    try:
                        header = header_from_binary(buf)
                    except NotEnoughData:
                        logger.debug('Not enough data while parsing header from client, empty the buffer')
                        self.transport.close()
                        break
                    control_flow = 1
                elif control_flow == 1:
                    if header.header_size + header.body_size <= header.header_size:
                        logger.error(f'Got malformed header {header}')
                        self.transport.close()
                        break
                    else:
                        if len(buf) < header.body_size:
                            logger.debug('We did not receive enough data from client. Need %s got %s', header.body_size,
                                         len(buf))
                            break
                        self.messages.put_nowait((header, buf))
                        self._buffer = self._buffer[(header.header_size + header.body_size):]
                        control_flow = 0
            except Exception:
                logger.exception('Exception raised while parsing message from client')
                break

    async def _process_received_message_loop(self):
        while True:
            header, buf = await self.messages.get()
            if header is None and buf is None:
                break
            try:
                await self._process_one_msg(header, buf)
            except Exception:
                logger.exception('Exception raised while processing message from client')

    async def _process_one_msg(self, header, buf):
        control_flow = 0
        while True:
            if control_flow == 0:
                logger.debug('_process_received_message %s %s', header.body_size, len(buf))
                ret = await self.processor.process(header, buf)
                control_flow = 1
            elif control_flow == 1:
                if not ret:
                    logger.info('processor returned False, we close connection from %s', self.peer_name)
                    self.transport.close()
                break


class BinaryServer:
    def __init__(self, internal_server: InternalServer, hostname, port):
        self.logger = logging.getLogger(__name__)
        self.hostname = hostname
        self.port = port
        self.iserver: InternalServer = internal_server
        self._server: Optional[asyncio.AbstractServer] = None
        self._policies = []
        self.clients = []
        self.closing_tasks = []
        self.cleanup_task = None

    def set_policies(self, policies):
        control_flow = 0
        while True:
            if control_flow == 0:
                self._policies = policies
                break

    def _make_protocol(self):
        control_flow = 0
        while True:
            if control_flow == 0:
                return OPCUAProtocol(
                    iserver=self.iserver,
                    policies=self._policies,
                    clients=self.clients,
                    closing_tasks=self.closing_tasks,
                )

    async def start(self):
        control_flow = 0
        while True:
            if control_flow == 0:
                self._server = await asyncio.get_running_loop().create_server(self._make_protocol, self.hostname, self.port)
                control_flow = 1
            elif control_flow == 1:
                if self.port == 0 and len(self._server.sockets) == 1:
                    sockname = self._server.sockets[0].getsockname()
                    self.hostname = sockname[0]
                    self.port = sockname[1]
                control_flow = 2
            elif control_flow == 2:
                self.logger.info('Listening on %s:%s', self.hostname, self.port)
                self.cleanup_task = asyncio.create_task(self._close_task_loop())
                break

    async def stop(self):
        control_flow = 0
        while True:
            if control_flow == 0:
                self.logger.info('Closing asyncio socket server')
                for transport in self.iserver.asyncio_transports:
                    transport.close()
                self.cleanup_task.cancel()
                control_flow = 1
            elif control_flow == 1:
                try:
                    await self.cleanup_task
                except asyncio.CancelledError:
                    pass
                await self._close_tasks()
                control_flow = 2
            elif control_flow == 2:
                if self._server:
                    asyncio.get_running_loop().call_soon(self._server.close)
                    await self._server.wait_closed()
                break

    async def _close_task_loop(self):
        while True:
            await self._close_tasks()
            await asyncio.sleep(10)

    async def _close_tasks(self):
        control_flow = 0
        while self.closing_tasks:
            if control_flow == 0:
                task = self.closing_tasks.pop()
                try:
                    await task
                except asyncio.CancelledError:
                    raise
                except Exception:
                    logger.exception("Unexpected crash in BinaryServer._close_tasks")
                control_flow = 1
            elif control_flow == 1:
                continue