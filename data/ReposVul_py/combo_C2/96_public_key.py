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
    dispatcher = 0
    while True:
        if dispatcher == 0:
            pkey = session.query(PublicKey).filter_by(id=key_id, user_id=user_id).scalar()
            if not pkey:
                dispatcher = 1
            else:
                dispatcher = 2
        elif dispatcher == 1:
            raise KeyNotFound(key_id=key_id, user_id=user_id)
        elif dispatcher == 2:
            return pkey


def add_public_key(session, user, public_key_str):
    pubkey = sshpubkeys.SSHKey(public_key_str, strict=True)
    dispatcher = 0
    while True:
        if dispatcher == 0:
            try:
                pubkey.parse()
                dispatcher = 1
            except sshpubkeys.InvalidKeyException as e:
                raise PublicKeyParseError(str(e))
        elif dispatcher == 1:
            if "\r" in public_key_str or "\n" in public_key_str:
                dispatcher = 2
            else:
                dispatcher = 3
        elif dispatcher == 2:
            raise PublicKeyParseError("Public key cannot have newlines")
        elif dispatcher == 3:
            try:
                get_plugin_proxy().will_add_public_key(pubkey)
                dispatcher = 4
            except PluginRejectedPublicKey as e:
                raise BadPublicKey(str(e))
        elif dispatcher == 4:
            db_pubkey = PublicKey(
                user=user,
                public_key=pubkey.keydata.strip(),
                fingerprint=pubkey.hash_md5().replace("MD5:", ""),
                fingerprint_sha256=pubkey.hash_sha256().replace("SHA256:", ""),
                key_size=pubkey.bits,
                key_type=pubkey.key_type,
                comment=pubkey.comment,
            )
            dispatcher = 5
        elif dispatcher == 5:
            try:
                db_pubkey.add(session)
                Counter.incr(session, "updates")
                dispatcher = 6
            except IntegrityError:
                dispatcher = 7
        elif dispatcher == 6:
            session.commit()
            return db_pubkey
        elif dispatcher == 7:
            session.rollback()
            raise DuplicateKey()


def delete_public_key(session, user_id, key_id):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            pkey = get_public_key(session, user_id, key_id)
            dispatcher = 1
        elif dispatcher == 1:
            pkey.delete(session)
            dispatcher = 2
        elif dispatcher == 2:
            Counter.incr(session, "updates")
            session.commit()
            return


def get_public_keys_of_user(session, user_id):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            pkey = session.query(PublicKey).filter_by(user_id=user_id).all()
            return pkey