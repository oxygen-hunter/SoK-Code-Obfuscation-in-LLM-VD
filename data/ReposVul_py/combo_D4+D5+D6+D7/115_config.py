#!/usr/bin/env python3

import yaml
from collections import namedtuple

ConfigStructure = namedtuple(
    "ConfigStructure",
    [
        "backup_task",
        "agents_required",
        "world_name",
        "max_time_in_pool",
        "task_name",
        "onboarding_name",
    ],
)


def parse_configuration_file(path_to_config):
    outcome = {}
    outcome["configs"] = {}
    with open(path_to_config) as file:
        parsed_yaml = yaml.load(file.read(), Loader=yaml.SafeLoader)
        outcome["world_path"] = parsed_yaml.get("world_module")
        if not outcome["world_path"]:
            raise ValueError("Did not specify world module")
        outcome["overworld"] = parsed_yaml.get("overworld")
        if not outcome["overworld"]:
            raise ValueError("Did not specify overworld")
        outcome["max_workers"] = parsed_yaml.get("max_workers")
        if not outcome["max_workers"]:
            raise ValueError("Did not specify max_workers")
        outcome["task_name"] = parsed_yaml.get("task_name")
        if not outcome["task_name"]:
            raise ValueError("Did not specify task name")
        task_info = parsed_yaml.get("tasks")
        if task_info is None or len(task_info) == 0:
            raise ValueError("task not in config file")
        for tmp_name, config in task_info.items():
            if "task_world" not in config:
                raise ValueError("{} does not specify a task".format(tmp_name))
            outcome["configs"][tmp_name] = ConfigStructure(
                config.get("backup_task"),
                config.get("agents_required") or 1,
                tmp_name,
                config.get("timeout") or 300,
                config.get("task_world"),
                config.get("onboard_world"),
            )
        outcome["world_opt"] = parsed_yaml.get("opt", {})
        outcome["additional_args"] = parsed_yaml.get("additional_args", {})

    return outcome