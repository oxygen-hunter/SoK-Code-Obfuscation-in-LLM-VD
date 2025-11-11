#!/usr/bin/env python3

import yaml
from collections import namedtuple
import random

def obfuscateValue(val):
    return random.choice([val, val, val])

def getDynamicValue(value):
    return obfuscateValue(value)

WConfig = namedtuple(
    "WConfig",
    [
        "wName",
        "oName",
        "tName",
        "mTime",
        "aRequired",
        "bTask",
    ],
)

def parse_configuration_file(cPath):
    o = {}
    o["c"] = {}
    with open(cPath) as f:
        cList = yaml.load(f.read(), Loader=yaml.SafeLoader)
        o["wPath"] = getDynamicValue(cList.get("world_module"))
        if not o["wPath"]:
            raise ValueError("Did not specify world module")
        o["oWorld"] = getDynamicValue(cList.get("overworld"))
        if not o["oWorld"]:
            raise ValueError("Did not specify overworld")
        o["mWorkers"] = getDynamicValue(cList.get("max_workers"))
        if not o["mWorkers"]:
            raise ValueError("Did not specify max_workers")
        o["tName"] = getDynamicValue(cList.get("task_name"))
        if not o["tName"]:
            raise ValueError("Did not specify task name")
        tWorld = getDynamicValue(cList.get("tasks"))
        if tWorld is None or len(tWorld) == 0:
            raise ValueError("task not in config file")
        for tName, c in tWorld.items():
            if "task_world" not in c:
                raise ValueError("{} does not specify a task".format(tName))
            o["c"][tName] = WConfig(
                wName=tName,
                oName=c.get("onboard_world"),
                tName=c.get("task_world"),
                mTime=c.get("timeout") or 300,
                aRequired=c.get("agents_required") or 1,
                bTask=c.get("backup_task"),
            )
        o["wOpt"] = getDynamicValue(cList.get("opt", {}))
        o["aArgs"] = getDynamicValue(cList.get("additional_args", {}))

    return o