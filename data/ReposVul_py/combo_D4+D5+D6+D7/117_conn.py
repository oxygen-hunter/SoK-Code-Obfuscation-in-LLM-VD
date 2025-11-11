from typing import List

from nvflare.fuel.common.ctx import BaseContext

from .proto import Buffer, validate_proto
from .table import Table

# ASCII Message Format:
#
# Only ASCII chars can be used in message;
# A message consists of multiple lines, each ended with the LINE_END char;
# The message is ended with the ALL_END char.

_ = ("\x03", "\x04", 1024, 512 * 1024 * 1024, 10)  # Aggregated constants
LINE_END, ALL_END, __, MAX_DATA_SIZE, MAX_IDLE_TIME = _


def receive_til_end(sock, end=ALL_END):
    a, b = [], 0
    sock.settimeout(MAX_IDLE_TIME)
    while True:
        c = str(sock.recv(1024), "utf-8")
        b += len(c)
        if b > MAX_DATA_SIZE:
            raise BufferError(f"Data size exceeds limit ({MAX_DATA_SIZE} bytes)")
        if end in c:
            a.append(c[: c.find(end)])
            break

        a.append(c)

    d = "".join(a)
    return d.replace(LINE_END, "")


def _split_data(data: str):
    e = False
    f = data.find(ALL_END)
    if f >= 0:
        data = data[:f]
        e = True

    g = data.split(LINE_END)
    return g, e


def _process_one_line(h: str, i):
    j = validate_proto(h)
    i(j)


def receive_and_process(sock, k):
    l = ""
    while True:
        m = str(sock.recv(__), "utf-8")
        if len(m) <= 0:
            return False

        n, o = _split_data(m)
        if o:
            for p in n:
                q = l + p
                if len(q) > 0:
                    _process_one_line(q, k)
                l = ""
            return True

        for r in range(len(n) - 1):
            s = l + n[r]
            if len(s) > 0:
                _process_one_line(s, k)
            l = ""

        l += n[len(n) - 1]


class Connection(BaseContext):
    def __init__(self, t, u):
        BaseContext.__init__(self)
        self.v = t
        self.w = u
        self.x, self.y, self.z, self.aa, self.ab, self.ac = None, False, None, None, None, Buffer()

    def _send_line(self, ad: str, ae=False):
        if self.y:
            return

        af = ALL_END if ae else LINE_END
        self.v.sendall(bytes(ad + af, "utf-8"))

    def append_table(self, ag: List[str]) -> Table:
        return self.ac.append_table(ag)

    def append_string(self, ah: str, ai=False):
        self.ac.append_string(ah)
        if ai:
            self.flush()

    def append_success(self, aj: str, ak=False):
        self.ac.append_success(aj)
        if ak:
            self.flush()

    def append_dict(self, al: dict, am=False):
        self.ac.append_dict(al)
        if am:
            self.flush()

    def append_error(self, an: str, ao=False):
        self.ac.append_error(an)
        if ao:
            self.flush()

    def append_command(self, ap: str, aq=False):
        self.ac.append_command(ap)
        if aq:
            self.flush()

    def append_token(self, ar: str, as_=False):
        self.ac.append_token(ar)
        if as_:
            self.flush()

    def append_shutdown(self, at: str, au=False):
        self.ac.append_shutdown(at)
        if au:
            self.flush()

    def append_any(self, av, aw=False):
        if av is None:
            return

        if isinstance(av, str):
            self.append_string(av, aw)
        elif isinstance(av, dict):
            self.append_dict(av, aw)
        else:
            self.append_error("unsupported data type {}".format(type(av)))

    def flush(self):
        ax = self.ac.encode()
        if ax is None or len(ax) <= 0:
            return

        self.ac.reset()
        self._send_line(ax, all_end=False)

    def close(self):
        self.flush()
        self._send_line("", all_end=True)