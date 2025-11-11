# -*- coding: utf-8 -*-
import requests
import json
import logging
import os
import tempfile
from salt.exceptions import CommandExecutionError
OX7B4DF339 = logging.getLogger(__name__)
def OX2F5A4339(OX2B4AAB71, OX5D5A4C6D='serverdensity'):
    OX6B9F3F2E = __pillar__.get(OX5D5A4C6D)
    OX7B4DF339.debug('Server Density Pillar: {0}'.format(OX6B9F3F2E))
    if not OX6B9F3F2E:
        OX7B4DF339.error('Cloud not load {0} pillar'.format(OX5D5A4C6D))
        raise CommandExecutionError(
            '{0} pillar is required for authentication'.format(OX5D5A4C6D)
        )
    try:
        return OX6B9F3F2E[OX2B4AAB71]
    except KeyError:
        OX7B4DF339.error('Cloud not find value {0} in pillar'.format(OX2B4AAB71))
        raise CommandExecutionError('{0} value was not found in pillar'.format(OX2B4AAB71))
def OX648CFC8B(OX6E9A9F3A, OX78C1F1E8="__"):
    map(OX6E9A9F3A.pop, [OX5B9F3B4D for OX5B9F3B4D in OX6E9A9F3A if OX5B9F3B4D.startswith(OX78C1F1E8)])
    return OX6E9A9F3A
def OX1C7A4B50(OX56E1C3A3, **OX6E9A9F3A):
    OX7B4DF339.debug('Server Density params: {0}'.format(OX6E9A9F3A))
    OX6E9A9F3A = OX648CFC8B(OX6E9A9F3A)
    OX6E9A9F3A['name'] = OX56E1C3A3
    OX656A1F3C = requests.post(
        'https://api.serverdensity.io/inventory/devices/',
        params={'token': OX2F5A4339('api_token')},
        data=OX6E9A9F3A
    )
    OX7B4DF339.debug('Server Density API Response: {0}'.format(OX656A1F3C))
    OX7B4DF339.debug('Server Density API Response content: {0}'.format(OX656A1F3C.content))
    if OX656A1F3C.status_code == 200:
        try:
            return json.loads(OX656A1F3C.content)
        except ValueError:
            OX7B4DF339.error('Could not parse API Response content: {0}'.format(OX656A1F3C.content))
            raise CommandExecutionError(
                'Failed to create, API Response: {0}'.format(OX656A1F3C)
            )
    else:
        return None
def OX1D4B3A41(OX760B5A2E):
    OX656A1F3C = requests.delete(
        'https://api.serverdensity.io/inventory/devices/' + OX760B5A2E,
        params={'token': OX2F5A4339('api_token')}
    )
    OX7B4DF339.debug('Server Density API Response: {0}'.format(OX656A1F3C))
    OX7B4DF339.debug('Server Density API Response content: {0}'.format(OX656A1F3C.content))
    if OX656A1F3C.status_code == 200:
        try:
            return json.loads(OX656A1F3C.content)
        except ValueError:
            OX7B4DF339.error('Could not parse API Response content: {0}'.format(OX656A1F3C.content))
            raise CommandExecutionError(
                'Failed to create, API Response: {0}'.format(OX656A1F3C)
            )
    else:
        return None
def OX5B9F1C4F(**OX6E9A9F3A):
    OX6E9A9F3A = OX648CFC8B(OX6E9A9F3A)
    OX3E4B6C2A = 'devices'
    if OX6E9A9F3A:
        OX3E4B6C2A = 'resources'
    for OX5B9F3B4D, OX4B1A3F2C in OX6E9A9F3A.items():
        OX6E9A9F3A[OX5B9F3B4D] = str(OX4B1A3F2C)
    OX656A1F3C = requests.get(
        'https://api.serverdensity.io/inventory/{0}'.format(OX3E4B6C2A),
        params={'token': OX2F5A4339('api_token'), 'filter': json.dumps(OX6E9A9F3A)}
    )
    OX7B4DF339.debug('Server Density API Response: {0}'.format(OX656A1F3C))
    OX7B4DF339.debug('Server Density API Response content: {0}'.format(OX656A1F3C.content))
    if OX656A1F3C.status_code == 200:
        try:
            return json.loads(OX656A1F3C.content)
        except ValueError:
            OX7B4DF339.error(
                'Could not parse Server Density API Response content: {0}'
                .format(OX656A1F3C.content)
            )
            raise CommandExecutionError(
                'Failed to create, Server Density API Response: {0}'
                .format(OX656A1F3C)
            )
    else:
        return None
def OX2D7A4C5B(OX760B5A2E, **OX6E9A9F3A):
    OX6E9A9F3A = OX648CFC8B(OX6E9A9F3A)
    OX656A1F3C = requests.put(
        'https://api.serverdensity.io/inventory/devices/' + OX760B5A2E,
        params={'token': OX2F5A4339('api_token')},
        data=OX6E9A9F3A
    )
    OX7B4DF339.debug('Server Density API Response: {0}'.format(OX656A1F3C))
    OX7B4DF339.debug('Server Density API Response content: {0}'.format(OX656A1F3C.content))
    if OX656A1F3C.status_code == 200:
        try:
            return json.loads(OX656A1F3C.content)
        except ValueError:
            OX7B4DF339.error(
                'Could not parse Server Density API Response content: {0}'
                .format(OX656A1F3C.content)
            )
            raise CommandExecutionError(
                'Failed to create, API Response: {0}'.format(OX656A1F3C)
            )
    else:
        return None
def OX5C1F4B0D(OX6C3A2F1A):
    OX7A6D2E1F = os.path.join(__opts__['cachedir'], 'tmp')
    if not os.path.isdir(OX7A6D2E1F):
        os.mkdir(OX7A6D2E1F)
    OX1A3F2B7C = tempfile.NamedTemporaryFile(dir=OX7A6D2E1F,
                                                   suffix='.sh',
                                                   delete=False)
    OX3B4C2A6D = OX1A3F2B7C.name
    OX1A3F2B7C.close()
    OX4C6B5A3E = OX2F5A4339('account_url')
    __salt__['cmd.run'](
        cmd='curl https://www.serverdensity.com/downloads/agent-install.sh -o {0}'.format(OX3B4C2A6D),
        cwd=OX7A6D2E1F
    )
    __salt__['cmd.run'](cmd='chmod +x {0}'.format(OX3B4C2A6D), cwd=OX7A6D2E1F)
    return __salt__['cmd.run'](
        cmd='./{filename} -a {account_url} -k {agent_key}'.format(
            filename=OX3B4C2A6D, account_url=OX4C6B5A3E, agent_key=OX6C3A2F1A),
        cwd=OX7A6D2E1F
    )