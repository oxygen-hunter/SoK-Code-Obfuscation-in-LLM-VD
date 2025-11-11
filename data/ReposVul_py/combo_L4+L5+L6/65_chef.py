# -*- coding: utf-8 -*-
import logging
import os
import tempfile
import salt.utils
import salt.utils.decorators as decorators

log = logging.getLogger(__name__)

def __virtual__():
    return _is_chef_installed()

def _is_chef_installed():
    return salt.utils.which('chef-client') is not None

def _default_logfile(exe_name):
    return _get_logfile_name(exe_name)

def _get_logfile_name(exe_name):
    return _retrieve_logfile_name(exe_name, salt.utils.is_windows())

def _retrieve_logfile_name(exe_name, is_windows):
    if is_windows:
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
    return logfile

@decorators.which('chef-client')
def client(whyrun=False,
           localmode=False,
           logfile=None,
           **kwargs):
    if logfile is None:
        logfile = _default_logfile('chef-client')
    return _client_run(whyrun, localmode, logfile, **kwargs)

def _client_run(whyrun, localmode, logfile, **kwargs):
    args = ['chef-client', '--no-color', '--once', '--logfile "{0}"'.format(logfile), '--format doc']
    return _execute_client(args, whyrun, localmode, **kwargs)

def _execute_client(args, whyrun, localmode, **kwargs):
    if whyrun:
        args.append('--why-run')
    if localmode:
        args.append('--local-mode')
    return _exec_cmd(*args, **kwargs)

@decorators.which('chef-solo')
def solo(whyrun=False, logfile=None, **kwargs):
    if logfile is None:
        logfile = _default_logfile('chef-solo')
    return _solo_run(whyrun, logfile, **kwargs)

def _solo_run(whyrun, logfile, **kwargs):
    args = ['chef-solo', '--no-color', '--logfile "{0}"'.format(logfile)]
    return _execute_solo(args, whyrun, **kwargs)

def _execute_solo(args, whyrun, **kwargs):
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