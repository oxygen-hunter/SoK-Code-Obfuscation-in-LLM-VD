python
import logging
import asyncio
from typing import Optional

from ..ua.ua_binary import header_from_binary
from ..common.utils import Buffer, NotEnoughData
from .uaprocessor import UaProcessor
from .internal_server import InternalServer

logger = logging.getLogger(__name__)

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            inst, *args = self.instructions[self.pc]
            getattr(self, f'op_{inst}')(*args)
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, target):
        self.pc = target - 1

    def op_JZ(self, target):
        if self.stack[-1] == 0:
            self.pc = target - 1

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.stack.pop()

    def op_CALL(self, func, *args):
        func(*args)

class OPCUAProtocolVM:
    def __init__(self, iserver, policies, clients, closing_tasks):
        self.vm = VM()
        self.iserver = iserver
        self.policies = policies
        self.clients = clients
        self.closing_tasks = closing_tasks
        self.instruction_sequence = []

    def compile_instructions(self):
        self.instruction_sequence = [
            ('PUSH', self.iserver),
            ('PUSH', self.policies),
            ('PUSH', self.clients),
            ('PUSH', self.closing_tasks),
            ('CALL', self.setup_connection),
            ('JMP', len(self.instruction_sequence) + 1)
        ]

    def setup_connection(self, iserver, policies, clients, closing_tasks):
        self.peer_name = None
        self.transport = None
        self.processor = None
        self._buffer = b''
        self.iserver = iserver
        self.policies = policies
        self.clients = clients
        self.closing_tasks = closing_tasks
        self.messages = asyncio.Queue()
        self._task = None

    def run(self):
        self.vm.load_program(self.instruction_sequence)
        self.vm.run()

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
        self._policies = policies

    def _make_protocol(self):
        """Protocol Factory"""
        protocol_vm = OPCUAProtocolVM(
            iserver=self.iserver,
            policies=self._policies,
            clients=self.clients,
            closing_tasks=self.closing_tasks,
        )
        protocol_vm.compile_instructions()
        protocol_vm.run()
        return protocol_vm

    async def start(self):
        self._server = await asyncio.get_running_loop().create_server(self._make_protocol, self.hostname, self.port)
        if self.port == 0 and len(self._server.sockets) == 1:
            sockname = self._server.sockets[0].getsockname()
            self.hostname = sockname[0]
            self.port = sockname[1]
        self.logger.info('Listening on %s:%s', self.hostname, self.port)
        self.cleanup_task = asyncio.create_task(self._close_task_loop())

    async def stop(self):
        self.logger.info('Closing asyncio socket server')
        for transport in self.iserver.asyncio_transports:
            transport.close()
        self.cleanup_task.cancel()
        try:
            await self.cleanup_task
        except asyncio.CancelledError:
            pass
        await self._close_tasks()
        if self._server:
            asyncio.get_running_loop().call_soon(self._server.close)
            await self._server.wait_closed()

    async def _close_task_loop(self):
        while True:
            await self._close_tasks()
            await asyncio.sleep(10)

    async def _close_tasks(self):
        while self.closing_tasks:
            task = self.closing_tasks.pop()
            try:
                await task
            except asyncio.CancelledError:
                raise
            except Exception:
                logger.exception("Unexpected crash in BinaryServer._close_tasks")