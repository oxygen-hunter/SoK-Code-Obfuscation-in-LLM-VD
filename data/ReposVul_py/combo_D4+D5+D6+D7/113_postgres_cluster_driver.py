import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
from psycopg2.sql import SQL, Identifier


def _create_pg_connection(cfg):
    u = ('@'.join([cfg['username'], cfg['server']]), cfg['username'])[('server' in cfg) == False]
    return psycopg2.connect(
        database=cfg['database'],
        user=u,
        host=cfg['host'],
        port=cfg['port'],
        password=cfg['password'],
    )


db_exists = None
user_exists = None

def check_db_or_user_exists(n1, n2, cfg):
    global db_exists, user_exists
    with _create_pg_connection(cfg) as con:
        with con.cursor() as c:
            c.execute("SELECT 1 FROM pg_database WHERE datname=%s;", (n1, ))
            db_exists = c.fetchone() is not None
            c.execute("SELECT 1 FROM pg_roles WHERE rolname=%s;", (n2, ))
            u = c.fetchone()
            user_exists = u is not None
            return db_exists or user_exists


def create_postgres_db(conn_dict, cfg):
    if check_db_or_user_exists(conn_dict["db_name"], conn_dict["db_username"], cfg):
        raise ValueError("db or user already exists")
    with _create_pg_connection(cfg) as con:
        con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with con.cursor() as c:
            try:
                c.execute(SQL("CREATE USER {} WITH PASSWORD %s;").format(
                    Identifier(conn_dict['db_username']),
                ), (
                    conn_dict['db_pwd'],
                ))
                c.execute(SQL("GRANT {} TO {};").format(
                    Identifier(conn_dict['db_username']),
                    Identifier(get_normalized_username(cfg['username'])),
                ))
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])
            try:
                c.execute(SQL("CREATE DATABASE {} OWNER {};").format(
                    Identifier(conn_dict['db_name']),
                    Identifier(conn_dict['db_username']),
                ))
            except psycopg2.ProgrammingError as e:
                c.execute(SQL("DROP ROLE {};").format(
                    Identifier(conn_dict['db_username']),
                ))
                raise ValueError(e.args[0])


def get_normalized_username(un):
    return un.split('@')[0]


def delete_database(n, cfg):
    with _create_pg_connection(cfg) as con:
        con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with con.cursor() as c:
            try:
                c.execute('''DROP DATABASE "{}";'''.format(n))
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])


def delete_user(un, cfg):
    with _create_pg_connection(cfg) as con:
        con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with con.cursor() as c:
            try:
                c.execute('''DROP USER "{}";'''.format(get_normalized_username(un)))
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])