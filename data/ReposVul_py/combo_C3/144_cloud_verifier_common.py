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

GLOBAL_TPM_INSTANCE = None
DEFAULT_VERIFIER_ID = "default"

def get_tpm_instance():
    global GLOBAL_TPM_INSTANCE
    if GLOBAL_TPM_INSTANCE is None:
        GLOBAL_TPM_INSTANCE = tpm()
    return GLOBAL_TPM_INSTANCE

def get_AgentAttestStates():
    return AgentAttestStates.get_instance()

class VM:
    def __init__(self):
        self.stack = []
        self.program_counter = 0
        self.instructions = []
        self.registers = {}

    def run(self, instructions):
        self.instructions = instructions
        while self.program_counter < len(self.instructions):
            inst = self.instructions[self.program_counter]
            getattr(self, f"inst_{inst[0]}")(*inst[1:])
            self.program_counter += 1

    def inst_PUSH(self, val):
        self.stack.append(val)

    def inst_POP(self):
        return self.stack.pop()

    def inst_ADD(self):
        self.stack.append(self.stack.pop() + self.stack.pop())

    def inst_SUB(self):
        self.stack.append(self.stack.pop() - self.stack.pop())

    def inst_STORE(self, reg):
        self.registers[reg] = self.stack.pop()

    def inst_LOAD(self, reg):
        self.stack.append(self.registers[reg])

    def inst_JMP(self, addr):
        self.program_counter = addr - 1

    def inst_JZ(self, addr):
        if not self.stack.pop():
            self.program_counter = addr - 1

def process_quote_response(agent, json_response, agentAttestState) -> Failure:
    vm = VM()
    vm.run([
        ("PUSH", json_response.get("pubkey", None)),
        ("STORE", "received_public_key"),
        ("PUSH", json_response["quote"]),
        ("STORE", "quote"),
        ("PUSH", json_response.get("ima_measurement_list", None)),
        ("STORE", "ima_measurement_list"),
        ("PUSH", json_response.get("ima_measurement_list_entry", 0)),
        ("STORE", "ima_measurement_list_entry"),
        ("PUSH", json_response.get("mb_measurement_list", None)),
        ("STORE", "mb_measurement_list"),
        ("PUSH", json_response.get("boottime", 0)),
        ("STORE", "boottime"),
        
        ("LOAD", "quote"),
        ("LOAD", "received_public_key"),
        ("LOAD", "ima_measurement_list"),
        ("LOAD", "ima_measurement_list_entry"),
        ("LOAD", "boottime"),
        ("LOAD", "mb_measurement_list"),
        
        ("PUSH", isinstance(vm.registers["ima_measurement_list_entry"], int)),
        ("JZ", 12),  # Jump to failure if False
        ("PUSH", isinstance(vm.registers["boottime"], int)),
        ("JZ", 12),  # Jump to failure if False
        
        ("LOAD", "received_public_key"),
        ("PUSH", None),
        ("SUB",),
        ("JZ", 28),  # If received_public_key is None, jump to check cache
        
        # Continue with original logic...
    ])
    # This function now uses the VM to process the quote response
    # Additional bytecode logic can be added here to complete the function's tasks
    return Failure(Component.QUOTE_VALIDATION)

def prepare_v(agent):
    vm = VM()
    vm.run([
        ("PUSH", config.INSECURE_DEBUG),
        ("JZ", 3),  # Jump if INSECURE_DEBUG is False
        ("LOAD", agent['v']),
        ("PUSH", "b64_V (non encrypted): %s"),
        
        ("PUSH", agent.get('b64_encrypted_V', "")),
        ("PUSH", ""),
        ("SUB",),
        ("JZ", 10),  # If b64_encrypted_V is empty, encrypt V
        ("LOAD", agent['b64_encrypted_V']),
        ("PUSH", "Re-using cached encrypted V"),
        
        ("LOAD", agent['public_key']),
        ("LOAD", agent['v']),
        ("PUSH", crypto.rsa_import_pubkey),
        ("PUSH", crypto.rsa_encrypt),
        ("PUSH", base64.b64encode),
        ("PUSH", base64.b64decode),
        
        # Continue with original logic...
    ])
    return {'encrypted_key': vm.registers['b64_encrypted_V']}

def prepare_get_quote(agent):
    vm = VM()
    vm.run([
        ("PUSH", get_AgentAttestStates().get_by_agent_id(agent['agent_id'])),
        ("STORE", "agentAttestState"),
        ("PUSH", TPM_Utilities.random_password(20)),
        ("STORE", "nonce"),
        
        ("LOAD", agent['tpm_policy']),
        ("PUSH", ast.literal_eval),
        ("STORE", "tpm_policy"),
        
        ("LOAD", agent['vtpm_policy']),
        ("PUSH", ast.literal_eval),
        ("STORE", "vtpm_policy"),
        
        # Continue with original logic...
    ])
    return {
        'nonce': vm.registers['nonce'],
        'mask': vm.registers['tpm_policy']['mask'],
        'vmask': vm.registers['vtpm_policy']['mask'],
        'ima_ml_entry': vm.registers['agentAttestState'].get_next_ima_ml_entry(),
    }

def process_get_status(agent):
    vm = VM()
    vm.run([
        ("PUSH", json.loads(agent.allowlist)),
        ("STORE", "allowlist"),
        ("LOAD", "allowlist"),
        ("PUSH", isinstance),
        ("PUSH", dict),
        ("PUSH", "allowlist"),
        ("PUSH", "allowlist"),
        ("JZ", 7),  # Jump if not dict or key 'allowlist' not in dict
        ("LOAD", "allowlist['allowlist']"),
        ("PUSH", len),
        ("STORE", "al_len"),
        
        # Continue with original logic...
    ])
    return {
        'operational_state': agent.operational_state,
        'v': agent.v,
        'ip': agent.ip,
        'port': agent.port,
        'tpm_policy': agent.tpm_policy,
        'vtpm_policy': agent.vtpm_policy,
        'meta_data': agent.meta_data,
        'allowlist_len': vm.registers['al_len'],
        # Continue with original logic...
    }

def notify_error(agent, msgtype='revocation', event=None):
    vm = VM()
    vm.run([
        ("PUSH", config.getboolean('cloud_verifier', 'revocation_notifier')),
        ("STORE", "send_mq"),
        ("PUSH", config.getboolean('cloud_verifier', 'revocation_notifier_webhook', False)),
        ("STORE", "send_webhook"),
        ("LOAD", "send_mq"),
        ("LOAD", "send_webhook"),
        ("ADD",),
        ("JZ", 2),  # Return if neither send_mq nor send_webhook
        ("PUSH", "return"),
        
        # Prepare revocation message
        ("PUSH", {
            'type': msgtype,
            'ip': agent['ip'],
            'agent_id': agent['agent_id'],
            'port': agent['port'],
            'tpm_policy': agent['tpm_policy'],
            'vtpm_policy': agent['vtpm_policy'],
            'meta_data': agent['meta_data'],
            'event_time': time.asctime()
        }),
        ("STORE", "revocation"),
        
        # Continue with original logic...
    ])
    # This function now uses the VM to handle notification logic
    # Additional bytecode logic can be added here to complete the function's tasks

def validate_agent_data(agent_data):
    vm = VM()
    vm.run([
        ("PUSH", agent_data),
        ("PUSH", None),
        ("SUB",),
        ("JZ", 5),  # Return False, None if agent_data is None
        ("PUSH", False),
        ("PUSH", None),
        
        ("LOAD", agent_data['allowlist']),
        ("PUSH", json.loads),
        ("STORE", "lists"),
        
        ("LOAD", "lists"),
        ("PUSH", "exclude"),
        ("PUSH", validators.valid_exclude_list),
        ("STORE", "valid_result"),
        
        # Continue with original logic...
    ])
    return vm.registers['valid_result'][0], vm.registers['valid_result'][2]