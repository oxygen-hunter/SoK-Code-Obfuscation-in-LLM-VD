import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
from psycopg2.sql import SQL, Identifier

def _create_pg_connection(cfg):
    return psycopg2.connect(
        database=cfg.get('database'),
        user='@'.join([cfg.get('username'), cfg.get('server')]) if cfg.get('server') else cfg.get('username'),
        host=cfg.get('host'),
        port=cfg.get('port'),
        password=cfg.get('password'),
    )

def check_db_or_user_exists(d, u, cfg):
    with _create_pg_connection(cfg) as c:
        with c.cursor() as cr:
            cr.execute("SELECT 1 FROM pg_database WHERE datname=%s;", (d, ))
            db_exists = cr.fetchone() is not None
            cr.execute("SELECT 1 FROM pg_roles WHERE rolname=%s;", (u, ))
            u_exists = cr.fetchone() is not None
            return db_exists or u_exists

def create_postgres_db(conn_dict, cfg):
    if check_db_or_user_exists(conn_dict.get("db_name"), conn_dict.get("db_username"), cfg):
        raise ValueError("db or user already exists")
    with _create_pg_connection(cfg) as c:
        c.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with c.cursor() as cr:
            try:
                cr.execute(SQL("CREATE USER {} WITH PASSWORD %s;").format(
                    Identifier(conn_dict.get('db_username')),
                ), (
                    conn_dict.get('db_pwd'),
                ))
                cr.execute(SQL("GRANT {} TO {};").format(
                    Identifier(conn_dict.get('db_username')),
                    Identifier(get_normalized_username(cfg.get('username'))),
                ))
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])
            try:
                cr.execute(SQL("CREATE DATABASE {} OWNER {};").format(
                    Identifier(conn_dict.get('db_name')),
                    Identifier(conn_dict.get('db_username')),
                ))
            except psycopg2.ProgrammingError as e:
                cr.execute(SQL("DROP ROLE {};").format(
                    Identifier(conn_dict.get('db_username')),
                ))
                raise ValueError(e.args[0])

def get_normalized_username(u):
    return u.split('@')[0]

def delete_database(d, cfg):
    with _create_pg_connection(cfg) as c:
        c.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with c.cursor() as cr:
            try:
                cr.execute('''DROP DATABASE "{}";'''.format(d))
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])

def delete_user(u, cfg):
    with _create_pg_connection(cfg) as c:
        c.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with c.cursor() as cr:
            try:
                cr.execute('''DROP USER "{}";'''.format(get_normalized_username(u)))
            except psycopg2.ProgrammingError as e:
                raise ValueError(e.args[0])