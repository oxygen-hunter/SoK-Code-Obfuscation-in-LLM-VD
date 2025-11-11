import yaml
from collections import namedtuple

WorldConfig = namedtuple(
    "WorldConfig",
    [
        "world_name",
        "onboarding_name",
        "task_name",
        "max_time_in_pool",
        "agents_required",
        "backup_task",
    ],
)

class StackVM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.memory = {}

    def execute(self, instructions):
        while self.program_counter < len(instructions):
            op, *args = instructions[self.program_counter]
            getattr(self, op)(*args)
            self.program_counter += 1

    def PUSH(self, value):
        self.stack.append(value)

    def POP(self):
        return self.stack.pop()

    def ADD(self):
        a = self.POP()
        b = self.POP()
        self.PUSH(a + b)

    def SUB(self):
        a = self.POP()
        b = self.POP()
        self.PUSH(a - b)

    def JMP(self, target):
        self.program_counter = target - 1

    def JZ(self, target):
        if self.POP() == 0:
            self.JMP(target)

    def LOAD(self, var):
        self.PUSH(self.memory[var])

    def STORE(self, var):
        self.memory[var] = self.POP()

def parse_configuration_file(config_path):
    vm = StackVM()
    instructions = [
        ("PUSH", config_path),
        ("STORE", "config_path"),
        ("LOAD", "config_path"),
        ("PUSH", "r"),
        ("STORE", "result"),
        ("PUSH", {}),
        ("STORE", "configs"),
        ("LOAD", "config_path"),
        ("PUSH", "f"),
        ("STORE", "file_handle"),
        ("PUSH", "yaml"),
        ("LOAD", "file_handle"),
        ("PUSH", "cfg"),
        ("STORE", "yaml_load"),
        ("PUSH", "world_module"),
        ("LOAD", "yaml_load"),
        ("PUSH", "world_path"),
        ("STORE", "world_module"),
        ("LOAD", "world_module"),
        ("PUSH", 0),
        ("JZ", 56),
        ("LOAD", "yaml_load"),
        ("PUSH", "overworld"),
        ("STORE", "overworld"),
        ("LOAD", "overworld"),
        ("PUSH", 0),
        ("JZ", 56),
        ("LOAD", "yaml_load"),
        ("PUSH", "max_workers"),
        ("STORE", "max_workers"),
        ("LOAD", "max_workers"),
        ("PUSH", 0),
        ("JZ", 56),
        ("LOAD", "yaml_load"),
        ("PUSH", "task_name"),
        ("STORE", "task_name"),
        ("LOAD", "task_name"),
        ("PUSH", 0),
        ("JZ", 56),
        ("LOAD", "yaml_load"),
        ("PUSH", "tasks"),
        ("STORE", "task_world"),
        ("LOAD", "task_world"),
        ("PUSH", 0),
        ("JZ", 56),
        ("LOAD", "task_world"),
        ("PUSH", "task_name"),
        ("STORE", "configuration"),
        ("LOAD", "configuration"),
        ("PUSH", "task_world"),
        ("STORE", "task_world"),
        ("LOAD", "task_world"),
        ("PUSH", "task_world"),
        ("STORE", "task"),
        ("LOAD", "task"),
        ("PUSH", 0),
        ("JZ", 56),
        ("PUSH", "opt"),
        ("LOAD", "yaml_load"),
        ("PUSH", "world_opt"),
        ("STORE", "opt"),
        ("PUSH", "additional_args"),
        ("LOAD", "yaml_load"),
        ("PUSH", "additional_args"),
        ("STORE", "additional_args"),
        ("LOAD", "result"),
    ]
    
    vm.execute(instructions)
    return vm.memory["result"]