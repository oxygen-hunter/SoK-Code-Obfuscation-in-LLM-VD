import ast
import base64
import time

from keylime import config
from keylime import keylime_logging
from keylime import crypto
from keylime import json
from keylime import revocation_notifier
from keylime.agentstates import AgentAttestStates
from keylime.failure import Failure, Component
from keylime.tpm.tpm_main import tpm
from keylime.tpm.tpm_abstract import TPM_Utilities
from keylime.common import algorithms, validators
from keylime import ima_file_signatures

logger = keylime_logging.init_logging('cloudverifier_common')

def get_tpm_instance():
    global GLOBAL_TPM_INSTANCE
    if GLOBAL_TPM_INSTANCE is None:
        GLOBAL_TPM_INSTANCE = tpm()
    return GLOBAL_TPM_INSTANCE

def get_AgentAttestStates():
    return AgentAttestStates.get_instance()

def process_quote_response(agent, json_response, agentAttestState) -> Failure:
    failure = Failure(Component.QUOTE_VALIDATION)
    a = [None, None]
    try:
        a[0] = json_response.get("pubkey", None)
        a[1] = json_response["quote"]

        b = [json_response.get("ima_measurement_list", None), 
             json_response.get("ima_measurement_list_entry", 0), 
             json_response.get("mb_measurement_list", None), 
             json_response.get("boottime", 0)]

        logger.debug("received quote:      %s", a[1])
        logger.debug("for nonce:           %s", agent['nonce'])
        logger.debug("received public key: %s", a[0])
        logger.debug("received ima_measurement_list    %s", (b[0] is not None))
        logger.debug("received ima_measurement_list_entry: %d", b[1])
        logger.debug("received boottime: %s", b[3])
        logger.debug("received boot log    %s", (b[2] is not None))
    except Exception as e:
        failure.add_event("invalid_data", {"message": "parsing agents get quote respone failed", "data": e}, False)
        return failure

    if not isinstance(b[1], int):
        raise Exception("ima_measurement_list_entry parameter must be an integer")

    if not isinstance(b[3], int):
        raise Exception("boottime parameter must be an integer")

    if a[0] is None:
        if agent.get('public_key', "") == "" or agent.get('b64_encrypted_V', "") == "":
            logger.error("agent did not provide public key and no key or encrypted_v was cached at CV")
            failure.add_event("no_pubkey", "agent did not provide public key and no key or encrypted_v was cached at CV", False)
            return failure
        agent['provide_V'] = False
        a[0] = agent['public_key']

    c = [json_response.get('hash_alg'), json_response.get('enc_alg'), json_response.get('sign_alg')]

    agent['hash_alg'] = c[0]
    agent['enc_alg'] = c[1]
    agent['sign_alg'] = c[2]

    if not algorithms.is_accepted(c[0], agent['accept_tpm_hash_algs'])\
            or not algorithms.Hash.is_recognized(c[0]):
        logger.error(f"TPM Quote is using an unaccepted hash algorithm: {c[0]}")
        failure.add_event("invalid_hash_alg",
                          {"message": f"TPM Quote is using an unaccepted hash algorithm: {c[0]}", "data": c[0]},
                          False)
        return failure

    if not algorithms.is_accepted(c[1], agent['accept_tpm_encryption_algs']):
        logger.error(f"TPM Quote is using an unaccepted encryption algorithm: {c[1]}")
        failure.add_event("invalid_enc_alg",
                          {"message": f"TPM Quote is using an unaccepted encryption algorithm: {c[1]}", "data": c[1]},
                          False)
        return failure

    if not algorithms.is_accepted(c[2], agent['accept_tpm_signing_algs']):
        logger.error(f"TPM Quote is using an unaccepted signing algorithm: {c[2]}")
        failure.add_event("invalid_sign_alg",
                          {"message": f"TPM Quote is using an unaccepted signing algorithm: {c[2]}", "data": {c[2]}},
                          False)
        return failure

    if b[1] == 0:
        agentAttestState.reset_ima_attestation()
    elif b[1] != agentAttestState.get_next_ima_ml_entry():
        logger.error("Agent did not respond with requested next IMA measurement list entry "
                     f"{agentAttestState.get_next_ima_ml_entry()} but started at {b[1]}")
        failure.add_event("invalid_ima_entry_nb",
                          {"message": "Agent did not respond with requested next IMA measurement list entry",
                           "got": b[1], "expected": agentAttestState.get_next_ima_ml_entry()},
                          False)
    elif not agentAttestState.is_expected_boottime(b[3]):
        agentAttestState.reset_ima_attestation()
        return failure

    agentAttestState.set_boottime(b[3])

    d = [agentAttestState.get_ima_keyrings()]
    d.append(ima_file_signatures.ImaKeyring.from_string(agent['ima_sign_verification_keys']))
    d[0].set_tenant_keyring(d[1])

    quote_validation_failure = get_tpm_instance().check_quote(
        agentAttestState,
        agent['nonce'],
        a[0],
        a[1],
        agent['ak_tpm'],
        agent['tpm_policy'],
        b[0],
        agent['allowlist'],
        algorithms.Hash(c[0]),
        d[0],
        b[2],
        agent['mb_refstate'],
        compressed=(agent['supported_version'] == "1.0"))
    failure.merge(quote_validation_failure)

    if not failure:
        agent['first_verified'] = True

        if a[0] != agent.get('public_key', ""):
            agent['public_key'] = a[0]
            agent['b64_encrypted_V'] = ""
            agent['provide_V'] = True

    return failure

def prepare_v(agent):
    if config.INSECURE_DEBUG:
        logger.debug("b64_V (non encrypted): %s", agent['v'])

    if agent.get('b64_encrypted_V', "") != "":
        b64_encrypted_V = agent['b64_encrypted_V']
        logger.debug("Re-using cached encrypted V")
    else:
        b64_encrypted_V = base64.b64encode(crypto.rsa_encrypt(
            crypto.rsa_import_pubkey(agent['public_key']), base64.b64decode(agent['v'])))
        agent['b64_encrypted_V'] = b64_encrypted_V

    post_data = {
        'encrypted_key': b64_encrypted_V
    }
    return post_data

def prepare_get_quote(agent):
    agentAttestState = get_AgentAttestStates().get_by_agent_id(agent['agent_id'])
    agent['nonce'] = TPM_Utilities.random_password(20)

    e = [ast.literal_eval(agent['tpm_policy']), 
         ast.literal_eval(agent['vtpm_policy'])]

    params = {
        'nonce': agent['nonce'],
        'mask': e[0]['mask'],
        'vmask': e[1]['mask'],
        'ima_ml_entry': agentAttestState.get_next_ima_ml_entry(),
    }
    return params

def process_get_status(agent):
    allowlist = json.loads(agent.allowlist)
    al_len = len(allowlist['allowlist']) if isinstance(allowlist, dict) and 'allowlist' in allowlist else 0

    try:
        mb_refstate = json.loads(agent.mb_refstate)
    except Exception as e:
        logger.warning('Non-fatal problem ocurred while attempting to evaluate agent attribute "mb_refstate" (%s). Will just consider the value of this attribute to be "None"', e.args)
        mb_refstate = None
        logger.debug('The contents of the agent attribute "mb_refstate" are %s', agent.mb_refstate)

    mb_refstate_len = len(mb_refstate['mb_refstate']) if isinstance(mb_refstate, dict) and 'mb_refstate' in mb_refstate else 0
    response = {'operational_state': agent.operational_state,
                'v': agent.v,
                'ip': agent.ip,
                'port': agent.port,
                'tpm_policy': agent.tpm_policy,
                'vtpm_policy': agent.vtpm_policy,
                'meta_data': agent.meta_data,
                'allowlist_len': al_len,
                'mb_refstate_len': mb_refstate_len,
                'accept_tpm_hash_algs': agent.accept_tpm_hash_algs,
                'accept_tpm_encryption_algs': agent.accept_tpm_encryption_algs,
                'accept_tpm_signing_algs': agent.accept_tpm_signing_algs,
                'hash_alg': agent.hash_alg,
                'enc_alg': agent.enc_alg,
                'sign_alg': agent.sign_alg,
                'verifier_id' : agent.verifier_id,
                'verifier_ip' : agent.verifier_ip,
                'verifier_port' : agent.verifier_port,
                'severity_level': agent.severity_level,
                'last_event_id': agent.last_event_id
                }
    return response

def notify_error(agent, msgtype='revocation', event=None):
    send_mq = config.getboolean('cloud_verifier', 'revocation_notifier')
    send_webhook = config.getboolean('cloud_verifier', 'revocation_notifier_webhook', fallback=False)
    if not (send_mq or send_webhook):
        return

    revocation = {'type': msgtype,
                  'ip': agent['ip'],
                  'agent_id': agent['agent_id'],
                  'port': agent['port'],
                  'tpm_policy': agent['tpm_policy'],
                  'vtpm_policy': agent['vtpm_policy'],
                  'meta_data': agent['meta_data'],
                  'event_time': time.asctime()}
    if event:
        revocation['event_id'] = event.event_id
        revocation['severity_label'] = event.severity_label.name
        revocation['context'] = event.context

    tosend = {'msg': json.dumps(revocation).encode('utf-8')}

    if agent['revocation_key'] != "":
        signing_key = crypto.rsa_import_privkey(agent['revocation_key'])
        tosend['signature'] = crypto.rsa_sign(signing_key, tosend['msg'])

    else:
        tosend['signature'] = "none"
    if send_mq:
        revocation_notifier.notify(tosend)
    if send_webhook:
        revocation_notifier.notify_webhook(tosend)

def validate_agent_data(agent_data):
    if agent_data is None:
        return False, None

    lists = json.loads(agent_data['allowlist'])

    is_valid, _, err_msg = validators.valid_exclude_list(lists.get('exclude'))
    if not is_valid:
        err_msg += " Exclude list regex is misformatted. Please correct the issue and try again."

    return is_valid, err_msg

GLOBAL_TPM_INSTANCE = None
DEFAULT_VERIFIER_ID = "default"