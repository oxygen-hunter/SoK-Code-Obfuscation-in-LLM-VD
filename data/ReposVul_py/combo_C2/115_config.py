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
    
    # Dispatcher variable and loop for control flow flattening
    dispatcher = 0
    while True:
        if dispatcher == 0:
            f = open(config_path)
            cfg = yaml.load(f.read(), Loader=yaml.SafeLoader)
            dispatcher = 1
        
        elif dispatcher == 1:
            result["world_path"] = cfg.get("world_module")
            if not result["world_path"]:
                dispatcher = 2
            else:
                dispatcher = 3

        elif dispatcher == 2:
            raise ValueError("Did not specify world module")
        
        elif dispatcher == 3:
            result["overworld"] = cfg.get("overworld")
            if not result["overworld"]:
                dispatcher = 4
            else:
                dispatcher = 5
        
        elif dispatcher == 4:
            raise ValueError("Did not specify overworld")
        
        elif dispatcher == 5:
            result["max_workers"] = cfg.get("max_workers")
            if not result["max_workers"]:
                dispatcher = 6
            else:
                dispatcher = 7
        
        elif dispatcher == 6:
            raise ValueError("Did not specify max_workers")
        
        elif dispatcher == 7:
            result["task_name"] = cfg.get("task_name")
            if not result["task_name"]:
                dispatcher = 8
            else:
                dispatcher = 9
        
        elif dispatcher == 8:
            raise ValueError("Did not specify task name")
        
        elif dispatcher == 9:
            task_world = cfg.get("tasks")
            if task_world is None or len(task_world) == 0:
                dispatcher = 10
            else:
                dispatcher = 11
        
        elif dispatcher == 10:
            raise ValueError("task not in config file")
        
        elif dispatcher == 11:
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
            dispatcher = 12
        
        elif dispatcher == 12:
            result["world_opt"] = cfg.get("opt", {})
            result["additional_args"] = cfg.get("additional_args", {})
            dispatcher = 13

        elif dispatcher == 13:
            break

    return result