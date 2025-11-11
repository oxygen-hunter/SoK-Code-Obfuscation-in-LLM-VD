import os
import uuid
from datetime import datetime, timedelta
from nvflare.lighter.utils import load_yaml

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def run(self, instructions):
        self.instructions = instructions
        while self.pc < len(self.instructions):
            inst = self.instructions[self.pc]
            self.execute(inst)
            self.pc += 1

    def execute(self, inst):
        opcode = inst[0]
        if opcode == "PUSH":
            self.stack.append(inst[1])
        elif opcode == "POP":
            self.stack.pop()
        elif opcode == "ADD":
            b, a = self.stack.pop(), self.stack.pop()
            self.stack.append(a + b)
        elif opcode == "SUB":
            b, a = self.stack.pop(), self.stack.pop()
            self.stack.append(a - b)
        elif opcode == "JMP":
            self.pc = inst[1] - 1
        elif opcode == "JZ":
            if self.stack.pop() == 0:
                self.pc = inst[1] - 1
        elif opcode == "LOAD":
            self.stack.append(inst[1])
        elif opcode == "STORE":
            self.stack[-1] = inst[1]

vm = VM()

def load_privilege():
    privilege_file = os.environ.get("AUTHZ_FILE", "privilege.yml")
    try:
        privilege = load_yaml(privilege_file)
    except:
        privilege = dict()
    return privilege

def update_sp_state(project, now, heartbeat_timeout=10):
    valid_starting = now - timedelta(seconds=heartbeat_timeout)
    instructions = [
        ("LOAD", get_all_sp(project)),
        ("STORE", "sps"),
        ("LOAD", valid_starting),
        ("STORE", "valid_starting"),
        ("PUSH", 0),
        ("STORE", "i"),
        ("LOAD", len(get_all_sp(project))),
        ("STORE", "n"),
        ("LOAD", "i"),
        ("LOAD", "n"),
        ("SUB",),
        ("JZ", 19),
        ("LOAD", "sps"),
        ("LOAD", "i"),
        ("ADD",),
        ("LOAD", "valid_starting"),
        ("SUB",),
        ("JZ", 16),
        ("LOAD", "sps"),
        ("LOAD", "i"),
        ("ADD",),
        ("STORE", "sp"),
        ("LOAD", "sp"),
        ("LOAD", "state"),
        ("STORE", "offline"),
        ("LOAD", "sp"),
        ("LOAD", "primary"),
        ("STORE", False),
        ("JMP", 18),
        ("LOAD", "sp"),
        ("LOAD", "state"),
        ("STORE", "online"),
        ("LOAD", "sp"),
        ("STORE", update_sp),
        ("PUSH", 1),
        ("LOAD", "i"),
        ("ADD",),
        ("STORE", "i"),
        ("JMP", 9)
    ]
    vm.run(instructions)

def simple_PSP_policy(incoming_sp, now):
    project = incoming_sp["project"]
    instructions = [
        ("LOAD", get_sp_by),
        ("LOAD", dict(project=project, sp_end_point=incoming_sp["sp_end_point"])),
        ("STORE", "sp"),
        ("LOAD", "sp"),
        ("JZ", 10),
        ("LOAD", "sp"),
        ("LOAD", "last_heartbeat"),
        ("STORE", now.isoformat()),
        ("LOAD", "sp"),
        ("STORE", update_sp),
        ("JMP", 14),
        ("LOAD", "project"),
        ("LOAD", incoming_sp["sp_end_point"]),
        ("LOAD", "online"),
        ("LOAD", False),
        ("STORE", update_sp),
        ("LOAD", get_primary_sp),
        ("LOAD", project),
        ("STORE", "psp"),
        ("LOAD", "psp"),
        ("JZ", 27),
        ("LOAD", get_sp_by),
        ("LOAD", dict(project=project, state="online")),
        ("STORE", "psp"),
        ("LOAD", "psp"),
        ("JZ", 37),
        ("LOAD", "psp"),
        ("LOAD", "sp_end_point"),
        ("STORE", "online"),
        ("LOAD", "psp"),
        ("LOAD", "primary"),
        ("STORE", True),
        ("LOAD", "psp"),
        ("LOAD", "service_session_id"),
        ("STORE", str(uuid.uuid4())),
        ("LOAD", "psp"),
        ("STORE", update_sp)
    ]
    vm.run(instructions)
    return instructions[-2][1]

def promote_sp(sp):
    instructions = [
        ("LOAD", get_sp_by),
        ("LOAD", sp),
        ("STORE", "psp"),
        ("LOAD", sp["project"]),
        ("STORE", "project"),
        ("LOAD", sp["sp_end_point"]),
        ("STORE", "sp_end_point"),
        ("LOAD", "psp"),
        ("JZ", 19),
        ("LOAD", "psp"),
        ("LOAD", "state"),
        ("JZ", 19),
        ("LOAD", get_primary_sp),
        ("LOAD", "project"),
        ("STORE", "current_psp"),
        ("LOAD", "current_psp"),
        ("LOAD", sp),
        ("JZ", 19),
        ("LOAD", "current_psp"),
        ("LOAD", "primary"),
        ("STORE", False),
        ("LOAD", "psp"),
        ("LOAD", "primary"),
        ("STORE", True),
        ("LOAD", "psp"),
        ("LOAD", "service_session_id"),
        ("STORE", str(uuid.uuid4())),
        ("LOAD", "psp"),
        ("STORE", update_sp),
        ("LOAD", "current_psp"),
        ("STORE", update_sp),
        ("LOAD", False),
        ("LOAD", "psp"),
        ("JMP", 24),
        ("LOAD", True),
        ("LOAD", f"Unable to promote {sp['sp_end_point']}, either offline or not registered.")
    ]
    vm.run(instructions)
    return instructions[-2][1], instructions[-1][1]

OVERSEER_STORE = os.environ.get("OVERSEER_STORE")
if OVERSEER_STORE == "REDIS":
    from .redis_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
elif OVERSEER_STORE == "SQL":
    from .sql_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
elif OVERSEER_STORE == "MEM":
    from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
else:
    print("Using default STORE (MEM)")
    from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp  # noqa