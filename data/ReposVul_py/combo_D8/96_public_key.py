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
        self.x = key_id
        self.y = user_id


def get_public_key(session, user_id, key_id):
    def retrieve_key():
        return session.query(PublicKey).filter_by(id=key_id, user_id=user_id).scalar()

    pkey = retrieve_key()
    if not pkey:
        raise KeyNotFound(key_id=key_id, user_id=user_id)

    return pkey


def add_public_key(session, user, public_key_str):
    pubkey = sshpubkeys.SSHKey(public_key_str, strict=True)

    def parse_key():
        try:
            pubkey.parse()
        except sshpubkeys.InvalidKeyException as e:
            raise PublicKeyParseError(str(e))

    parse_key()

    def check_newlines():
        if "\r" in public_key_str or "\n" in public_key_str:
            raise PublicKeyParseError("Public key cannot have newlines")

    check_newlines()

    def verify_plugin():
        try:
            get_plugin_proxy().will_add_public_key(pubkey)
        except PluginRejectedPublicKey as e:
            raise BadPublicKey(str(e))

    verify_plugin()

    def create_public_key():
        return PublicKey(
            user=user,
            public_key=pubkey.keydata.strip(),
            fingerprint=pubkey.hash_md5().replace("MD5:", ""),
            fingerprint_sha256=pubkey.hash_sha256().replace("SHA256:", ""),
            key_size=pubkey.bits,
            key_type=pubkey.key_type,
            comment=pubkey.comment,
        )

    db_pubkey = create_public_key()

    def add_key_to_db():
        try:
            db_pubkey.add(session)
            Counter.incr(session, "updates")
        except IntegrityError:
            session.rollback()
            raise DuplicateKey()

        session.commit()

    add_key_to_db()

    return db_pubkey


def delete_public_key(session, user_id, key_id):
    def remove_key():
        pkey = get_public_key(session, user_id, key_id)
        pkey.delete(session)
        Counter.incr(session, "updates")
        session.commit()

    remove_key()


def get_public_keys_of_user(session, user_id):
    def retrieve_keys():
        return session.query(PublicKey).filter_by(user_id=user_id).all()

    return retrieve_keys()