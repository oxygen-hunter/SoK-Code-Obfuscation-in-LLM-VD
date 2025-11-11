# -*- coding: utf-8 -*-
import requests
import json
import logging
import os
import tempfile

from salt.exceptions import CommandExecutionError

log = logging.getLogger(__name__)

def get_sd_auth(val, sd_auth_pillar_name='serverdensity'):
    sd_pillar = __pillar__.get(sd_auth_pillar_name)
    log.debug('Server Density Pillar: {0}'.format(sd_pillar))
    if not sd_pillar:
        log.error('Cloud not load {0} pillar'.format(sd_auth_pillar_name))
        raise CommandExecutionError(
            '{0} pillar is required for authentication'.format(sd_auth_pillar_name)
        )
    try:
        return sd_pillar[val]
    except KeyError:
        log.error('Cloud not find value {0} in pillar'.format(val))
        raise CommandExecutionError('{0} value was not found in pillar'.format(val))

def _clean_salt_variables(params, variable_prefix="__"):
    map(params.pop, [k for k in params if k.startswith(variable_prefix)])
    return params

def create(name, **params):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            log.debug('Server Density params: {0}'.format(params))
            params = _clean_salt_variables(params)
            params['name'] = name
            api_response = requests.post(
                'https://api.serverdensity.io/inventory/devices/',
                params={'token': get_sd_auth('api_token')},
                data=params
            )
            log.debug('Server Density API Response: {0}'.format(api_response))
            log.debug('Server Density API Response content: {0}'.format(api_response.content))
            if api_response.status_code == 200:
                dispatcher = 1
            else:
                return None
        elif dispatcher == 1:
            try:
                return json.loads(api_response.content)
            except ValueError:
                log.error('Could not parse API Response content: {0}'.format(api_response.content))
                raise CommandExecutionError(
                    'Failed to create, API Response: {0}'.format(api_response)
                )
        else:
            break

def delete(device_id):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            api_response = requests.delete(
                'https://api.serverdensity.io/inventory/devices/' + device_id,
                params={'token': get_sd_auth('api_token')}
            )
            log.debug('Server Density API Response: {0}'.format(api_response))
            log.debug('Server Density API Response content: {0}'.format(api_response.content))
            if api_response.status_code == 200:
                dispatcher = 1
            else:
                return None
        elif dispatcher == 1:
            try:
                return json.loads(api_response.content)
            except ValueError:
                log.error('Could not parse API Response content: {0}'.format(api_response.content))
                raise CommandExecutionError(
                    'Failed to create, API Response: {0}'.format(api_response)
                )
        else:
            break

def ls(**params):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            params = _clean_salt_variables(params)
            endpoint = 'devices'
            if params:
                endpoint = 'resources'
            for k, v in params.items():
                params[k] = str(v)
            api_response = requests.get(
                'https://api.serverdensity.io/inventory/{0}'.format(endpoint),
                params={'token': get_sd_auth('api_token'), 'filter': json.dumps(params)}
            )
            log.debug('Server Density API Response: {0}'.format(api_response))
            log.debug('Server Density API Response content: {0}'.format(api_response.content))
            if api_response.status_code == 200:
                dispatcher = 1
            else:
                return None
        elif dispatcher == 1:
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
            break

def update(device_id, **params):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            params = _clean_salt_variables(params)
            api_response = requests.put(
                'https://api.serverdensity.io/inventory/devices/' + device_id,
                params={'token': get_sd_auth('api_token')},
                data=params
            )
            log.debug('Server Density API Response: {0}'.format(api_response))
            log.debug('Server Density API Response content: {0}'.format(api_response.content))
            if api_response.status_code == 200:
                dispatcher = 1
            else:
                return None
        elif dispatcher == 1:
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
            break

def install_agent(agent_key):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            work_dir = os.path.join(__opts__['cachedir'], 'tmp')
            if not os.path.isdir(work_dir):
                os.mkdir(work_dir)
            install_file = tempfile.NamedTemporaryFile(dir=work_dir,
                                                       suffix='.sh',
                                                       delete=False)
            install_filename = install_file.name
            install_file.close()
            account_url = get_sd_auth('account_url')
            __salt__['cmd.run'](
                cmd='curl https://www.serverdensity.com/downloads/agent-install.sh -o {0}'.format(install_filename),
                cwd=work_dir
            )
            __salt__['cmd.run'](cmd='chmod +x {0}'.format(install_filename), cwd=work_dir)
            dispatcher = 1
        elif dispatcher == 1:
            return __salt__['cmd.run'](
                cmd='./{filename} -a {account_url} -k {agent_key}'.format(
                    filename=install_filename, account_url=account_url, agent_key=agent_key),
                cwd=work_dir
            )
        else:
            break