from __future__ import annotations
import datetime
import hashlib
import logging
import secrets
import typing as t
from piccolo.columns import Boolean, Secret, Timestamp, Varchar
from piccolo.columns.column_types import Serial
from piccolo.columns.readable import Readable
from piccolo.table import Table
from piccolo.utils.sync import run_sync

logger = logging.getLogger(__name__)

class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []
        self.registers = {}

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            instruction = self.instructions[self.pc]
            self.execute(instruction)
            self.pc += 1

    def execute(self, instruction):
        op, *args = instruction
        if op == "PUSH":
            self.stack.append(args[0])
        elif op == "POP":
            self.stack.pop()
        elif op == "ADD":
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a + b)
        elif op == "SUB":
            b = self.stack.pop()
            a = self.stack.pop()
            self.stack.append(a - b)
        elif op == "JMP":
            self.pc = args[0] - 1
        elif op == "JZ":
            if self.stack.pop() == 0:
                self.pc = args[0] - 1
        elif op == "STORE":
            self.registers[args[0]] = self.stack.pop()
        elif op == "LOAD":
            self.stack.append(self.registers.get(args[0], 0))

class BaseUser(Table, tablename="piccolo_user"):
    id: Serial
    username = Varchar(length=100, unique=True)
    password = Secret(length=255)
    first_name = Varchar(null=True)
    last_name = Varchar(null=True)
    email = Varchar(length=255, unique=True)
    active = Boolean(default=False)
    admin = Boolean(default=False, help_text="An admin can log into the Piccolo admin GUI.")
    superuser = Boolean(default=False, help_text="If True, this user can manage other users's passwords in the Piccolo admin GUI.")
    last_login = Timestamp(null=True, default=None, required=False, help_text="When this user last logged in.")

    _min_password_length = 6
    _max_password_length = 128
    _pbkdf2_iteration_count = 600_000

    def __init__(self, **kwargs):
        password = kwargs.get("password", None)
        if password:
            vm = SimpleVM()
            vm.load_instructions([
                ("PUSH", "pbkdf2_sha256"),
                ("PUSH", password),
                ("LOAD", "0"),
                ("JZ", 7),
                ("STORE", "password"),
                ("JMP", 8),
                ("STORE", "password_hashed"),
                ("LOAD", "password"),
                ("STORE", "password")
            ])
            vm.run()
            if "password_hashed" in vm.registers:
                kwargs["password"] = self.__class__.hash_password(password)
        super().__init__(**kwargs)

    @classmethod
    def get_salt(cls):
        return secrets.token_hex(16)

    @classmethod
    def get_readable(cls) -> Readable:
        return Readable(template="%s", columns=[cls.username])

    @classmethod
    def _validate_password(cls, password: str):
        vm = SimpleVM()
        vm.load_instructions([
            ("PUSH", len(password)),
            ("PUSH", cls._min_password_length),
            ("SUB", ),
            ("JZ", 5),
            ("PUSH", len(password)),
            ("PUSH", cls._max_password_length),
            ("SUB", ),
            ("JZ", 8),
            ("PUSH", "pbkdf2_sha256"),
            ("PUSH", password),
            ("LOAD", "0"),
            ("JZ", 11),
            ("JMP", 12),
            ("STORE", "valid")
        ])
        vm.run()
        if "valid" not in vm.registers:
            raise ValueError("Invalid password criteria.")

    @classmethod
    def update_password_sync(cls, user: t.Union[str, int], password: str):
        return run_sync(cls.update_password(user, password))

    @classmethod
    async def update_password(cls, user: t.Union[str, int], password: str):
        vm = SimpleVM()
        vm.load_instructions([
            ("PUSH", isinstance(user, str)),
            ("JZ", 5),
            ("STORE", "clause"),
            ("JMP", 7),
            ("PUSH", isinstance(user, int)),
            ("JZ", 10),
            ("STORE", "clause"),
            ("JMP", 12),
            ("STORE", "error")
        ])
        vm.run()
        if "error" in vm.registers:
            raise ValueError("The `user` arg must be a user id, or a username.")
        
        if "clause" not in vm.registers:
            clause = cls.id == user
        else:
            clause = cls.username == user

        cls._validate_password(password=password)
        password = cls.hash_password(password)
        await cls.update({cls.password: password}).where(clause).run()

    @classmethod
    def hash_password(cls, password: str, salt: str = "", iterations: t.Optional[int] = None) -> str:
        vm = SimpleVM()
        vm.load_instructions([
            ("PUSH", len(password)),
            ("PUSH", cls._max_password_length),
            ("SUB", ),
            ("JZ", 5),
            ("STORE", "excessive")
        ])
        vm.run()
        if "excessive" in vm.registers:
            logger.warning("Excessively long password provided.")
            raise ValueError("The password is too long.")

        if not salt:
            salt = cls.get_salt()

        if iterations is None:
            iterations = cls._pbkdf2_iteration_count

        hashed = hashlib.pbkdf2_hmac(
            "sha256",
            bytes(password, encoding="utf-8"),
            bytes(salt, encoding="utf-8"),
            iterations,
        ).hex()
        return f"pbkdf2_sha256${iterations}${salt}${hashed}"

    def __setattr__(self, name: str, value: t.Any):
        if name == "password" and not value.startswith("pbkdf2_sha256"):
            value = self.__class__.hash_password(value)
        super().__setattr__(name, value)

    @classmethod
    def split_stored_password(cls, password: str) -> t.List[str]:
        elements = password.split("$")
        if len(elements) != 4:
            raise ValueError("Unable to split hashed password")
        return elements

    @classmethod
    def login_sync(cls, username: str, password: str) -> t.Optional[int]:
        return run_sync(cls.login(username, password))

    @classmethod
    async def login(cls, username: str, password: str) -> t.Optional[int]:
        vm = SimpleVM()
        vm.load_instructions([
            ("PUSH", len(username)),
            ("PUSH", cls.username.length),
            ("SUB", ),
            ("JZ", 5),
            ("STORE", "long_username"),
            ("PUSH", len(password)),
            ("PUSH", cls._max_password_length),
            ("SUB", ),
            ("JZ", 9),
            ("STORE", "long_password")
        ])
        vm.run()
        if "long_username" in vm.registers or "long_password" in vm.registers:
            logger.warning("Excessively long username or password provided.")
            return None

        response = (
            await cls.select(cls._meta.primary_key, cls.password)
            .where(cls.username == username)
            .first()
            .run()
        )
        if not response:
            cls.hash_password(password)
            return None

        stored_password = response["password"]
        algorithm, iterations_, salt, hashed = cls.split_stored_password(stored_password)
        iterations = int(iterations_)

        if cls.hash_password(password, salt, iterations) == stored_password:
            if iterations != cls._pbkdf2_iteration_count:
                await cls.update_password(username, password)

            await cls.update({cls.last_login: datetime.datetime.now()}).where(cls.username == username)
            return response["id"]
        else:
            return None

    @classmethod
    def create_user_sync(cls, username: str, password: str, **extra_params) -> BaseUser:
        return run_sync(cls.create_user(username=username, password=password, **extra_params))

    @classmethod
    async def create_user(cls, username: str, password: str, **extra_params) -> BaseUser:
        vm = SimpleVM()
        vm.load_instructions([
            ("PUSH", username),
            ("JZ", 3),
            ("STORE", "valid_user")
        ])
        vm.run()
        if "valid_user" not in vm.registers:
            raise ValueError("A username must be provided.")

        cls._validate_password(password=password)

        user = cls(username=username, password=password, **extra_params)
        await user.save()
        return user