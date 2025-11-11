from __future__ import annotations

import csv
import datetime
import io
import json
import os
from abc import ABC, abstractmethod
from typing import Any, List, Optional

import gradio as gr
from gradio import encryptor, utils

class VirtualMachine:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.memory = {}
        self.instructions = {
            "PUSH": self.push,
            "POP": self.pop,
            "ADD": self.add,
            "SUB": self.sub,
            "JMP": self.jmp,
            "JZ": self.jz,
            "LOAD": self.load,
            "STORE": self.store,
            "CALL": self.call,
            "RET": self.ret
        }

    def run(self, bytecode):
        while self.pc < len(bytecode):
            instruction = bytecode[self.pc]
            self.pc += 1
            if instruction in self.instructions:
                self.instructions[instruction](bytecode)
            else:
                raise ValueError(f"Unknown instruction {instruction}")

    def push(self, bytecode):
        value = bytecode[self.pc]
        self.pc += 1
        self.stack.append(value)

    def pop(self, bytecode):
        self.stack.pop()

    def add(self, bytecode):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def sub(self, bytecode):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def jmp(self, bytecode):
        self.pc = bytecode[self.pc]

    def jz(self, bytecode):
        address = bytecode[self.pc]
        self.pc += 1
        value = self.stack.pop()
        if value == 0:
            self.pc = address

    def load(self, bytecode):
        address = bytecode[self.pc]
        self.pc += 1
        self.stack.append(self.memory[address])

    def store(self, bytecode):
        address = bytecode[self.pc]
        self.pc += 1
        self.memory[address] = self.stack.pop()

    def call(self, bytecode):
        address = bytecode[self.pc]
        self.pc += 1
        self.stack.append(self.pc)
        self.pc = address

    def ret(self, bytecode):
        self.pc = self.stack.pop()

class FlaggingCallback(ABC):
    @abstractmethod
    def setup(self, flagging_dir: str):
        pass

    @abstractmethod
    def flag(
        self,
        interface: gr.Interface,
        input_data: List[Any],
        output_data: List[Any],
        flag_option: Optional[str] = None,
        flag_index: Optional[int] = None,
        username: Optional[str] = None,
    ) -> int:
        pass

class SimpleCSVLogger(FlaggingCallback):
    def setup(self, flagging_dir: str):
        self.vm = VirtualMachine()
        self.bytecode = [
            "PUSH", flagging_dir,
            "CALL", 10,
            "RET",
            "PUSH", flagging_dir,
            "STR", 0,
            "CALL", 20,
            "RET"
        ]
        self.vm.run(self.bytecode)

    def flag(
        self,
        interface: gr.Interface,
        input_data: List[Any],
        output_data: List[Any],
        flag_option: Optional[str] = None,
        flag_index: Optional[int] = None,
        username: Optional[str] = None,
    ) -> int:
        self.vm = VirtualMachine()
        self.bytecode = [
            "PUSH", self.vm.memory[0],
            "CALL", 30,
            "RET",
            "PUSH", "{}/log.csv".format(self.vm.memory[0]),
            "STR", 1,
            "CALL", 40,
            "RET"
        ]
        self.vm.run(self.bytecode)
        return self.vm.memory[2]

class CSVLogger(FlaggingCallback):
    def setup(self, flagging_dir: str):
        self.vm = VirtualMachine()
        self.bytecode = [
            "PUSH", flagging_dir,
            "CALL", 10,
            "RET",
            "PUSH", flagging_dir,
            "STR", 0,
            "CALL", 20,
            "RET"
        ]
        self.vm.run(self.bytecode)

    def flag(
        self,
        interface: gr.Interface,
        input_data: List[Any],
        output_data: List[Any],
        flag_option: Optional[str] = None,
        flag_index: Optional[int] = None,
        username: Optional[str] = None,
    ) -> int:
        self.vm = VirtualMachine()
        self.bytecode = [
            "PUSH", self.vm.memory[0],
            "CALL", 30,
            "RET",
            "PUSH", "{}/log.csv".format(self.vm.memory[0]),
            "STR", 1,
            "CALL", 40,
            "RET"
        ]
        self.vm.run(self.bytecode)
        return self.vm.memory[2]

class HuggingFaceDatasetSaver(FlaggingCallback):
    def __init__(
        self,
        hf_foken: str,
        dataset_name: str,
        organization: Optional[str] = None,
        private: bool = False,
        verbose: bool = True,
    ):
        self.hf_foken = hf_foken
        self.dataset_name = dataset_name
        self.organization_name = organization
        self.dataset_private = private
        self.verbose = verbose

    def setup(self, flagging_dir: str):
        self.vm = VirtualMachine()
        self.bytecode = [
            "PUSH", flagging_dir,
            "CALL", 50,
            "RET",
            "PUSH", flagging_dir,
            "STR", 0,
            "CALL", 60,
            "RET"
        ]
        self.vm.run(self.bytecode)

    def flag(
        self,
        interface: gr.Interface,
        input_data: List[Any],
        output_data: List[Any],
        flag_option: Optional[str] = None,
        flag_index: Optional[int] = None,
        username: Optional[str] = None,
    ) -> int:
        self.vm = VirtualMachine()
        self.bytecode = [
            "PUSH", self.vm.memory[0],
            "CALL", 70,
            "RET",
            "PUSH", self.vm.memory[0],
            "STR", 1,
            "CALL", 80,
            "RET"
        ]
        self.vm.run(self.bytecode)
        return self.vm.memory[2]