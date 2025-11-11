import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT
from psycopg2.sql import SQL, Identifier

def OX4B7D5FA9(OX8D3A9E7C):
    if 'server' in OX8D3A9E7C:
        OX3D9C7C4F = '@'.join([OX8D3A9E7C['username'], OX8D3A9E7C['server']])
    else:
        OX3D9C7C4F = OX8D3A9E7C['username']
    return psycopg2.connect(
        database=OX8D3A9E7C['database'],
        user=OX3D9C7C4F,
        host=OX8D3A9E7C['host'],
        port=OX8D3A9E7C['port'],
        password=OX8D3A9E7C['password'],
    )

def OX2A4F8E9D(OX4F7E1C5B, OX3A7D2E9F, OX8D3A9E7C):
    with OX4B7D5FA9(OX8D3A9E7C) as OX4D2E6A9F:
        with OX4D2E6A9F.cursor() as OX9F7C3E1A:
            OX9F7C3E1A.execute("SELECT 1 FROM pg_database WHERE datname=%s;", (OX4F7E1C5B, ))
            OX5A1C8F3E = OX9F7C3E1A.fetchone() is not None
            OX9F7C3E1A.execute("SELECT 1 FROM pg_roles WHERE rolname=%s;", (OX3A7D2E9F, ))
            OX3B9F2D7A = OX9F7C3E1A.fetchone()
            OX6E2A9C1D = OX3B9F2D7A is not None
            return OX5A1C8F3E or OX6E2A9C1D

def OX3F8A2D9C(OX1B7F3E9D, OX8D3A9E7C):
    if OX2A4F8E9D(OX1B7F3E9D["db_name"], OX1B7F3E9D["db_username"], OX8D3A9E7C):
        raise ValueError("db or user already exists")
    with OX4B7D5FA9(OX8D3A9E7C) as OX4D2E6A9F:
        OX4D2E6A9F.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with OX4D2E6A9F.cursor() as OX9F7C3E1A:
            try:
                OX9F7C3E1A.execute(SQL("CREATE USER {} WITH PASSWORD %s;").format(
                    Identifier(OX1B7F3E9D['db_username']),
                ), (
                    OX1B7F3E9D['db_pwd'],
                ))
                OX9F7C3E1A.execute(SQL("GRANT {} TO {};").format(
                    Identifier(OX1B7F3E9D['db_username']),
                    Identifier(OX7A3D9F6B(OX8D3A9E7C['username'])),
                ))
            except psycopg2.ProgrammingError as OX8C5D7B9E:
                raise ValueError(OX8C5D7B9E.args[0])
            try:
                OX9F7C3E1A.execute(SQL("CREATE DATABASE {} OWNER {};").format(
                    Identifier(OX1B7F3E9D['db_name']),
                    Identifier(OX1B7F3E9D['db_username']),
                ))
            except psycopg2.ProgrammingError as OX8C5D7B9E:
                OX9F7C3E1A.execute(SQL("DROP ROLE {};").format(
                    Identifier(OX1B7F3E9D['db_username']),
                ))
                raise ValueError(OX8C5D7B9E.args[0])

def OX7A3D9F6B(OX3D9C7C4F):
    return OX3D9C7C4F.split('@')[0]

def OX3C7E1A9F(OX4F7E1C5B, OX8D3A9E7C):
    with OX4B7D5FA9(OX8D3A9E7C) as OX4D2E6A9F:
        OX4D2E6A9F.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with OX4D2E6A9F.cursor() as OX9F7C3E1A:
            try:
                OX9F7C3E1A.execute('''DROP DATABASE "{}";'''.format(OX4F7E1C5B))
            except psycopg2.ProgrammingError as OX8C5D7B9E:
                raise ValueError(OX8C5D7B9E.args[0])

def OX9E3A6D2F(OX3D9C7C4F, OX8D3A9E7C):
    with OX4B7D5FA9(OX8D3A9E7C) as OX4D2E6A9F:
        OX4D2E6A9F.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        with OX4D2E6A9F.cursor() as OX9F7C3E1A:
            try:
                OX9F7C3E1A.execute('''DROP USER "{}";'''.format(OX7A3D9F6B(OX3D9C7C4F)))
            except psycopg2.ProgrammingError as OX8C5D7B9E:
                raise ValueError(OX8C5D7B9E.args[0])