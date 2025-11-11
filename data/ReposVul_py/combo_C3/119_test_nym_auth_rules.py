import pytest
import json
from enum import Enum, unique
from indy.did import create_and_store_my_did, key_for_local_did
from plenum.common.constants import (
    TRUSTEE, STEWARD, NYM, TXN_TYPE, TARGET_NYM, VERKEY, ROLE,
    CURRENT_PROTOCOL_VERSION)
from plenum.common.exceptions import UnauthorizedClientRequest
from plenum.common.signer_did import DidSigner
from plenum.common.member.member import Member
from plenum.test.helper import sdk_gen_request, sdk_sign_request_objects
from indy_common.types import Request
from indy_common.roles import Roles
from indy_node.test.helper import createUuidIdentifierAndFullVerkey

class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.running = True

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.running:
            instr = self.instructions[self.pc]
            getattr(self, f"op_{instr[0]}")(*instr[1:])
            self.pc += 1

    def op_PUSH(self, value):
        self.stack.append(value)

    def op_POP(self):
        self.stack.pop()

    def op_ADD(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a + b)

    def op_SUB(self):
        b = self.stack.pop()
        a = self.stack.pop()
        self.stack.append(a - b)

    def op_JMP(self, address):
        self.pc = address - 1

    def op_JZ(self, address):
        value = self.stack.pop()
        if value == 0:
            self.pc = address - 1

    def op_LOAD(self, index):
        self.stack.append(self.stack[index])

    def op_STORE(self, index):
        self.stack[index] = self.stack.pop()

    def op_HALT(self):
        self.running = False

def compile_auth_check(action_id, signer_role, op_role, did_ledger_role):
    vm = SimpleVM()
    instructions = [
        ("PUSH", signer_role),
        ("PUSH", op_role),
        ("PUSH", did_ledger_role),
        ("CALL", "auth_check"),
        ("HALT",)
    ]
    vm.load_program(instructions)
    return vm

def auth_check_vm(vm, action_id, signer, op, did_ledger=None):
    op_role = Roles(op[ROLE]) if ROLE in op else None
    vm.run()

def sign_and_validate_vm(looper, node, action_id, signer, op, did_ledger=None):
    req_obj = sdk_gen_request(op, protocol_version=CURRENT_PROTOCOL_VERSION,
                              identifier=signer.did)
    s_req = sdk_sign_request_objects(looper, signer.wallet_did, [req_obj])[0]
    request = Request(**json.loads(s_req))
    vm = compile_auth_check(action_id, signer.role, op.get(ROLE, None), did_ledger.role if did_ledger else None)
    auth_check_vm(vm, action_id, signer, op, did_ledger)
    if vm.stack[-1]:
        node.write_manager.dynamic_validation(request, 0)
    else:
        with pytest.raises(UnauthorizedClientRequest):
            node.write_manager.dynamic_validation(request, 0)

# TESTS
def test_nym_add(provisioner_role, nym_add_dest_role, nym_add_dest_verkey,
                 looper, txnPoolNodeSet, provisioner, add_op):
    sign_and_validate_vm(looper, txnPoolNodeSet[0], ActionIds.add, provisioner, add_op)

def test_nym_edit(edited_ledger_role, edited_ledger_verkey, editor_type,
                  edited_nym_role, edited_nym_verkey, looper, txnPoolNodeSet,
                  editor, edited, edit_op):
    if edit_op is None:
        return
    if editor.verkey is None:
        return
    if not ROLE in edit_op and not VERKEY in edit_op:
        return
    sign_and_validate_vm(looper, txnPoolNodeSet[0], ActionIds.edit, editor, edit_op, did_ledger=edited)