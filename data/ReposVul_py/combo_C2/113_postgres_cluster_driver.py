import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
from psycopg2.sql import SQL, Identifier

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

def check_db_or_user_exists(db_name, db_user, config):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            conn = _create_pg_connection(config)
            dispatcher = 1
        elif dispatcher == 1:
            with conn as con:
                dispatcher = 2
        elif dispatcher == 2:
            with con.cursor() as cur:
                dispatcher = 3
        elif dispatcher == 3:
            cur.execute("SELECT 1 FROM pg_database WHERE datname=%s;", (db_name,))
            db_exists = cur.fetchone() is not None
            dispatcher = 4
        elif dispatcher == 4:
            cur.execute("SELECT 1 FROM pg_roles WHERE rolname=%s;", (db_user,))
            user = cur.fetchone()
            user_exists = user is not None
            return db_exists or user_exists

def create_postgres_db(connection_dict, config):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            if check_db_or_user_exists(connection_dict["db_name"], connection_dict["db_username"], config):
                raise ValueError("db or user already exists")
            dispatcher = 1
        elif dispatcher == 1:
            conn = _create_pg_connection(config)
            dispatcher = 2
        elif dispatcher == 2:
            with conn as con:
                dispatcher = 3
        elif dispatcher == 3:
            con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
            dispatcher = 4
        elif dispatcher == 4:
            with con.cursor() as cur:
                dispatcher = 5
        elif dispatcher == 5:
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
                dispatcher = 7
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])
        elif dispatcher == 7:
            try:
                cur.execute(SQL("CREATE DATABASE {} OWNER {};").format(
                    Identifier(connection_dict['db_name']),
                    Identifier(connection_dict['db_username']),
                ))
                return
            except psycopg2.ProgrammingError as e:
                cur.execute(SQL("DROP ROLE {};").format(
                    Identifier(connection_dict['db_username']),
                ))
                raise ValueError(e.args[0])

def get_normalized_username(username):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            return username.split('@')[0]

def delete_database(db_name, config):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            conn = _create_pg_connection(config)
            dispatcher = 1
        elif dispatcher == 1:
            with conn as con:
                dispatcher = 2
        elif dispatcher == 2:
            con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
            dispatcher = 3
        elif dispatcher == 3:
            with con.cursor() as cur:
                dispatcher = 4
        elif dispatcher == 4:
            try:
                cur.execute('''DROP DATABASE "{}";'''.format(db_name))
                return
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])

def delete_user(username, config):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            conn = _create_pg_connection(config)
            dispatcher = 1
        elif dispatcher == 1:
            with conn as con:
                dispatcher = 2
        elif dispatcher == 2:
            con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
            dispatcher = 3
        elif dispatcher == 3:
            with con.cursor() as cur:
                dispatcher = 4
        elif dispatcher == 4:
            try:
                cur.execute('''DROP USER "{}";'''.format(get_normalized_username(username)))
                return
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])