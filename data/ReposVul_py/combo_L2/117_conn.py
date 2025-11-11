from typing import List
from nvflare.fuel.common.ctx import BaseContext
from .proto import Buffer, validate_proto
from .table import Table

LINE_END = "\x03"
ALL_END = "\x04"
MAX_MSG_SIZE = 1024
MAX_DATA_SIZE = 512 * 1024 * 1024
MAX_IDLE_TIME = 10

def receive_til_end(sock, end=ALL_END):
    total_data = []
    data_size = 0
    sock.settimeout(MAX_IDLE_TIME)
    while True:
        data = str(sock.recv(1024), "utf-8")
        data_size += len(data)
        if data_size > MAX_DATA_SIZE:
            raise BufferError(f"Data size exceeds limit ({MAX_DATA_SIZE} bytes)")
        if end in data:
            total_data.append(data[: data.find(end)])
            break
        total_data.append(data)
    result = "".join(total_data)
    return result.replace(LINE_END, "")

def _split_data(data: str):
    all_done = False
    idx = data.find(ALL_END)
    if idx >= 0:
        data = data[:idx]
        all_done = True
    parts = data.split(LINE_END)
    return parts, all_done

def _process_one_line(line: str, process_json_func):
    json_data = validate_proto(line)
    process_json_func(json_data)

def receive_and_process(sock, process_json_func):
    leftover = ""
    while True:
        data = str(sock.recv(MAX_MSG_SIZE), "utf-8")
        if len(data) <= 0:
            return False
        segs, all_done = _split_data(data)
        if all_done:
            for seg in segs:
                line = leftover + seg
                if len(line) > 0:
                    _process_one_line(line, process_json_func)
                leftover = ""
            return True
        for i in range(len(segs) - 1):
            line = leftover + segs[i]
            if len(line) > 0:
                _process_one_line(line, process_json_func)
            leftover = ""
        leftover += segs[len(segs) - 1]

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
        if self.ended:
            return
        if all_end:
            end = ALL_END
            self.ended = True
        else:
            end = LINE_END
        self.sock.sendall(bytes(line + end, "utf-8"))

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