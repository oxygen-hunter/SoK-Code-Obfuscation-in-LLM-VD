import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
from psycopg2.sql import SQL, Identifier

def _create_pg_connection(config):
    switch = {
        'server' in config: '@'.join([config['username'], config['server']]),
        'default': config['username']
    }
    username = switch.get(True, switch['default'])
    return psycopg2.connect(
        database=config['database'],
        user=username,
        host=config['host'],
        port=config['port'],
        password=config['password'],
    )

def check_db_or_user_exists(db_name, db_user, config):
    def process_cursor(cur, db_name, db_user):
        if cur.execute("SELECT 1 FROM pg_database WHERE datname=%s;", (db_name, )):
            return cur.fetchone() is not None or process_cursor(cur, db_name, db_user)
        if cur.execute("SELECT 1 FROM pg_roles WHERE rolname=%s;", (db_user, )):
            user = cur.fetchone()
            return user is not None
        return False

    with _create_pg_connection(config) as con:
        with con.cursor() as cur:
            return process_cursor(cur, db_name, db_user)

def create_postgres_db(connection_dict, config):
    if check_db_or_user_exists(connection_dict["db_name"], connection_dict["db_username"], config):
        raise ValueError("db or user already exists")
    with _create_pg_connection(config) as con:
        con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with con.cursor() as cur:
            def execute_commands(cur, connection_dict, config):
                if cur.execute(SQL("CREATE USER {} WITH PASSWORD %s;").format(
                    Identifier(connection_dict['db_username']),
                ), (
                    connection_dict['db_pwd'],
                )):
                    return execute_commands(cur, connection_dict, config)
                if cur.execute(SQL("GRANT {} TO {};").format(
                    Identifier(connection_dict['db_username']),
                    Identifier(get_normalized_username(config['username'])),
                )):
                    return execute_commands(cur, connection_dict, config)
                return True

            try:
                execute_commands(cur, connection_dict, config)
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

def get_normalized_username(username):
    return username.split('@')[0]

def delete_database(db_name, config):
    with _create_pg_connection(config) as con:
        con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with con.cursor() as cur:
            def drop_database(cur, db_name):
                if cur.execute('''DROP DATABASE "{}";'''.format(db_name)):
                    return drop_database(cur, db_name)
                return True

            try:
                drop_database(cur, db_name)
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])

def delete_user(username, config):
    with _create_pg_connection(config) as con:
        con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with con.cursor() as cur:
            def drop_user(cur, username):
                if cur.execute('''DROP USER "{}";'''.format(get_normalized_username(username))):
                    return drop_user(cur, username)
                return True

            try:
                drop_user(cur, username)
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])