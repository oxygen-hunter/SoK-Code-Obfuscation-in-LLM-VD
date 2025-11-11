#!/usr/bin/env python3

import yaml
from ctypes import CDLL, c_char_p, c_void_p, Structure, POINTER, c_size_t
from collections import namedtuple

# Load a C library for demonstration (e.g., libc)
libc = CDLL("libc.so.6")

class WorldConfig(Structure):
    _fields_ = [
        ("world_name", c_char_p),
        ("onboarding_name", c_char_p),
        ("task_name", c_char_p),
        ("max_time_in_pool", c_size_t),
        ("agents_required", c_size_t),
        ("backup_task", c_char_p),
    ]

def parse_configuration_file(config_path):
    result = {}
    result["configs"] = {}
    with open(config_path.encode('utf-8')) as f:
        cfg = yaml.load(f.read(), Loader=yaml.SafeLoader)
        result["world_path"] = c_char_p(cfg.get("world_module").encode('utf-8'))
        if not result["world_path"]:
            libc.printf(b"Did not specify world module\n")
            raise ValueError()
        result["overworld"] = c_char_p(cfg.get("overworld").encode('utf-8'))
        if not result["overworld"]:
            libc.printf(b"Did not specify overworld\n")
            raise ValueError()
        result["max_workers"] = c_size_t(cfg.get("max_workers"))
        if not result["max_workers"]:
            libc.printf(b"Did not specify max_workers\n")
            raise ValueError()
        result["task_name"] = c_char_p(cfg.get("task_name").encode('utf-8'))
        if not result["task_name"]:
            libc.printf(b"Did not specify task name\n")
            raise ValueError()
        task_world = cfg.get("tasks")
        if task_world is None or len(task_world) == 0:
            libc.printf(b"task not in config file\n")
            raise ValueError()

        for task_name, configuration in task_world.items():
            if "task_world" not in configuration:
                libc.printf(b"%s does not specify a task\n" % task_name.encode('utf-8'))
                raise ValueError()
            result["configs"][task_name] = WorldConfig(
                world_name=task_name.encode('utf-8'),
                onboarding_name=configuration.get("onboard_world").encode('utf-8'),
                task_name=configuration.get("task_world").encode('utf-8'),
                max_time_in_pool=c_size_t(configuration.get("timeout") or 300),
                agents_required=c_size_t(configuration.get("agents_required") or 1),
                backup_task=configuration.get("backup_task").encode('utf-8'),
            )
        result["world_opt"] = cfg.get("opt", {})
        result["additional_args"] = cfg.get("additional_args", {})

    return result