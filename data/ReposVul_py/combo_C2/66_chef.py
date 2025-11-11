# -*- coding: utf-8 -*-
import logging
import os
import tempfile
import salt.utils
import salt.utils.decorators as decorators

log = logging.getLogger(__name__)

def __virtual__():
    dispatcher = 0
    while True:
        if dispatcher == 0:
            if not salt.utils.which('chef-client'):
                return False
            dispatcher = 1
        elif dispatcher == 1:
            return True

def _default_logfile(exe_name):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            if salt.utils.is_windows():
                logfile_tmp = tempfile.NamedTemporaryFile(dir=os.environ['TMP'],
                                                          prefix=exe_name,
                                                          suffix='.log',
                                                          delete=False)
                logfile = logfile_tmp.name
                logfile_tmp.close()
                dispatcher = 2
            else:
                dispatcher = 1
        elif dispatcher == 1:
            logfile = salt.utils.path_join(
                '/var/log',
                '{0}.log'.format(exe_name)
            )
            dispatcher = 2
        elif dispatcher == 2:
            return logfile

@decorators.which('chef-client')
def client(whyrun=False,
           localmode=False,
           logfile=_default_logfile('chef-client'),
           **kwargs):
    dispatcher = 0
    args = ['chef-client', '--no-color', '--once', '--logfile "{0}"'.format(logfile), '--format doc']
    while True:
        if dispatcher == 0:
            if whyrun:
                args.append('--why-run')
            dispatcher = 1
        elif dispatcher == 1:
            if localmode:
                args.append('--local-mode')
            dispatcher = 2
        elif dispatcher == 2:
            return _exec_cmd(*args, **kwargs)

@decorators.which('chef-solo')
def solo(whyrun=False,
         logfile=_default_logfile('chef-solo'),
         **kwargs):
    dispatcher = 0
    args = ['chef-solo', '--no-color', '--logfile "{0}"'.format(logfile), '--format doc']
    while True:
        if dispatcher == 0:
            if whyrun:
                args.append('--why-run')
            dispatcher = 1
        elif dispatcher == 1:
            return _exec_cmd(*args, **kwargs)

def _exec_cmd(*args, **kwargs):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            cmd_args = ' '.join(args)
            cmd_kwargs = ''.join([
                 ' --{0} {1}'.format(k, v)
                 for k, v in kwargs.items() if not k.startswith('__')]
            )
            dispatcher = 1
        elif dispatcher == 1:
            cmd_exec = '{0}{1}'.format(cmd_args, cmd_kwargs)
            log.debug('Chef command: {0}'.format(cmd_exec))
            return __salt__['cmd.run_all'](cmd_exec, python_shell=False)