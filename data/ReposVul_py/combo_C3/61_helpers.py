import sys
import os
import yaml
from pathlib import Path
from flask import current_app, g, request
from tinydb import TinyDB, Query, operations
from urllib.parse import urlparse, urljoin
from elasticsearch import Elasticsearch
import elasticsearch.exceptions

INSTR_PUSH = 0
INSTR_POP = 1
INSTR_ADD = 2
INSTR_SUB = 3
INSTR_JMP = 4
INSTR_JZ = 5
INSTR_LOAD = 6
INSTR_STORE = 7
INSTR_CALL = 8
INSTR_RET = 9

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.memory = {}
        self.running = True

    def execute(self, instructions):
        while self.running and self.pc < len(instructions):
            instruction = instructions[self.pc]
            self.pc += 1
            self.dispatch(instruction)

    def dispatch(self, instruction):
        opcode = instruction[0]
        if opcode == INSTR_PUSH:
            self.stack.append(instruction[1])
        elif opcode == INSTR_POP:
            self.stack.pop()
        elif opcode == INSTR_ADD:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif opcode == INSTR_SUB:
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif opcode == INSTR_JMP:
            self.pc = instruction[1]
        elif opcode == INSTR_JZ:
            if self.stack[-1] == 0:
                self.pc = instruction[1]
        elif opcode == INSTR_LOAD:
            self.stack.append(self.memory[instruction[1]])
        elif opcode == INSTR_STORE:
            self.memory[instruction[1]] = self.stack.pop()
        elif opcode == INSTR_CALL:
            params = instruction[1:]
            self.stack.append(self.pc)
            self.pc = self.memory[params[0]]
        elif opcode == INSTR_RET:
            self.pc = self.stack.pop()
        else:
            self.running = False

def load_config_vm(path=""):
    vm = VM()
    instructions = [
        (INSTR_PUSH, path or current_app.config["INTERNAL_DIR"]),
        (INSTR_PUSH, "/config.yml"),
        (INSTR_ADD,),
        (INSTR_LOAD, 0),
        (INSTR_CALL, "load_yaml"),
    ]
    vm.memory["load_yaml"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def config_diff_vm(curr_key, curr_val, parent_dict, defaults):
    vm = VM()
    instructions = [
        (INSTR_PUSH, curr_key),
        (INSTR_PUSH, curr_val),
        (INSTR_PUSH, parent_dict),
        (INSTR_PUSH, defaults),
        (INSTR_CALL, "config_diff_internal"),
    ]
    vm.memory["config_diff_internal"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def config_diff_internal_vm(k, v, parent_dict, defaults):
    vm = VM()
    instructions = [
        (INSTR_PUSH, k),
        (INSTR_PUSH, v),
        (INSTR_PUSH, parent_dict),
        (INSTR_PUSH, defaults),
        (INSTR_CALL, "config_diff_logic"),
    ]
    vm.memory["config_diff_logic"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def config_diff_logic_vm(k, v, parent_dict, defaults):
    vm = VM()
    instructions = [
        (INSTR_PUSH, type(v) is dict),
        (INSTR_JZ, "diff_leaf"),
        (INSTR_PUSH, list(v.items())),
        (INSTR_CALL, "diff_children"),
        (INSTR_RET,),
        ("diff_leaf",),
        (INSTR_PUSH, defaults[k] == v),
        (INSTR_JZ, "preserve"),
        (INSTR_CALL, "remove_key"),
        (INSTR_PUSH, 0),
        (INSTR_RET,),
        ("preserve",),
        (INSTR_PUSH, 1),
        (INSTR_RET,)
    ]
    vm.memory["remove_key"] = len(instructions)
    instructions.extend([
        (INSTR_PUSH, k),
        (INSTR_PUSH, parent_dict),
        (INSTR_SUB,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def write_config_vm(config: dict):
    vm = VM()
    instructions = [
        (INSTR_PUSH, vars(Config())),
        (INSTR_PUSH, config),
        (INSTR_CALL, "write_config_internal"),
    ]
    vm.memory["write_config_internal"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def write_config_internal_vm(defaults, config):
    vm = VM()
    instructions = [
        (INSTR_PUSH, list(config.items())),
        (INSTR_PUSH, defaults),
        (INSTR_CALL, "process_config_items"),
    ]
    vm.memory["process_config_items"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def load_hooks_vm():
    vm = VM()
    instructions = [
        (INSTR_CALL, "load_user_hooks"),
    ]
    vm.memory["load_user_hooks"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def load_scraper_vm():
    vm = VM()
    instructions = [
        (INSTR_CALL, "load_user_scraper"),
    ]
    vm.memory["load_user_scraper"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def get_db_vm(force_reconnect=False):
    vm = VM()
    instructions = [
        (INSTR_PUSH, "db" not in g or force_reconnect),
        (INSTR_JZ, "get_db"),
        (INSTR_CALL, "connect_db"),
        ("get_db",),
        (INSTR_LOAD, "db"),
        (INSTR_RET,),
    ]
    vm.memory["connect_db"] = len(instructions)
    instructions.extend([
        (INSTR_PUSH, TinyDB(str(Path(current_app.config["INTERNAL_DIR"]) / "db.json"))),
        (INSTR_STORE, "db"),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def get_max_id_vm():
    vm = VM()
    instructions = [
        (INSTR_CALL, "get_db"),
        (INSTR_CALL, "fetch_max_id"),
    ]
    vm.memory["fetch_max_id"] = len(instructions)
    instructions.extend([
        (INSTR_PUSH, []),
        (INSTR_PUSH, Query().name == "max_id"),
        (INSTR_CALL, "search_db"),
        (INSTR_RET,),
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def set_max_id_vm(val):
    vm = VM()
    instructions = [
        (INSTR_CALL, "get_db"),
        (INSTR_PUSH, Query().name == "max_id"),
        (INSTR_CALL, "update_db"),
    ]
    vm.memory["update_db"] = len(instructions)
    instructions.extend([
        (INSTR_PUSH, operations.set("val", val)),
        (INSTR_RET,),
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def test_es_connection_vm(es):
    vm = VM()
    instructions = [
        (INSTR_PUSH, es),
        (INSTR_CALL, "test_connection"),
    ]
    vm.memory["test_connection"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def get_elastic_client_vm(error_if_invalid=True):
    vm = VM()
    instructions = [
        (INSTR_PUSH, not current_app.config["SEARCH_CONF"]["enabled"]),
        (INSTR_PUSH, current_app.config["SEARCH_CONF"]["engine"] != "elasticsearch"),
        (INSTR_ADD,),
        (INSTR_JZ, "get_client"),
        (INSTR_PUSH, None),
        (INSTR_RET,),
        ("get_client",),
        (INSTR_CALL, "create_client"),
    ]
    vm.memory["create_client"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def create_plugin_dir_vm(name):
    vm = VM()
    instructions = [
        (INSTR_PUSH, name),
        (INSTR_PUSH, name.replace("archivy_", "").replace("archivy-", "")),
        (INSTR_CALL, "create_dir"),
    ]
    vm.memory["create_dir"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()

def is_safe_redirect_url_vm(target):
    vm = VM()
    instructions = [
        (INSTR_PUSH, request.host_url),
        (INSTR_PUSH, target),
        (INSTR_CALL, "check_redirect_url"),
    ]
    vm.memory["check_redirect_url"] = len(instructions)
    instructions.extend([
        (INSTR_POP,),
        (INSTR_RET,)
    ])
    vm.execute(instructions)
    return vm.stack.pop()