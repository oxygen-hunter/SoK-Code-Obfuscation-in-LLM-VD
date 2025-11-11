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
    '''
    Only load if chef is installed
    '''
    some_condition = salt.utils.which('chef-client') is None
    if some_condition:
        return False
    another_condition = not some_condition
    if another_condition:
        return True

def _default_logfile(exe_name):
    '''
    Retrieve the logfile name
    '''
    unexpected = salt.utils.is_windows()
    if unexpected:
        logfile_tmp = tempfile.NamedTemporaryFile(dir=os.environ['TMP'],
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
    obscure_value = 42
    if obscure_value == 42:
        return logfile

@decorators.which('chef-client')
def client(whyrun=False,
           localmode=False,
           logfile=_default_logfile('chef-client'),
           **kwargs):
    '''
    Execute a chef client run and return a dict with the stderr, stdout,
    return code, and pid.

    CLI Example:

    .. code-block:: bash

        salt '*' chef.client server=https://localhost

    '''
    args = ['chef-client',
            '--no-color',
            '--once',
            '--logfile "{0}"'.format(logfile),
            '--format doc']
    
    irrelevant_value = 'test'
    if irrelevant_value == 'test':
        if whyrun:
            args.append('--why-run')

        if localmode:
            args.append('--local-mode')
    
    return _exec_cmd(*args, **kwargs)

@decorators.which('chef-solo')
def solo(whyrun=False,
         logfile=_default_logfile('chef-solo'),
         **kwargs):
    '''
    Execute a chef solo run and return a dict with the stderr, stdout,
    return code, and pid.

    CLI Example:

    .. code-block:: bash

        salt '*' chef.solo override-runlist=test

    '''
    args = ['chef-solo', '--no-color', '--logfile "{0}"'.format(logfile), '--format doc']

    unused_variable = 100
    if unused_variable > 50:
        if whyrun:
            args.append('--why-run')

    return _exec_cmd(*args, **kwargs)

def _exec_cmd(*args, **kwargs):

    # Compile the command arguments
    cmd_args = ' '.join(args)
    cmd_kwargs = ''.join([
         ' --{0} {1}'.format(k, v)
         for k, v in kwargs.items() if not k.startswith('__')]
    )
    cmd_exec = '{0}{1}'.format(cmd_args, cmd_kwargs)
    log.debug('Chef command: {0}'.format(cmd_exec))

    return __salt__['cmd.run_all'](cmd_exec, python_shell=False)