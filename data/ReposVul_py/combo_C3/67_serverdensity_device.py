import requests
import json
import logging
import os
import tempfile
from salt.exceptions import CommandExecutionError

log = logging.getLogger(__name__)

def vm_interpreter(bytecode):
    stack = []
    pc = 0
    variables = {}

    while pc < len(bytecode):
        instruction = bytecode[pc]
        pc += 1

        if instruction == 'PUSH':
            val = bytecode[pc]
            pc += 1
            stack.append(val)
        elif instruction == 'POP':
            stack.pop()
        elif instruction == 'ADD':
            a = stack.pop()
            b = stack.pop()
            stack.append(a + b)
        elif instruction == 'SUB':
            a = stack.pop()
            b = stack.pop()
            stack.append(a - b)
        elif instruction == 'JMP':
            pc = bytecode[pc]
        elif instruction == 'JZ':
            if stack.pop() == 0:
                pc = bytecode[pc]
            else:
                pc += 1
        elif instruction == 'LOAD':
            var_name = bytecode[pc]
            pc += 1
            stack.append(variables[var_name])
        elif instruction == 'STORE':
            var_name = bytecode[pc]
            pc += 1
            variables[var_name] = stack.pop()
        elif instruction == 'CALL_GET_SD_AUTH':
            val = stack.pop()
            sd_auth_pillar_name = stack.pop()
            result = get_sd_auth(val, sd_auth_pillar_name)
            stack.append(result)
        elif instruction == 'CALL_CREATE':
            name = stack.pop()
            params = stack.pop()
            result = create(name, **params)
            stack.append(result)
        elif instruction == 'CALL_DELETE':
            device_id = stack.pop()
            result = delete(device_id)
            stack.append(result)
        elif instruction == 'CALL_LS':
            params = stack.pop()
            result = ls(**params)
            stack.append(result)
        elif instruction == 'CALL_UPDATE':
            device_id = stack.pop()
            params = stack.pop()
            result = update(device_id, **params)
            stack.append(result)
        elif instruction == 'CALL_INSTALL_AGENT':
            agent_key = stack.pop()
            result = install_agent(agent_key)
            stack.append(result)
        else:
            raise ValueError(f"Unknown instruction: {instruction}")

    return stack.pop() if stack else None

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
    api_response = requests.delete(
        'https://api.serverdensity.io/inventory/devices/' + device_id,
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

    api_response = requests.put(
        'https://api.serverdensity.io/inventory/devices/' + device_id,
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

    return __salt__['cmd.run'](
        cmd='./{filename} -a {account_url} -k {agent_key}'.format(
            filename=install_filename, account_url=account_url, agent_key=agent_key),
        cwd=work_dir
    )

# Example of invoking vm_interpreter with a compiled instruction sequence
def example_usage():
    bytecode = [
        'PUSH', 'api_token', 
        'PUSH', 'serverdensity',
        'CALL_GET_SD_AUTH'
    ]
    return vm_interpreter(bytecode)

# Example usage
result = example_usage()