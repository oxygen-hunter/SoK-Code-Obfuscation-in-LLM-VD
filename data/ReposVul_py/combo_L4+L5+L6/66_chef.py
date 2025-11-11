# -*- coding: utf-8 -*-
'''
Execute chef in server or solo mode
'''

# Import Python libs
import logging
import os
import tempfile

# Import Salt libs
import salt.utils
import salt.utils.decorators as decorators

log = logging.getLogger(__name__)


def __virtual__():
    '''
    Only load if chef is installed
    '''
    return salt.utils.which('chef-client') is not None


def _default_logfile(exe_name):
    '''
    Retrieve the logfile name
    '''
    return _logfile_windows(exe_name) if salt.utils.is_windows() else _logfile_unix(exe_name)


def _logfile_windows(exe_name):
    logfile_tmp = tempfile.NamedTemporaryFile(dir=os.environ['TMP'],
                                              prefix=exe_name,
                                              suffix='.log',
                                              delete=False)
    logfile = logfile_tmp.name
    logfile_tmp.close()
    return logfile


def _logfile_unix(exe_name):
    return salt.utils.path_join('/var/log', '{0}.log'.format(exe_name))


@decorators.which('chef-client')
def client(whyrun=False,
           localmode=False,
           logfile=None,
           **kwargs):
    '''
    Execute a chef client run and return a dict with the stderr, stdout,
    return code, and pid.
    '''
    args = _prepare_args('chef-client', logfile or _default_logfile('chef-client'), whyrun, localmode)
    return _exec_cmd(*args, **kwargs)


@decorators.which('chef-solo')
def solo(whyrun=False,
         logfile=None,
         **kwargs):
    '''
    Execute a chef solo run and return a dict with the stderr, stdout,
    return code, and pid.
    '''
    args = _prepare_args('chef-solo', logfile or _default_logfile('chef-solo'), whyrun, localmode=False)
    return _exec_cmd(*args, **kwargs)


def _prepare_args(exe_name, logfile, whyrun, localmode):
    args = [exe_name, '--no-color', '--logfile "{0}"'.format(logfile), '--format doc']
    if whyrun:
        args.append('--why-run')
    if localmode:
        args.append('--local-mode')
    return args


def _exec_cmd(*args, **kwargs):
    return _execute_command(' '.join(args), ''.join([
         ' --{0} {1}'.format(k, v)
         for k, v in kwargs.items() if not k.startswith('__')]
    ))


def _execute_command(cmd_args, cmd_kwargs):
    cmd_exec = '{0}{1}'.format(cmd_args, cmd_kwargs)
    log.debug('Chef command: {0}'.format(cmd_exec))
    return __salt__['cmd.run_all'](cmd_exec, python_shell=False)