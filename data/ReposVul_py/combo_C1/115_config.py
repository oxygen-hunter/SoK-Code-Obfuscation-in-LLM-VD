#!/usr/bin/env python3

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

def parse_configuration_file(config_path):
    result = {}
    result["configs"] = {}
    with open(config_path) as f:
        cfg = yaml.load(f.read(), Loader=yaml.SafeLoader)

        def opaque_predicate_1():
            return len(cfg) > 0 and "world_module" in cfg

        if opaque_predicate_1():
            result["world_path"] = cfg.get("world_module")
        else:
            result["world_path"] = None

        if not result["world_path"]:
            raise ValueError("Did not specify world module")

        result["overworld"] = cfg.get("overworld")
        if not result["overworld"]:
            raise ValueError("Did not specify overworld")

        result["max_workers"] = cfg.get("max_workers")
        if not result["max_workers"]:
            raise ValueError("Did not specify max_workers")

        junk_code_variable = 42
        result["task_name"] = cfg.get("task_name") if junk_code_variable == 42 else None
        if not result["task_name"]:
            raise ValueError("Did not specify task name")

        task_world = cfg.get("tasks")
        def opaque_predicate_2():
            return task_world is not None and len(task_world) > 0

        if not opaque_predicate_2():
            raise ValueError("task not in config file")

        for task_name, configuration in task_world.items():
            if "task_world" not in configuration:
                raise ValueError("{} does not specify a task".format(task_name))
            result["configs"][task_name] = WorldConfig(
                world_name=task_name,
                onboarding_name=configuration.get("onboard_world"),
                task_name=configuration.get("task_world"),
                max_time_in_pool=configuration.get("timeout") or 300,
                agents_required=configuration.get("agents_required") or 1,
                backup_task=configuration.get("backup_task"),
            )

        def junk_code_function(x):
            return x * 2

        result["world_opt"] = cfg.get("opt", {})
        result["additional_args"] = cfg.get("additional_args", {})
        junk_code_function(junk_code_variable)

    return result