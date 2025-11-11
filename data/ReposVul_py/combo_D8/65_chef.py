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
    if not salt.utils.which('chef-client'):
        return False
    return True

def _dynamic_logfile_name(exe_name):
    if salt.utils.is_windows():
        tmp_dir = _get_tmp_dir()
        if not os.path.isdir(tmp_dir):
            os.mkdir(tmp_dir)
        logfile_tmp = tempfile.NamedTemporaryFile(dir=tmp_dir,
                                                  prefix=exe_name,
                                                  suffix='.log',
                                                  delete=False)
        logfile = logfile_tmp.name
        logfile_tmp.close()
    else:
        logfile = salt.utils.path_join('/var/log', '{0}.log'.format(exe_name))
    return logfile

@decorators.which('chef-client')
def client(whyrun=False, localmode=False, logfile=_dynamic_logfile_name('chef-client'), **kwargs):
    args = _get_client_args(logfile, whyrun, localmode)
    return _exec_cmd(*args, **kwargs)

def _get_tmp_dir():
    return os.path.join(__opts__['cachedir'], 'tmp')

def _get_client_args(logfile, whyrun, localmode):
    args = ['chef-client', '--no-color', '--once', '--logfile "{0}"'.format(logfile), '--format doc']
    if whyrun:
        args.append('--why-run')
    if localmode:
        args.append('--local-mode')
    return args

@decorators.which('chef-solo')
def solo(whyrun=False, logfile=_dynamic_logfile_name('chef-solo'), **kwargs):
    args = _get_solo_args(logfile, whyrun)
    return _exec_cmd(*args, **kwargs)

def _get_solo_args(logfile, whyrun):
    args = ['chef-solo', '--no-color', '--logfile "{0}"'.format(logfile), '--format doc']
    if whyrun:
        args.append('--why-run')
    return args

def _exec_cmd(*args, **kwargs):
    cmd_args = ' '.join(args)
    cmd_kwargs = ''.join([' --{0} {1}'.format(k, v) for k, v in kwargs.items() if not k.startswith('__')])
    cmd_exec = '{0}{1}'.format(cmd_args, cmd_kwargs)
    log.debug('Chef command: {0}'.format(cmd_exec))
    return __salt__['cmd.run_all'](cmd_exec, python_shell=False)