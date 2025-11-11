import sshpubkeys
from sqlalchemy.exc import IntegrityError
from grouper.models.counter import Counter
from grouper.models.public_key import PublicKey
from grouper.plugin import get_plugin_proxy
from grouper.plugin.exceptions import PluginRejectedPublicKey

class DuplicateKey(Exception): pass
class PublicKeyParseError(Exception): pass
class BadPublicKey(Exception): pass
class KeyNotFound(Exception):
    def __init__(self, key_id, user_id):
        self.key_id = key_id
        self.user_id = user_id

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            inst = self.instructions[self.pc]
            if inst[0] == "PUSH":
                self.stack.append(inst[1])
            elif inst[0] == "POP":
                self.stack.pop()
            elif inst[0] == "ADD":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(a + b)
            elif inst[0] == "SUB":
                a = self.stack.pop()
                b = self.stack.pop()
                self.stack.append(b - a)
            elif inst[0] == "JMP":
                self.pc = inst[1] - 1
            elif inst[0] == "JZ":
                a = self.stack.pop()
                if a == 0:
                    self.pc = inst[1] - 1
            elif inst[0] == "LOAD":
                self.stack.append(inst[1])
            elif inst[0] == "STORE":
                idx = inst[1]
                self.stack[idx] = self.stack.pop()
            self.pc += 1

def vm_execute(instructions):
    vm = VM()
    vm.load(instructions)
    vm.run()
    return vm.stack[-1]

def get_public_key(session, user_id, key_id):
    instructions = [
        ("LOAD", session),
        ("LOAD", user_id),
        ("LOAD", key_id),
        ("PUSH", 0),
        ("JMP", 9),
        ("POP",),
        ("PUSH", 1),
        ("JZ", 5),
        ("LOAD", KeyNotFound(key_id, user_id)),
        ("STORE", 0),
    ]
    return vm_execute(instructions)

def add_public_key(session, user, public_key_str):
    pubkey = sshpubkeys.SSHKey(public_key_str, strict=True)
    try:
        pubkey.parse()
    except sshpubkeys.InvalidKeyException as e:
        raise PublicKeyParseError(str(e))

    if "\r" in public_key_str or "\n" in public_key_str:
        raise PublicKeyParseError("Public key cannot have newlines")

    try:
        get_plugin_proxy().will_add_public_key(pubkey)
    except PluginRejectedPublicKey as e:
        raise BadPublicKey(str(e))

    db_pubkey = PublicKey(
        user=user,
        public_key=pubkey.keydata.strip(),
        fingerprint=pubkey.hash_md5().replace("MD5:", ""),
        fingerprint_sha256=pubkey.hash_sha256().replace("SHA256:", ""),
        key_size=pubkey.bits,
        key_type=pubkey.key_type,
        comment=pubkey.comment,
    )

    instructions = [
        ("LOAD", db_pubkey),
        ("LOAD", session),
        ("PUSH", 0),
        ("JMP", 9),
        ("POP",),
        ("PUSH", 1),
        ("JZ", 5),
        ("LOAD", DuplicateKey()),
        ("STORE", 0),
    ]
    return vm_execute(instructions)

def delete_public_key(session, user_id, key_id):
    instructions = [
        ("LOAD", session),
        ("LOAD", user_id),
        ("LOAD", key_id),
        ("PUSH", 0),
        ("JMP", 9),
        ("POP",),
        ("PUSH", 1),
        ("JZ", 5),
        ("LOAD", KeyNotFound(key_id, user_id)),
        ("STORE", 0),
    ]
    return vm_execute(instructions)

def get_public_keys_of_user(session, user_id):
    instructions = [
        ("LOAD", session),
        ("LOAD", user_id),
        ("PUSH", 0),
        ("JMP", 6),
        ("POP",),
        ("PUSH", 1),
        ("STORE", 0),
    ]
    return vm_execute(instructions)