import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
from psycopg2.sql import SQL, Identifier

# VM Definitions
PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET = range(10)

class SimpleVM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.memory = {}
        self.instructions = []
        self.functions = {}

    def load_program(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            opcode, *args = self.instructions[self.pc]
            self.pc += 1
            if opcode == PUSH:
                self.stack.append(args[0])
            elif opcode == POP:
                self.stack.pop()
            elif opcode == ADD:
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a + b)
            elif opcode == SUB:
                b = self.stack.pop()
                a = self.stack.pop()
                self.stack.append(a - b)
            elif opcode == JMP:
                self.pc = args[0]
            elif opcode == JZ:
                value = self.stack.pop()
                if value == 0:
                    self.pc = args[0]
            elif opcode == LOAD:
                self.stack.append(self.memory.get(args[0], 0))
            elif opcode == STORE:
                self.memory[args[0]] = self.stack.pop()
            elif opcode == CALL:
                self.functions[args[0]]()
            elif opcode == RET:
                break

    def register_function(self, name, func):
        self.functions[name] = func

def _create_pg_connection(config):
    if 'server' in config:
        username = '@'.join([config['username'], config['server']])
    else:
        username = config['username']
    return psycopg2.connect(
        database=config['database'],
        user=username,
        host=config['host'],
        port=config['port'],
        password=config['password'],
    )

def vm_check_db_or_user_exists(vm):
    db_name = vm.stack.pop()
    db_user = vm.stack.pop()
    config = vm.stack.pop()
    with _create_pg_connection(config) as con:
        with con.cursor() as cur:
            cur.execute("SELECT 1 FROM pg_database WHERE datname=%s;", (db_name, ))
            db_exists = cur.fetchone() is not None
            cur.execute("SELECT 1 FROM pg_roles WHERE rolname=%s;", (db_user, ))
            user = cur.fetchone()
            user_exists = user is not None
            vm.stack.append(db_exists or user_exists)

def vm_create_postgres_db(vm):
    connection_dict = vm.stack.pop()
    config = vm.stack.pop()
    if check_db_or_user_exists(connection_dict["db_name"], connection_dict["db_username"], config):
        raise ValueError("db or user already exists")
    with _create_pg_connection(config) as con:
        con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with con.cursor() as cur:
            try:
                cur.execute(SQL("CREATE USER {} WITH PASSWORD %s;").format(
                    Identifier(connection_dict['db_username']),
                ), (
                    connection_dict['db_pwd'],
                ))
                cur.execute(SQL("GRANT {} TO {};").format(
                    Identifier(connection_dict['db_username']),
                    Identifier(get_normalized_username(config['username'])),
                ))
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])
            try:
                cur.execute(SQL("CREATE DATABASE {} OWNER {};").format(
                    Identifier(connection_dict['db_name']),
                    Identifier(connection_dict['db_username']),
                ))
            except psycopg2.ProgrammingError as e:
                cur.execute(SQL("DROP ROLE {};").format(
                    Identifier(connection_dict['db_username']),
                ))
                raise ValueError(e.args[0])

def vm_delete_database(vm):
    db_name = vm.stack.pop()
    config = vm.stack.pop()
    with _create_pg_connection(config) as con:
        con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with con.cursor() as cur:
            try:
                cur.execute('''DROP DATABASE "{}";'''.format(db_name))
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])

def vm_delete_user(vm):
    username = vm.stack.pop()
    config = vm.stack.pop()
    with _create_pg_connection(config) as con:
        con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with con.cursor() as cur:
            try:
                cur.execute('''DROP USER "{}";'''.format(get_normalized_username(username)))
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])

def get_normalized_username(username):
    return username.split('@')[0]

# Create and run VM
vm = SimpleVM()

vm.register_function('check_db_or_user_exists', vm_check_db_or_user_exists)
vm.register_function('create_postgres_db', vm_create_postgres_db)
vm.register_function('delete_database', vm_delete_database)
vm.register_function('delete_user', vm_delete_user)

# Example usage
vm.load_program([
    # This is a placeholder for the actual sequence of instructions to run the functions
    # The actual opcodes and arguments would be generated based on the program requirements
    (CALL, 'check_db_or_user_exists'),
    (CALL, 'create_postgres_db'),
    (CALL, 'delete_database'),
    (CALL, 'delete_user'),
])

vm.run()