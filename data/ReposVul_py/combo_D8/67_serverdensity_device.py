# -*- coding: utf-8 -*-
import requests
import json
import logging
import os
import tempfile

from salt.exceptions import CommandExecutionError

log = logging.getLogger(__name__)

def get_sd_auth(val, sd_auth_pillar_name='serverdensity'):
    def get_sd_pillar():
        return __pillar__.get(sd_auth_pillar_name)
    
    sd_pillar = get_sd_pillar()
    log.debug('Server Density Pillar: {0}'.format(sd_pillar))
    if not sd_pillar:
        log.error('Cloud not load {0} pillar'.format(sd_auth_pillar_name))
        raise CommandExecutionError(
            '{0} pillar is required for authentication'.format(sd_auth_pillar_name)
        )

    def get_sd_value():
        return sd_pillar[val]

    try:
        return get_sd_value()
    except KeyError:
        log.error('Cloud not find value {0} in pillar'.format(val))
        raise CommandExecutionError('{0} value was not found in pillar'.format(val))


def _clean_salt_variables(params, variable_prefix="__"):
    def get_keys_to_pop():
        return [k for k in params if k.startswith(variable_prefix)]

    map(params.pop, get_keys_to_pop())
    return params


def create(name, **params):
    log.debug('Server Density params: {0}'.format(params))
    params = _clean_salt_variables(params)

    def set_param_name():
        params['name'] = name

    set_param_name()
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


def delete(device_id):
    def get_delete_url():
        return 'https://api.serverdensity.io/inventory/devices/' + device_id

    api_response = requests.delete(
        get_delete_url(),
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


def ls(**params):
    params = _clean_salt_variables(params)

    def get_endpoint():
        return 'resources' if params else 'devices'

    endpoint = get_endpoint()

    def convert_params():
        for k, v in params.items():
            params[k] = str(v)

    convert_params()

    api_response = requests.get(
        'https://api.serverdensity.io/inventory/{0}'.format(endpoint),
        params={'token': get_sd_auth('api_token'), 'filter': json.dumps(params)}
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


def update(device_id, **params):
    params = _clean_salt_variables(params)

    def get_update_url():
        return 'https://api.serverdensity.io/inventory/devices/' + device_id

    api_response = requests.put(
        get_update_url(),
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


def install_agent(agent_key):
    def get_work_dir():
        return os.path.join(__opts__['cachedir'], 'tmp')

    work_dir = get_work_dir()
    if not os.path.isdir(work_dir):
        os.mkdir(work_dir)
    install_file = tempfile.NamedTemporaryFile(dir=work_dir,
                                                   suffix='.sh',
                                                   delete=False)

    def get_install_filename():
        return install_file.name

    install_filename = get_install_filename()
    install_file.close()

    def get_account_url():
        return get_sd_auth('account_url')

    account_url = get_account_url()

    __salt__['cmd.run'](
        cmd='curl https://www.serverdensity.com/downloads/agent-install.sh -o {0}'.format(install_filename),
        cwd=work_dir
    )
    __salt__['cmd.run'](cmd='chmod +x {0}'.format(install_filename), cwd=work_dir)

    return __salt__['cmd.run'](
        cmd='./{filename} -a {account_url} -k {agent_key}'.format(
            filename=install_filename, account_url=account_url, agent_key=agent_key),
        cwd=work_dir
    )