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
        result["world_path"] = cfg.get("world_module")
        if not result["world_path"]:
            raise ValueError("Did not specify world module")
        result["overworld"] = cfg.get("overworld")
        if not result["overworld"]:
            raise ValueError("Did not specify overworld")
        result["max_workers"] = cfg.get("max_workers")
        if not result["max_workers"]:
            raise ValueError("Did not specify max_workers")
        result["task_name"] = cfg.get("task_name")
        if not result["task_name"]:
            raise ValueError("Did not specify task name")
        task_world = cfg.get("tasks")
        if task_world is None or len(task_world) == 0:
            raise ValueError("task not in config file")

        def process_tasks(items):
            if not items:
                return
            task_name, configuration = items[0]
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
            process_tasks(items[1:])

        process_tasks(list(task_world.items()))

        result["world_opt"] = cfg.get("opt", {})
        result["additional_args"] = cfg.get("additional_args", {})

    return result