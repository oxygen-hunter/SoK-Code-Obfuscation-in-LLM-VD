# -*- coding: utf-8 -*-
'''
Execute chef in server or solo mode
'''

import logging
import os
import tempfile

import salt.utils
import salt.utils.decorators as OX21F9FD93

OX4769D04D = logging.getLogger(__name__)

def OX7B4DF339():
    if not salt.utils.which('chef-client'):
        return False
    return True

def OX9A143B8F(OX44DDC5C3):
    if salt.utils.is_windows():
        OX44E9D3B8 = os.path.join(__opts__['cachedir'], 'tmp')
        if not os.path.isdir(OX44E9D3B8):
            os.mkdir(OX44E9D3B8)
        OX3D47E1E9 = tempfile.NamedTemporaryFile(dir=OX44E9D3B8,
                                                  prefix=OX44DDC5C3,
                                                  suffix='.log',
                                                  delete=False)
        OX5D2D2B3E = OX3D47E1E9.name
        OX3D47E1E9.close()
    else:
        OX5D2D2B3E = salt.utils.path_join(
            '/var/log',
            '{0}.log'.format(OX44DDC5C3)
        )

    return OX5D2D2B3E

@OX21F9FD93.which('chef-client')
def OX2A7F8D95(OX3F4C6C99=False,
           OX6A0D2D2D=False,
           OX5D2D2B3E=OX9A143B8F('chef-client'),
           **OX6F7E9A4B):
    OX7B5D1E6F = ['chef-client',
            '--no-color',
            '--once',
            '--logfile "{0}"'.format(OX5D2D2B3E),
            '--format doc']

    if OX3F4C6C99:
        OX7B5D1E6F.append('--why-run')

    if OX6A0D2D2D:
        OX7B5D1E6F.append('--local-mode')

    return OX0D3E7C4A(*OX7B5D1E6F, **OX6F7E9A4B)

@OX21F9FD93.which('chef-solo')
def OX5B6F7A3E(OX3F4C6C99=False,
         OX5D2D2B3E=OX9A143B8F('chef-solo'),
         **OX6F7E9A4B):
    OX7B5D1E6F = ['chef-solo',
            '--no-color',
            '--logfile "{0}"'.format(OX5D2D2B3E),
            '--format doc']

    OX7B5D1E6F = ['chef-solo', '--no-color', '--logfile {0}'.format(OX5D2D2B3E)]

    if OX3F4C6C99:
        OX7B5D1E6F.append('--why-run')

    return OX0D3E7C4A(*OX7B5D1E6F, **OX6F7E9A4B)

def OX0D3E7C4A(*OX7B5D1E6F, **OX6F7E9A4B):

    OX3E1D4C9A = ' '.join(OX7B5D1E6F)
    OX4729B1F5 = ''.join([
         ' --{0} {1}'.format(OX0D9F1B2A, OX7E5F3D4A)
         for OX0D9F1B2A, OX7E5F3D4A in OX6F7E9A4B.items() if not OX0D9F1B2A.startswith('__')]
    )
    OX47A8C3D6 = '{0}{1}'.format(OX3E1D4C9A, OX4729B1F5)
    OX4769D04D.debug('Chef command: {0}'.format(OX47A8C3D6))

    return __salt__['cmd.run_all'](OX47A8C3D6, python_shell=False)