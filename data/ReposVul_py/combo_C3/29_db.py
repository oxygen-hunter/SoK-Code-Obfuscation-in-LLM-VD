import os
import time
import json
from secrets import token_bytes, token_hex, randbits
from hashlib import sha256
import asyncio
import aiosqlite as sql
from responses import *

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.prog = []
        self.memory = {}
    
    def execute(self, prog):
        self.prog = prog
        while self.pc < len(self.prog):
            instr = self.prog[self.pc]
            self.pc += 1
            self.dispatch(instr)
    
    def dispatch(self, instr):
        opcode, *args = instr
        if opcode == "PUSH":
            self.stack.append(args[0])
        elif opcode == "POP":
            self.stack.pop()
        elif opcode == "ADD":
            self.stack.append(self.stack.pop() + self.stack.pop())
        elif opcode == "SUB":
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif opcode == "LOAD":
            self.stack.append(self.memory[args[0]])
        elif opcode == "STORE":
            self.memory[args[0]] = self.stack.pop()
        elif opcode == "JMP":
            self.pc = args[0]
        elif opcode == "JZ":
            if self.stack.pop() == 0:
                self.pc = args[0]
    
    def run(self, func, *args):
        self.memory["args"] = args
        self.pc = 0
        func(self)

def vm_client_matches(vm):
    vm.execute([
        ("LOAD", "args"), ("PUSH", 0), ("LOAD", 0), ("LOAD", 1),
        ("PUSH", "SELECT client_id FROM scratchverifier_clients WHERE client_id=? AND token=?"),
        ("CALL", "exec_sql"), ("PUSH", 1), ("JZ", 13),
        ("PUSH", True), ("JMP", 15), ("PUSH", False)
    ])
    return vm.stack.pop()

async def exec_sql(vm):
    db, query, params = vm.memory["db"], vm.stack.pop(), vm.stack.pop()
    async with vm.memory["lock"]:
        await db.execute(query, params)
        result = await db.fetchone()
    vm.stack.append(result is not None)

class Database:
    def __init__(self, session):
        self.vm = VirtualMachine()
        self.vm.memory["db"] = None
        self.vm.memory["lock"] = asyncio.Lock()
        loop = asyncio.get_event_loop()
        self.dbw = loop.run_until_complete(sql.connect(DATABASE_FILENAME))
        self.dbw.row_factory = sql.Row
        self.db = loop.run_until_complete(self.dbw.cursor())
        with open(os.path.join(os.path.dirname(__file__), 'sql', 'startup.sql')) as startup:
            loop.run_until_complete(self.db.executescript(startup.read()))
        self.session = session

    async def client_matches(self, client_id, token):
        self.vm.memory["db"] = self.db
        self.vm.memory["lock"] = self.lock
        self.vm.run(vm_client_matches, client_id, token)
        return self.vm.stack.pop()
    
    # The rest of the original functions would be similarly refactored to use the VM