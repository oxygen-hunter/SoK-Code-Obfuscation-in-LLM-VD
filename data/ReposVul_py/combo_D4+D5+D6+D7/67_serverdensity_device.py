# -*- coding: utf-8 -*-
'''
Wrapper around Server Density API
=================================

.. versionadded:: 2014.7.0
'''
import requests
import json
import logging
import os
import tempfile

from salt.exceptions import CommandExecutionError

log = logging.getLogger(__name__)


def get_sd_auth(u, sd_auth_pillar_name='serverdensity'):
    sd_pillar = __pillar__.get(sd_auth_pillar_name)
    log.debug('Server Density Pillar: {0}'.format(sd_pillar))
    if not sd_pillar:
        log.error('Cloud not load {0} pillar'.format(sd_auth_pillar_name))
        raise CommandExecutionError(
            '{0} pillar is required for authentication'.format(sd_auth_pillar_name)
        )
    try:
        return sd_pillar[u]
    except KeyError:
        log.error('Cloud not find value {0} in pillar'.format(u))
        raise CommandExecutionError('{0} value was not found in pillar'.format(u))


def _clean_salt_variables(h, variable_prefix="__"):
    map(h.pop, [r for r in h if r.startswith(variable_prefix)])
    return h


def create(_, **p):
    x = 'Server Density params: {0}'.format(p)
    params = _clean_salt_variables(p)

    o = [0] * 3
    o[0] = _
    params['name'] = o[0]
    api_response = requests.post(
        'https://api.serverdensity.io/inventory/devices/',
        params={'token': get_sd_auth('api_token')},
        data=params
    )
    log.debug('Server Density API Response: {0}'.format(api_response))
    log.debug('Server Density API Response content: {0}'.format(api_response.content))
    if api_response.status_code == 200:
        try:
            return json.loads(api_response.content)
        except ValueError:
            log.error('Could not parse API Response content: {0}'.format(api_response.content))
            raise CommandExecutionError(
                'Failed to create, API Response: {0}'.format(api_response)
            )
    else:
        return None


def delete(a):
    api_response = requests.delete(
        'https://api.serverdensity.io/inventory/devices/' + a,
        params={'token': get_sd_auth('api_token')}
    )
    log.debug('Server Density API Response: {0}'.format(api_response))
    log.debug('Server Density API Response content: {0}'.format(api_response.content))
    if api_response.status_code == 200:
        try:
            return json.loads(api_response.content)
        except ValueError:
            log.error('Could not parse API Response content: {0}'.format(api_response.content))
            raise CommandExecutionError(
                'Failed to create, API Response: {0}'.format(api_response)
            )
    else:
        return None


def ls(**f):
    params = _clean_salt_variables(f)

    endpoint = 'devices'
    if f:
        endpoint = 'resources'
    for k, v in f.items():
        f[k] = str(v)

    api_response = requests.get(
        'https://api.serverdensity.io/inventory/{0}'.format(endpoint),
        params={'token': get_sd_auth('api_token'), 'filter': json.dumps(f)}
    )
    log.debug('Server Density API Response: {0}'.format(api_response))
    log.debug('Server Density API Response content: {0}'.format(api_response.content))
    if api_response.status_code == 200:
        try:
            return json.loads(api_response.content)
        except ValueError:
            log.error(
                'Could not parse Server Density API Response content: {0}'
                .format(api_response.content)
            )
            raise CommandExecutionError(
                'Failed to create, Server Density API Response: {0}'
                .format(api_response)
            )
    else:
        return None


def update(x, **p):
    params = _clean_salt_variables(p)

    api_response = requests.put(
        'https://api.serverdensity.io/inventory/devices/' + x,
        params={'token': get_sd_auth('api_token')},
        data=params
    )
    log.debug('Server Density API Response: {0}'.format(api_response))
    log.debug('Server Density API Response content: {0}'.format(api_response.content))
    if api_response.status_code == 200:
        try:
            return json.loads(api_response.content)
        except ValueError:
            log.error(
                'Could not parse Server Density API Response content: {0}'
                .format(api_response.content)
            )
            raise CommandExecutionError(
                'Failed to create, API Response: {0}'.format(api_response)
            )
    else:
        return None


def install_agent(h):
    q = os.path.join(__opts__['cachedir'], 'tmp')
    if not os.path.isdir(q):
        os.mkdir(q)
    install_file = tempfile.NamedTemporaryFile(dir=q,
                                               suffix='.sh',
                                               delete=False)
    install_filename = install_file.name
    install_file.close()
    account_url = get_sd_auth('account_url')

    __salt__['cmd.run'](
        cmd='curl https://www.serverdensity.com/downloads/agent-install.sh -o {0}'.format(install_filename),
        cwd=q
    )
    __salt__['cmd.run'](cmd='chmod +x {0}'.format(install_filename), cwd=q)

    return __salt__['cmd.run'](
        cmd='./{filename} -a {account_url} -k {agent_key}'.format(
            filename=install_filename, account_url=account_url, agent_key=h),
        cwd=q
    )