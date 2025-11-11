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
        self._a = None  # peer_name
        self._b = None  # transport
        self._c = None  # processor
        self._d = b''  # _buffer
        self.e: InternalServer = iserver  # iserver
        self.f = policies  # policies
        self.g = clients  # clients
        self.h = closing_tasks  # closing_tasks
        self.i = asyncio.Queue()  # messages
        self.j = None  # _task

    def __str__(self):
        return f'OPCUAProtocol({self._a}, {self._c.session})'

    __repr__ = __str__

    def connection_made(self, transport):
        self._a = transport.get_extra_info('peername')
        logger.info('New connection from %s', self._a)
        self._b = transport
        self._c = UaProcessor(self.e, self._b)
        self._c.set_policies(self.f)
        self.e.asyncio_transports.append(transport)
        self.g.append(self)
        self.j = asyncio.create_task(self._process_received_message_loop())

    def connection_lost(self, ex):
        logger.info('Lost connection from %s, %s', self._a, ex)
        self._b.close()
        self.e.asyncio_transports.remove(self._b)
        closing_task = asyncio.create_task(self._c.close())
        self.h.append(closing_task)
        if self in self.g:
            self.g.remove(self)
        self.i.put_nowait((None, None))
        self.j.cancel()

    def data_received(self, data):
        self._d += data
        while self._d:
            try:
                buf = Buffer(self._d)
                try:
                    header = header_from_binary(buf)
                except NotEnoughData:
                    logger.debug('Not enough data while parsing header from client, empty the buffer')
                    self._b.close()
                    return
                if header.header_size + header.body_size <= header.header_size:
                    logger.error(f'Got malformed header {header}')
                    self._b.close()
                else:
                    if len(buf) < header.body_size:
                        logger.debug('We did not receive enough data from client. Need %s got %s', header.body_size,
                                     len(buf))
                        return
                    self.i.put_nowait((header, buf))
                    self._d = self._d[(header.header_size + header.body_size):]
            except Exception:
                logger.exception('Exception raised while parsing message from client')
                return

    async def _process_received_message_loop(self):
        while True:
            header, buf = await self.i.get()
            if header is None and buf is None:
                break
            try:
                await self._process_one_msg(header, buf)
            except Exception:
                logger.exception('Exception raised while processing message from client')

    async def _process_one_msg(self, header, buf):
        logger.debug('_process_received_message %s %s', header.body_size, len(buf))
        ret = await self._c.process(header, buf)
        if not ret:
            logger.info('processor returned False, we close connection from %s', self._a)
            self._b.close()
            return


class BinaryServer:
    def __init__(self, internal_server: InternalServer, hostname, port):
        self.a = logging.getLogger(__name__)  # logger
        self.b = hostname  # hostname
        self.c = port  # port
        self.d: InternalServer = internal_server  # iserver
        self._e: Optional[asyncio.AbstractServer] = None  # _server
        self._f = []  # _policies
        self.g = []  # clients
        self.h = []  # closing_tasks
        self.i = None  # cleanup_task

    def set_policies(self, policies):
        self._f = policies

    def _make_protocol(self):
        return OPCUAProtocol(
            iserver=self.d,
            policies=self._f,
            clients=self.g,
            closing_tasks=self.h,
        )

    async def start(self):
        self._e = await asyncio.get_running_loop().create_server(self._make_protocol, self.b, self.c)
        if self.c == 0 and len(self._e.sockets) == 1:
            sockname = self._e.sockets[0].getsockname()
            self.b = sockname[0]
            self.c = sockname[1]
        self.a.info('Listening on %s:%s', self.b, self.c)
        self.i = asyncio.create_task(self._close_task_loop())

    async def stop(self):
        self.a.info('Closing asyncio socket server')
        for transport in self.d.asyncio_transports:
            transport.close()

        self.i.cancel()
        try:
            await self.i
        except asyncio.CancelledError:
            pass
        await self._close_tasks()

        if self._e:
            asyncio.get_running_loop().call_soon(self._e.close)
            await self._e.wait_closed()

    async def _close_task_loop(self):
        while True:
            await self._close_tasks()
            await asyncio.sleep(10)

    async def _close_tasks(self):
        while self.h:
            task = self.h.pop()
            try:
                await task
            except asyncio.CancelledError:
                raise
            except Exception:
                logger.exception("Unexpected crash in BinaryServer._close_tasks")