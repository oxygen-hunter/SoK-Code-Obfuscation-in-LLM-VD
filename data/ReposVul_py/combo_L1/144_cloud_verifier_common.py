import ast
import base64
import time

from keylime import config
from keylime import keylime_logging
from keylime import crypto
from keylime import json
from keylime import revocation_notifier
from keylime.agentstates import AgentAttestStates as OX7B4DF339
from keylime.failure import Failure as OX7B4DF340, Component as OX7B4DF341
from keylime.tpm.tpm_main import tpm as OX7B4DF342
from keylime.tpm.tpm_abstract import TPM_Utilities as OX7B4DF343
from keylime.common import algorithms as OX7B4DF344, validators as OX7B4DF345
from keylime import ima_file_signatures as OX7B4DF346

OX7B4DF347 = keylime_logging.init_logging('cloudverifier_common')

OX7B4DF348 = None
OX7B4DF349 = "default"

def OX7B4DF350():
    global OX7B4DF348
    if OX7B4DF348 is None:
        OX7B4DF348 = OX7B4DF342()
    return OX7B4DF348

def OX7B4DF351():
    return OX7B4DF339.get_instance()

def OX7B4DF352(OX7B4DF353, OX7B4DF354, OX7B4DF355) -> OX7B4DF340:
    OX7B4DF356 = OX7B4DF340(OX7B4DF341.QUOTE_VALIDATION)
    OX7B4DF357 = None
    OX7B4DF358 = None
    try:
        OX7B4DF357 = OX7B4DF354.get("pubkey", None)
        OX7B4DF358 = OX7B4DF354["quote"]

        OX7B4DF359 = OX7B4DF354.get("ima_measurement_list", None)
        OX7B4DF360 = OX7B4DF354.get("ima_measurement_list_entry", 0)
        OX7B4DF361 = OX7B4DF354.get("mb_measurement_list", None)
        OX7B4DF362 = OX7B4DF354.get("boottime", 0)

        OX7B4DF347.debug("received quote:      %s", OX7B4DF358)
        OX7B4DF347.debug("for nonce:           %s", OX7B4DF353['nonce'])
        OX7B4DF347.debug("received public key: %s", OX7B4DF357)
        OX7B4DF347.debug("received ima_measurement_list    %s", (OX7B4DF359 is not None))
        OX7B4DF347.debug("received ima_measurement_list_entry: %d", OX7B4DF360)
        OX7B4DF347.debug("received boottime: %s", OX7B4DF362)
        OX7B4DF347.debug("received boot log    %s", (OX7B4DF361 is not None))
    except Exception as OX7B4DF363:
        OX7B4DF356.add_event("invalid_data", {"message": "parsing agents get quote respone failed", "data": OX7B4DF363}, False)
        return OX7B4DF356

    if not isinstance(OX7B4DF360, int):
        raise Exception("ima_measurement_list_entry parameter must be an integer")

    if not isinstance(OX7B4DF362, int):
        raise Exception("boottime parameter must be an integer")

    if OX7B4DF357 is None:
        if OX7B4DF353.get('public_key', "") == "" or OX7B4DF353.get('b64_encrypted_V', "") == "":
            OX7B4DF347.error("agent did not provide public key and no key or encrypted_v was cached at CV")
            OX7B4DF356.add_event("no_pubkey", "agent did not provide public key and no key or encrypted_v was cached at CV", False)
            return OX7B4DF356
        OX7B4DF353['provide_V'] = False
        OX7B4DF357 = OX7B4DF353['public_key']

    OX7B4DF364 = OX7B4DF354.get('hash_alg')
    OX7B4DF365 = OX7B4DF354.get('enc_alg')
    OX7B4DF366 = OX7B4DF354.get('sign_alg')

    OX7B4DF353['hash_alg'] = OX7B4DF364
    OX7B4DF353['enc_alg'] = OX7B4DF365
    OX7B4DF353['sign_alg'] = OX7B4DF366

    if not OX7B4DF344.is_accepted(OX7B4DF364, OX7B4DF353['accept_tpm_hash_algs'])\
            or not OX7B4DF344.Hash.is_recognized(OX7B4DF364):
        OX7B4DF347.error(f"TPM Quote is using an unaccepted hash algorithm: {OX7B4DF364}")
        OX7B4DF356.add_event("invalid_hash_alg",
                          {"message": f"TPM Quote is using an unaccepted hash algorithm: {OX7B4DF364}", "data": OX7B4DF364},
                          False)
        return OX7B4DF356

    if not OX7B4DF344.is_accepted(OX7B4DF365, OX7B4DF353['accept_tpm_encryption_algs']):
        OX7B4DF347.error(f"TPM Quote is using an unaccepted encryption algorithm: {OX7B4DF365}")
        OX7B4DF356.add_event("invalid_enc_alg",
                          {"message": f"TPM Quote is using an unaccepted encryption algorithm: {OX7B4DF365}", "data": OX7B4DF365},
                          False)
        return OX7B4DF356

    if not OX7B4DF344.is_accepted(OX7B4DF366, OX7B4DF353['accept_tpm_signing_algs']):
        OX7B4DF347.error(f"TPM Quote is using an unaccepted signing algorithm: {OX7B4DF366}")
        OX7B4DF356.add_event("invalid_sign_alg",
                          {"message": f"TPM Quote is using an unaccepted signing algorithm: {OX7B4DF366}", "data": {OX7B4DF366}},
                          False)
        return OX7B4DF356

    if OX7B4DF360 == 0:
        OX7B4DF355.reset_ima_attestation()
    elif OX7B4DF360 != OX7B4DF355.get_next_ima_ml_entry():
        OX7B4DF347.error("Agent did not respond with requested next IMA measurement list entry "
                     f"{OX7B4DF355.get_next_ima_ml_entry()} but started at {OX7B4DF360}")
        OX7B4DF356.add_event("invalid_ima_entry_nb",
                          {"message": "Agent did not respond with requested next IMA measurement list entry",
                           "got": OX7B4DF360, "expected": OX7B4DF355.get_next_ima_ml_entry()},
                          False)
    elif not OX7B4DF355.is_expected_boottime(OX7B4DF362):
        OX7B4DF355.reset_ima_attestation()
        return OX7B4DF356

    OX7B4DF355.set_boottime(OX7B4DF362)

    OX7B4DF367 = OX7B4DF355.get_ima_keyrings()
    OX7B4DF368 = OX7B4DF346.ImaKeyring.from_string(OX7B4DF353['ima_sign_verification_keys'])
    OX7B4DF367.set_tenant_keyring(OX7B4DF368)

    OX7B4DF369 = OX7B4DF350().check_quote(
        OX7B4DF355,
        OX7B4DF353['nonce'],
        OX7B4DF357,
        OX7B4DF358,
        OX7B4DF353['ak_tpm'],
        OX7B4DF353['tpm_policy'],
        OX7B4DF359,
        OX7B4DF353['allowlist'],
        OX7B4DF344.Hash(OX7B4DF364),
        OX7B4DF367,
        OX7B4DF361,
        OX7B4DF353['mb_refstate'],
        compressed=(OX7B4DF353['supported_version'] == "1.0"))
    OX7B4DF356.merge(OX7B4DF369)

    if not OX7B4DF356:
        OX7B4DF353['first_verified'] = True

        if OX7B4DF357 != OX7B4DF353.get('public_key', ""):
            OX7B4DF353['public_key'] = OX7B4DF357
            OX7B4DF353['b64_encrypted_V'] = ""
            OX7B4DF353['provide_V'] = True

    return OX7B4DF356

def OX7B4DF370(OX7B4DF371):
    if config.INSECURE_DEBUG:
        OX7B4DF347.debug("b64_V (non encrypted): %s", OX7B4DF371['v'])

    if OX7B4DF371.get('b64_encrypted_V', "") != "":
        OX7B4DF372 = OX7B4DF371['b64_encrypted_V']
        OX7B4DF347.debug("Re-using cached encrypted V")
    else:
        OX7B4DF372 = base64.b64encode(crypto.rsa_encrypt(
            crypto.rsa_import_pubkey(OX7B4DF371['public_key']), base64.b64decode(OX7B4DF371['v'])))
        OX7B4DF371['b64_encrypted_V'] = OX7B4DF372

    OX7B4DF373 = {
        'encrypted_key': OX7B4DF372
    }
    return OX7B4DF373

def OX7B4DF374(OX7B4DF375):
    OX7B4DF376 = OX7B4DF351().get_by_agent_id(OX7B4DF375['agent_id'])
    OX7B4DF375['nonce'] = OX7B4DF343.random_password(20)

    OX7B4DF377 = ast.literal_eval(OX7B4DF375['tpm_policy'])
    OX7B4DF378 = ast.literal_eval(OX7B4DF375['vtpm_policy'])

    OX7B4DF379 = {
        'nonce': OX7B4DF375['nonce'],
        'mask': OX7B4DF377['mask'],
        'vmask': OX7B4DF378['mask'],
        'ima_ml_entry': OX7B4DF376.get_next_ima_ml_entry(),
    }
    return OX7B4DF379

def OX7B4DF380(OX7B4DF381):
    OX7B4DF382 = json.loads(OX7B4DF381.allowlist)
    if isinstance(OX7B4DF382, dict) and 'allowlist' in OX7B4DF382:
        OX7B4DF383 = len(OX7B4DF382['allowlist'])
    else:
        OX7B4DF383 = 0

    try :
        OX7B4DF384 = json.loads(OX7B4DF381.mb_refstate)
    except Exception as OX7B4DF385:
        OX7B4DF347.warning('Non-fatal problem ocurred while attempting to evaluate agent attribute "mb_refstate" (%s). Will just consider the value of this attribute to be "None"', OX7B4DF385.args)
        OX7B4DF384 = None
        OX7B4DF347.debug('The contents of the agent attribute "mb_refstate" are %s', OX7B4DF381.mb_refstate)

    if isinstance(OX7B4DF384, dict) and 'mb_refstate' in OX7B4DF384:
        OX7B4DF386 = len(OX7B4DF384['mb_refstate'])
    else:
        OX7B4DF386 = 0
    OX7B4DF387 = {'operational_state': OX7B4DF381.operational_state,
                'v': OX7B4DF381.v,
                'ip': OX7B4DF381.ip,
                'port': OX7B4DF381.port,
                'tpm_policy': OX7B4DF381.tpm_policy,
                'vtpm_policy': OX7B4DF381.vtpm_policy,
                'meta_data': OX7B4DF381.meta_data,
                'allowlist_len': OX7B4DF383,
                'mb_refstate_len': OX7B4DF386,
                'accept_tpm_hash_algs': OX7B4DF381.accept_tpm_hash_algs,
                'accept_tpm_encryption_algs': OX7B4DF381.accept_tpm_encryption_algs,
                'accept_tpm_signing_algs': OX7B4DF381.accept_tpm_signing_algs,
                'hash_alg': OX7B4DF381.hash_alg,
                'enc_alg': OX7B4DF381.enc_alg,
                'sign_alg': OX7B4DF381.sign_alg,
                'verifier_id' : OX7B4DF381.verifier_id,
                'verifier_ip' : OX7B4DF381.verifier_ip,
                'verifier_port' : OX7B4DF381.verifier_port,
                'severity_level': OX7B4DF381.severity_level,
                'last_event_id': OX7B4DF381.last_event_id
                }
    return OX7B4DF387

def OX7B4DF388(OX7B4DF389, OX7B4DF390='revocation', OX7B4DF391=None):
    OX7B4DF392 = config.getboolean('cloud_verifier', 'revocation_notifier')
    OX7B4DF393 = config.getboolean('cloud_verifier', 'revocation_notifier_webhook', fallback=False)
    if not (OX7B4DF392 or OX7B4DF393):
        return

    OX7B4DF394 = {'type': OX7B4DF390,
                  'ip': OX7B4DF389['ip'],
                  'agent_id': OX7B4DF389['agent_id'],
                  'port': OX7B4DF389['port'],
                  'tpm_policy': OX7B4DF389['tpm_policy'],
                  'vtpm_policy': OX7B4DF389['vtpm_policy'],
                  'meta_data': OX7B4DF389['meta_data'],
                  'event_time': time.asctime()}
    if OX7B4DF391:
        OX7B4DF394['event_id'] = OX7B4DF391.event_id
        OX7B4DF394['severity_label'] = OX7B4DF391.severity_label.name
        OX7B4DF394['context'] = OX7B4DF391.context

    OX7B4DF395 = {'msg': json.dumps(OX7B4DF394).encode('utf-8')}

    if OX7B4DF389['revocation_key'] != "":
        OX7B4DF396 = crypto.rsa_import_privkey(OX7B4DF389['revocation_key'])
        OX7B4DF395['signature'] = crypto.rsa_sign(OX7B4DF396, OX7B4DF395['msg'])

    else:
        OX7B4DF395['signature'] = "none"
    if OX7B4DF392:
        revocation_notifier.notify(OX7B4DF395)
    if OX7B4DF393:
        revocation_notifier.notify_webhook(OX7B4DF395)

def OX7B4DF397(OX7B4DF398):
    if OX7B4DF398 is None:
        return False, None

    OX7B4DF399 = json.loads(OX7B4DF398['allowlist'])

    OX7B4DF400, _, OX7B4DF401 = OX7B4DF345.valid_exclude_list(OX7B4DF399.get('exclude'))
    if not OX7B4DF400:
        OX7B4DF401 += " Exclude list regex is misformatted. Please correct the issue and try again."

    return OX7B4DF400, OX7B4DF401