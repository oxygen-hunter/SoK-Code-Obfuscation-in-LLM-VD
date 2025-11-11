# -*- coding: utf-8 -*-
'''
Execute chef in server or solo mode
'''

import logging
import os
import tempfile

import salt.utils
import salt.utils.decorators as OX8A5F7D4

OX935F1C = logging.getLogger(__name__)

def OX9DC6F2():
    '''
    Only load if chef is installed
    '''
    if not salt.utils.which('chef-client'):
        return False
    return True

def OX2E7A9F(exe_name):
    '''
    Retrieve the logfile name
    '''
    if salt.utils.is_windows():
        OX2B3F5A = tempfile.NamedTemporaryFile(dir=os.environ['TMP'],
                                                  prefix=exe_name,
                                                  suffix='.log',
                                                  delete=False)
        OX9D5E3B = OX2B3F5A.name
        OX2B3F5A.close()
    else:
        OX9D5E3B = salt.utils.path_join(
            '/var/log',
            '{0}.log'.format(exe_name)
        )

    return OX9D5E3B

@OX8A5F7D4.which('chef-client')
def OX5A4D2A(OX7C2E8F=False,
           OX4F1E7D=False,
           OX9C1F0B=OX2E7A9F('chef-client'),
           **OX1D3B7E):
    '''
    Execute a chef client run and return a dict with the stderr, stdout,
    return code, and pid.

    CLI Example:

    .. code-block:: bash

        salt '*' chef.client server=https://localhost

    server
        The chef server URL

    client_key
        Set the client key file location

    config
        The configuration file to use

    config-file-jail
        Directory under which config files are allowed to be loaded
        (no client.rb or knife.rb outside this path will be loaded).

    environment
        Set the Chef Environment on the node

    group
        Group to set privilege to

    json-attributes
        Load attributes from a JSON file or URL

    localmode
        Point chef-client at local repository if True

    log_level
        Set the log level (debug, info, warn, error, fatal)

    logfile
        Set the log file location

    node-name
        The node name for this client

    override-runlist
        Replace current run list with specified items for a single run

    pid
        Set the PID file location, defaults to /tmp/chef-client.pid

    run-lock-timeout
        Set maximum duration to wait for another client run to finish,
        default is indefinitely.

    runlist
        Permanently replace current run list with specified items

    user
        User to set privilege to

    validation_key
        Set the validation key file location, used for registering new clients

    whyrun
        Enable whyrun mode when set to True

    '''
    OX3A1C2D = ['chef-client',
            '--no-color',
            '--once',
            '--logfile "{0}"'.format(OX9C1F0B),
            '--format doc']

    if OX7C2E8F:
        OX3A1C2D.append('--why-run')

    if OX4F1E7D:
        OX3A1C2D.append('--local-mode')

    return OX6F2C7D(*OX3A1C2D, **OX1D3B7E)

@OX8A5F7D4.which('chef-solo')
def OX1D5B7F(OX7C2E8F=False,
         OX9C1F0B=OX2E7A9F('chef-solo'),
         **OX1D3B7E):
    '''
    Execute a chef solo run and return a dict with the stderr, stdout,
    return code, and pid.

    CLI Example:

    .. code-block:: bash

        salt '*' chef.solo override-runlist=test

    config
        The configuration file to use

    environment
        Set the Chef Environment on the node

    group
        Group to set privilege to

    json-attributes
        Load attributes from a JSON file or URL

    log_level
        Set the log level (debug, info, warn, error, fatal)

    logfile
        Set the log file location

    node-name
        The node name for this client

    override-runlist
        Replace current run list with specified items for a single run

    recipe-url
        Pull down a remote gzipped tarball of recipes and untar it to
        the cookbook cache

    run-lock-timeout
        Set maximum duration to wait for another client run to finish,
        default is indefinitely.

    user
        User to set privilege to

    whyrun
        Enable whyrun mode when set to True
    '''
    OX3A1C2D = ['chef-solo',
            '--no-color',
            '--logfile "{0}"'.format(OX9C1F0B),
            '--format doc']

    OX3A1C2D = ['chef-solo', '--no-color', '--logfile {0}'.format(OX9C1F0B)]

    if OX7C2E8F:
        OX3A1C2D.append('--why-run')

    return OX6F2C7D(*OX3A1C2D, **OX1D3B7E)

def OX6F2C7D(*OX7E3D5F, **OX1D3B7E):

    OX5C1B3A = ' '.join(OX7E3D5F)
    OX2F3D9E = ''.join([
         ' --{0} {1}'.format(OX0A4C3D, OX9D5E3B)
         for OX0A4C3D, OX9D5E3B in OX1D3B7E.items() if not OX0A4C3D.startswith('__')]
    )
    OX8B3C7D = '{0}{1}'.format(OX5C1B3A, OX2F3D9E)
    OX935F1C.debug('Chef command: {0}'.format(OX8B3C7D))

    return __salt__['cmd.run_all'](OX8B3C7D, python_shell=False)