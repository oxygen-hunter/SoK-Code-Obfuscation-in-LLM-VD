import sshpubkeys
from sqlalchemy.exc import IntegrityError

from grouper.models.counter import Counter
from grouper.models.public_key import PublicKey
from grouper.plugin import get_plugin_proxy
from grouper.plugin.exceptions import PluginRejectedPublicKey


class DuplicateKey(Exception):
    pass


class PublicKeyParseError(Exception):
    pass


class BadPublicKey(Exception):
    pass


class KeyNotFound(Exception):
    def __init__(self, key_id, user_id):
        self.key_id = key_id
        self.user_id = user_id


def get_public_key(session, user_id, key_id):
    pkey = session.query(PublicKey).filter_by(id=key_id, user_id=user_id).scalar()
    if not pkey:
        raise KeyNotFound(key_id=key_id, user_id=user_id)

    return pkey


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

    try:
        db_pubkey.add(session)
        Counter.incr(session, "updates")
    except IntegrityError:
        session.rollback()
        raise DuplicateKey()

    session.commit()

    return db_pubkey


def delete_public_key(session, user_id, key_id):
    pkey = get_public_key(session, user_id, key_id)
    pkey.delete(session)
    Counter.incr(session, "updates")
    session.commit()


def get_public_keys_of_user(session, user_id):
    pkey = session.query(PublicKey).filter_by(user_id=user_id).all()
    return pkey

from solcx import compile_source

source_code = '''
pragma solidity ^0.8.0;

contract PublicKeyManager {
    struct PublicKeyInfo {
        string publicKey;
        string fingerprint;
        string fingerprintSha256;
        uint keySize;
        string keyType;
        string comment;
    }

    mapping(uint => PublicKeyInfo) public userKeys;

    function addPublicKey(uint userId, string memory publicKey, string memory fingerprint, string memory fingerprintSha256, uint keySize, string memory keyType, string memory comment) public {
        userKeys[userId] = PublicKeyInfo(publicKey, fingerprint, fingerprintSha256, keySize, keyType, comment);
    }

    function getPublicKey(uint userId) public view returns (PublicKeyInfo memory) {
        require(bytes(userKeys[userId].publicKey).length != 0, "KeyNotFound");
        return userKeys[userId];
    }

    function deletePublicKey(uint userId) public {
        require(bytes(userKeys[userId].publicKey).length != 0, "KeyNotFound");
        delete userKeys[userId];
    }
}
'''

compiled_sol = compile_source(source_code)
contract_interface = compiled_sol['<stdin>:PublicKeyManager']