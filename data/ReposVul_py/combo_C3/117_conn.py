from typing import List
from nvflare.fuel.common.ctx import BaseContext
from .proto import Buffer, validate_proto
from .table import Table

LINE_END = "\x03"
ALL_END = "\x04"
MAX_MSG_SIZE = 1024
MAX_DATA_SIZE = 512 * 1024 * 1024
MAX_IDLE_TIME = 10

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running:
            instr = self.instructions[self.pc]
            self.pc += 1
            op = instr[0]
            if op == "PUSH":
                self.stack.append(instr[1])
            elif op == "POP":
                self.stack.pop()
            elif op == "ADD":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif op == "SUB":
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif op == "JMP":
                self.pc = instr[1]
            elif op == "JZ":
                if self.stack.pop() == 0:
                    self.pc = instr[1]
            elif op == "LOAD":
                self.stack.append(self.stack[instr[1]])
            elif op == "STORE":
                self.stack[instr[1]] = self.stack.pop()
            elif op == "CALL":
                func, args = instr[1], instr[2]
                func(*args)
            elif op == "HALT":
                self.running = False

def receive_til_end(sock, end=ALL_END):
    vm = VM()
    vm.load_instructions([
        ("PUSH", []),
        ("PUSH", 0),
        ("CALL", sock.settimeout, (MAX_IDLE_TIME,)),
        ("PUSH", True),
        ("STORE", 0),
        ("PUSH", 0),
        ("STORE", 1),
        ("PUSH", 0),
        ("STORE", 2),
        ("PUSH", []),
        ("STORE", 3),
        ("PUSH", sock),
        ("CALL", sock.recv, (1024,)),
        ("CALL", str, ("utf-8",)),
        ("CALL", len, ()),
        ("ADD",),
        ("LOAD", 1),
        ("ADD",),
        ("STORE", 1),
        ("LOAD", 1),
        ("PUSH", MAX_DATA_SIZE),
        ("SUB",),
        ("JZ", 34),
        ("LOAD", 0),
        ("PUSH", end),
        ("CALL", str.find, ()),
        ("PUSH", -1),
        ("SUB",),
        ("JZ", 29),
        ("LOAD", 3),
        ("LOAD", 4),
        ("LOAD", 2),
        ("CALL", str.find, ()),
        ("PUSH", 0),
        ("SUB",),
        ("CALL", list.append, ()),
        ("HALT",),
        ("LOAD", 3),
        ("LOAD", 4),
        ("CALL", list.append, ()),
        ("HALT",),
        ("LOAD", 0),
        ("JMP", 9)
    ])
    vm.run()
    result = "".join(vm.stack[0])
    return result.replace(LINE_END, "")

def receive_and_process(sock, process_json_func):
    vm = VM()
    vm.load_instructions([
        ("PUSH", ""),
        ("STORE", 0),
        ("PUSH", sock),
        ("CALL", sock.recv, (MAX_MSG_SIZE,)),
        ("CALL", str, ("utf-8",)),
        ("CALL", len, ()),
        ("PUSH", 0),
        ("SUB",),
        ("JZ", 21),
        ("CALL", _split_data, ()),
        ("PUSH", 0),
        ("STORE", 1),
        ("JZ", 20),
        ("PUSH", 0),
        ("STORE", 2),
        ("LOAD", 1),
        ("LOAD", 2),
        ("CALL", enumerate, ()),
        ("STORE", 3),
        ("LOAD", 3),
        ("CALL", list.__iter__, ()),
        ("STORE", 4),
        ("LOAD", 2),
        ("CALL", list.__len__, ()),
        ("JZ", 19),
        ("LOAD", 0),
        ("LOAD", 2),
        ("CALL", list.__getitem__, ()),
        ("ADD",),
        ("CALL", _process_one_line, (process_json_func,)),
        ("JMP", 12),
        ("LOAD", 0),
        ("LOAD", 2),
        ("CALL", list.__getitem__, ()),
        ("ADD",),
        ("JMP", 7),
        ("HALT",),
        ("LOAD", 0),
        ("CALL", _split_data, ()),
        ("STORE", 1),
        ("LOAD", 1),
        ("JZ", 18),
        ("PUSH", True),
        ("HALT",),
    ])
    vm.run()
    return vm.stack[0]

class Connection(BaseContext):
    def __init__(self, sock, server):
        BaseContext.__init__(self)
        self.sock = sock
        self.server = server
        self.app_ctx = None
        self.ended = False
        self.request = None
        self.command = None
        self.args = None
        self.buffer = Buffer()

    def _send_line(self, line: str, all_end=False):
        vm = VM()
        vm.load_instructions([
            ("PUSH", self.ended),
            ("JZ", 5),
            ("HALT",),
            ("PUSH", all_end),
            ("JZ", 14),
            ("PUSH", ALL_END),
            ("STORE", 0),
            ("PUSH", True),
            ("STORE", 1),
            ("JMP", 10),
            ("PUSH", LINE_END),
            ("STORE", 0),
            ("PUSH", False),
            ("STORE", 1),
            ("LOAD", 0),
            ("LOAD", line),
            ("ADD",),
            ("CALL", bytes, ("utf-8",)),
            ("CALL", self.sock.sendall, ()),
            ("HALT",),
        ])
        vm.run()

    def append_table(self, headers: List[str]) -> Table:
        return self.buffer.append_table(headers)

    def append_string(self, data: str, flush=False):
        self.buffer.append_string(data)
        if flush:
            self.flush()

    def append_success(self, data: str, flush=False):
        self.buffer.append_success(data)
        if flush:
            self.flush()

    def append_dict(self, data: dict, flush=False):
        self.buffer.append_dict(data)
        if flush:
            self.flush()

    def append_error(self, data: str, flush=False):
        self.buffer.append_error(data)
        if flush:
            self.flush()

    def append_command(self, cmd: str, flush=False):
        self.buffer.append_command(cmd)
        if flush:
            self.flush()

    def append_token(self, token: str, flush=False):
        self.buffer.append_token(token)
        if flush:
            self.flush()

    def append_shutdown(self, msg: str, flush=False):
        self.buffer.append_shutdown(msg)
        if flush:
            self.flush()

    def append_any(self, data, flush=False):
        if data is None:
            return

        if isinstance(data, str):
            self.append_string(data, flush)
        elif isinstance(data, dict):
            self.append_dict(data, flush)
        else:
            self.append_error("unsupported data type {}".format(type(data)))

    def flush(self):
        line = self.buffer.encode()
        if line is None or len(line) <= 0:
            return

        self.buffer.reset()
        self._send_line(line, all_end=False)

    def close(self):
        self.flush()
        self._send_line("", all_end=True)