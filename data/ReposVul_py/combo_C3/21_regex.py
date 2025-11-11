import re
import sys
import os.path

from .. import config, log

if sys.version_info[0] == 2:
    from ConfigParser import ConfigParser
    from StringIO import StringIO
else:
    from configparser import ConfigParser
    from io import StringIO

DEFINED_RIGHTS = {
    "authenticated": "[rw]\nuser:.+\ncollection:.*\npermission:rw",
    "owner_write": "[r]\nuser:.+\ncollection:.*\npermission:r\n"
                   "[w]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:w",
    "owner_only": "[rw]\nuser:.+\ncollection:^%(login)s(/.*)?$\npermission:rw",
}

PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET = range(10)

def vm_exec(instructions):
    stack = []
    pc = 0
    while pc < len(instructions):
        instr = instructions[pc]
        pc += 1
        op = instr[0]
        if op == PUSH:
            stack.append(instr[1])
        elif op == POP:
            stack.pop()
        elif op == ADD:
            b = stack.pop()
            a = stack.pop()
            stack.append(a + b)
        elif op == SUB:
            b = stack.pop()
            a = stack.pop()
            stack.append(a - b)
        elif op == JMP:
            pc = instr[1]
        elif op == JZ:
            if stack.pop() == 0:
                pc = instr[1]
        elif op == LOAD:
            stack.append(globals()[instr[1]])
        elif op == STORE:
            globals()[instr[1]] = stack.pop()
        elif op == CALL:
            stack.append(pc)
            pc = instr[1]
        elif op == RET:
            pc = stack.pop()

def _read_from_sections_vm(user, collection_url, permission):
    instructions = [
        (PUSH, user),
        (PUSH, collection_url),
        (PUSH, permission),
        (CALL, 10),
        (RET,),
        (LOAD, 'os'),
        (LOAD, 'config'),
        (LOAD, 're'),
        (LOAD, 'ConfigParser'),
        (LOAD, 'StringIO'),
        (LOAD, 'DEFINED_RIGHTS'),
        (LOAD, 'log'),
        (STORE, 'os'),
        (STORE, 'config'),
        (STORE, 're'),
        (STORE, 'ConfigParser'),
        (STORE, 'StringIO'),
        (STORE, 'DEFINED_RIGHTS'),
        (STORE, 'log'),
        (LOAD, 'os'),
        (CALL, 20),
        (RET,),
        (LOAD, 'os.path.expanduser'),
        (CALL, 30),
        (RET,),
        (LOAD, 'config.get'),
        (CALL, 40),
        (RET,),
        (LOAD, 'log.LOGGER.debug'),
        (CALL, 50),
        (RET,),
        (LOAD, 'log.LOGGER.error'),
        (CALL, 60),
        (RET,)
    ]
    vm_exec(instructions)
    return _read_from_sections(user, collection_url, permission)

def authorized(user, collection, permission):
    instructions = [
        (PUSH, user),
        (PUSH, collection),
        (PUSH, permission),
        (CALL, 70),
        (RET,),
        (LOAD, 'config'),
        (CALL, 80),
        (RET,),
        (LOAD, '_read_from_sections_vm'),
        (CALL, 90),
        (RET,)
    ]
    vm_exec(instructions)
    return _read_from_sections_vm(user, collection.url.rstrip("/") or "/", permission)