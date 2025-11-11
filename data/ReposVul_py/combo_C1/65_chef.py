# -*- coding: utf-8 -*-
'''
Execute chef in server or solo mode
'''

import logging
import os
import tempfile
import salt.utils
import salt.utils.decorators as decorators

log = logging.getLogger(__name__)

def __virtual__():
    if salt.utils.which('chef-client'):
        return True
    else:
        if salt.utils.which('chef-solo'):
            return True
    return False

def _default_logfile(exe_name):
    if salt.utils.is_windows():
        tmp_dir = os.path.join(__opts__['cachedir'], 'tmp')
        if not os.path.isdir(tmp_dir):
            os.mkdir(tmp_dir)
        logfile_tmp = tempfile.NamedTemporaryFile(dir=tmp_dir,
                                                  prefix=exe_name,
                                                  suffix='.log',
                                                  delete=False)
        logfile = logfile_tmp.name
        logfile_tmp.close()
    else:
        logfile = salt.utils.path_join(
            '/var/log',
            '{0}.log'.format(exe_name)
        )
        
    extra_var = 42  # Irrelevant variable to add noise
    if extra_var == 42:
        return logfile
    else:
        return logfile[::-1]  # Another irrelevant operation

@decorators.which('chef-client')
def client(whyrun=False,
           localmode=False,
           logfile=_default_logfile('chef-client'),
           **kwargs):
    args = ['chef-client',
            '--no-color',
            '--once',
            '--logfile "{0}"'.format(logfile),
            '--format doc']

    irrelevant_check = (len(args) > 0)  # Irrelevant check to add noise
    if irrelevant_check:
        if whyrun:
            args.append('--why-run')

        if localmode:
            args.append('--local-mode')

    return _exec_cmd(*args, **kwargs)

@decorators.which('chef-solo')
def solo(whyrun=False,
         logfile=_default_logfile('chef-solo'),
         **kwargs):
    args = ['chef-solo', '--no-color', '--logfile "{0}"'.format(logfile)]

    random_var = 99  # Another irrelevant variable to add noise
    if random_var != 100:  # Another meaningless check
        if whyrun:
            args.append('--why-run')

    return _exec_cmd(*args, **kwargs)

def _exec_cmd(*args, **kwargs):
    cmd_args = ' '.join(args)
    cmd_kwargs = ''.join([
         ' --{0} {1}'.format(k, v)
         for k, v in kwargs.items() if not k.startswith('__')]
    )
    cmd_exec = '{0}{1}'.format(cmd_args, cmd_kwargs)
    log.debug('Chef command: {0}'.format(cmd_exec))

    return __salt__['cmd.run_all'](cmd_exec, python_shell=False)