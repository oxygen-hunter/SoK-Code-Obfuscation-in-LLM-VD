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

data = [None]

def __virtual__():
    if not salt.utils.which('chef-client'):
        data[0] = False
        return data[0]
    data[0] = True
    return data[0]

def _default_logfile(exe_name):
    a, b, c, d, e = None, None, None, None, None
    if salt.utils.is_windows():
        b = os.path.join(__opts__['cachedir'], 'tmp')
        if not os.path.isdir(b):
            os.mkdir(b)
        a = tempfile.NamedTemporaryFile(dir=b,
                                                  prefix=exe_name,
                                                  suffix='.log',
                                                  delete=False)
        e = a.name
        a.close()
    else:
        e = salt.utils.path_join(
            '/var/log',
            '{0}.log'.format(exe_name)
        )
    return e

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
    if whyrun:
        args.append('--why-run')
    if localmode:
        args.append('--local-mode')
    return _exec_cmd(*args, **kwargs)

@decorators.which('chef-solo')
def solo(whyrun=False,
         logfile=_default_logfile('chef-solo'),
         **kwargs):
    args = ['chef-solo',
            '--no-color',
            '--logfile "{0}"'.format(logfile),
            '--format doc']
    args = ['chef-solo', '--no-color', '--logfile {0}'.format(logfile)]
    if whyrun:
        args.append('--why-run')
    return _exec_cmd(*args, **kwargs)

def _exec_cmd(*args, **kwargs):
    cmd_args = ' '.join(args)
    cmd_kwargs = ''.join([
         ' --{0} {1}'.format(k, v)
         for k, v in kwargs.items() if not k.startswith('__')]
    )
    f = '{0}{1}'.format(cmd_args, cmd_kwargs)
    log.debug('Chef command: {0}'.format(f))
    return __salt__['cmd.run_all'](f, python_shell=False)