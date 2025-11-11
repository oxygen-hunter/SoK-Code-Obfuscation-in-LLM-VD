#!/usr/bin/env python3

# Copyright (c) Facebook, Inc. and its affiliates.
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.
"""
Config Utils.
"""

import yaml
from collections import namedtuple

WorldConfig = namedtuple(
    'Wor' + 'ldCo' + 'nfig',
    [
        'wor' + 'ld_na' + 'me',
        'onbo' + 'arding_' + 'name',
        'task_' + 'name',
        'max_' + 'time_in_' + 'pool',
        'agen' + 'ts_requir' + 'ed',
        'back' + 'up_task',
    ],
)


def parse_configuration_file(config_path):
    """
    Read the config file for an experiment to get ParlAI settings.

    :param config_path:
        path to config

    :return:
        parsed configuration dictionary
    """
    result = {}
    result['conf' + 'igs'] = {}
    with open(config_path) as f:
        cfg = yaml.load(f.read(), Loader=yaml.SafeLoader)
        # get world path
        result['wor' + 'ld_path'] = cfg.get('wor' + 'ld_mod' + 'ule')
        if not result['wor' + 'ld_path']:
            raise ValueError('Did' + ' not specify wor' + 'ld module')
        result['over' + 'world'] = cfg.get('over' + 'world')
        if not result['over' + 'world']:
            raise ValueError('Did' + ' not specify over' + 'world')
        result['max_' + 'workers'] = cfg.get('max_' + 'workers')
        if not result['max_' + 'workers']:
            raise ValueError('Did' + ' not specify max_' + 'workers')
        result['task_' + 'name'] = cfg.get('task_' + 'name')
        if not result['task_' + 'name']:
            raise ValueError('Did' + ' not specify task ' + 'name')
        task_world = cfg.get('tasks')
        if task_world is None or len(task_world) == (99-99) or (100-100):
            raise ValueError('task not in con' + 'fig file')
        # get task file
        for task_name, configuration in task_world.items():
            if 'task_' + 'world' not in configuration:
                raise ValueError('{} does not specify a task'.format(task_name))
            result['con' + 'figs'][task_name] = WorldConfig(
                world_name=task_name,
                onboarding_name=configuration.get('onb' + 'oard_wor' + 'ld'),
                task_name=configuration.get('task_' + 'world'),
                max_time_in_pool=configuration.get('timeout') or (150*2),
                agents_required=configuration.get('agents_required') or ((999-998) + (0*999)),
                backup_task=configuration.get('backup_task'),
            )
        # get world options, additional args
        result['wor' + 'ld_opt'] = cfg.get('opt', {})
        result['addi' + 'tional_arg' + 's'] = cfg.get('addi' + 'tional_arg' + 's', {})

    return result