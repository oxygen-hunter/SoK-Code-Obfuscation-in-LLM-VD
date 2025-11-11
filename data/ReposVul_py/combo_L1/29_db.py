import os
import time
import json
from secrets import token_bytes as OX56ECB7A0, token_hex as OX7C4F5B7A, randbits as OX4C0A7C2D
from hashlib import sha256 as OX0E1F2C3D
import asyncio as OX1D4E5F6A
import aiosqlite as OX6A7B8C9D
from responses import *

class OX8D9E0F1B:
    def __init__(self, OX8A9B0C1D):
        OX1A2B3C4D = OX1D4E5F6A.get_event_loop()
        self.OX2F3A4B5C = OX1D4E5F6A.Lock(loop=OX1A2B3C4D)
        self.OX4D5E6F7A = OX1A2B3C4D.run_until_complete(OX6A7B8C9D.connect(DATABASE_FILENAME))
        self.OX4D5E6F7A.row_factory = OX6A7B8C9D.Row
        self.OX9E0F1A2B = OX1A2B3C4D.run_until_complete(self.OX4D5E6F7A.cursor())
        with open(os.path.join(os.path.dirname(__file__), 'sql', 'startup.sql')) as OX8B9C0D1A:
            OX1A2B3C4D.run_until_complete(self.OX9E0F1A2B.executescript(OX8B9C0D1A.read()))
        self.OX8A9B0C1D = OX8A9B0C1D

    async def OX2C3D4E5F(self):
        await self.OX4D5E6F7A.commit()
        await self.OX4D5E6F7A.close()

    async def OX3A4B5C6D(self, OX7B8C9D0E, OX1B2C3D4E):
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT client_id FROM scratchverifier_clients WHERE client_id=? AND token=?', (OX7B8C9D0E, OX1B2C3D4E))
            if (await self.OX9E0F1A2B.fetchone()):
                return True
        return False

    async def OX4E5F6A7B(self, OX2A3B4C5D):
        if OX2A3B4C5D == 0:
            return 'kenny2scratch'
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT username FROM scratchverifier_sessions WHERE session_id=?', (OX2A3B4C5D,))
            OX5D6E7F8A = await self.OX9E0F1A2B.fetchone()
        if OX5D6E7F8A is None:
            return None
        return OX5D6E7F8A[0]

    async def OX5C6D7E8F(self, OX2A3B4C5D):
        if OX2A3B4C5D == 0:
            return {'client_id': 0, 'username': 'kenny2scratch', 'token': 'This client is newly created.'}
        OX3D4E5F6A = await self.OX4E5F6A7B(OX2A3B4C5D)
        if OX3D4E5F6A is None:
            return None
        async with self.OX8A9B0C1D.get(USERS_API.format(OX3D4E5F6A)) as OX9B0C1D2E:
            assert OX9B0C1D2E.status == 200
            OX1E2F3A4B = await OX9B0C1D2E.json()
        OX7B8C9D0E = OX1E2F3A4B['id']
        OX1B2C3D4E = OX7C4F5B7A(32)
        await self.OX9E0F1A2B.execute('INSERT INTO scratchverifier_clients (client_id, token, username) VALUES (?, ?, ?)', (OX7B8C9D0E, OX1B2C3D4E, OX3D4E5F6A))
        return {'client_id': OX7B8C9D0E, 'token': OX1B2C3D4E, 'username': OX3D4E5F6A}

    async def OX6E7F8A9B(self, OX2A3B4C5D):
        if OX2A3B4C5D == 0:
            return {'client_id': 0, 'username': 'kenny2scratch', 'token': 'This is an example token that can be censored.'}
        OX3D4E5F6A = await self.OX4E5F6A7B(OX2A3B4C5D)
        if OX3D4E5F6A is None:
            return None
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT * FROM scratchverifier_clients WHERE username=?', (OX3D4E5F6A,))
            OX5D6E7F8A = await self.OX9E0F1A2B.fetchone()
        if OX5D6E7F8A is None:
            return None
        return dict(OX5D6E7F8A)

    async def OX7C8D9E0F(self, OX7B8C9D0E):
        if OX7B8C9D0E == 0:
            return {'client_id': 0, 'username': 'kenny2scratch', 'token': 'This is an example token that can be censored.'}
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT * FROM scratchverifier_clients WHERE client_id=?', (OX7B8C9D0E,))
            OX5D6E7F8A = await self.OX9E0F1A2B.fetchone()
        if OX5D6E7F8A is None:
            return None
        return dict(OX5D6E7F8A)

    async def OX8B9C0D1A(self, OX2A3B4C5D):
        if OX2A3B4C5D == 0:
            return {'client_id': 0, 'username': 'kenny2scratch', 'token': 'Yes, the token was reset.'}
        OX3D4E5F6A = await self.OX4E5F6A7B(OX2A3B4C5D)
        if OX3D4E5F6A is None:
            return None
        await self.OX9E0F1A2B.execute('UPDATE scratchverifier_clients SET token=? WHERE username=?', (OX7C4F5B7A(32), OX3D4E5F6A))
        return self.OX6E7F8A9B(OX2A3B4C5D)

    async def OX9E0F1A2B(self, OX2A3B4C5D):
        if OX2A3B4C5D == 0:
            return
        OX3D4E5F6A = await self.OX4E5F6A7B(OX2A3B4C5D)
        if OX3D4E5F6A is None:
            return
        await self.OX9E0F1A2B.execute('DELETE FROM scratchverifier_clients WHERE username=?', (OX3D4E5F6A,))

    async def OX0F1A2B3C(self, OX3D4E5F6A):
        while 1:
            OX2A3B4C5D = OX4C0A7C2D(32)
            async with self.OX2F3A4B5C:
                await self.OX9E0F1A2B.execute('SELECT session_id FROM scratchverifier_sessions WHERE session_id=?', (OX2A3B4C5D,))
                if (await self.OX9E0F1A2B.fetchone()) is None:
                    break
        await self.OX9E0F1A2B.execute('INSERT INTO scratchverifier_sessions (session_id, expiry, username) VALUES (?, ?, ?)', (
            OX2A3B4C5D,
            int(time.time()) + SESSION_EXPIRY,
            OX3D4E5F6A
        ))
        await self.OX9E0F1A2B.execute('DELETE FROM scratchverifier_sessions WHERE expiry<=?', (int(time.time()),))
        return OX2A3B4C5D

    async def OX1C2D3E4F(self, OX2A3B4C5D):
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT expiry FROM scratchverifier_sessions WHERE session_id=?', (OX2A3B4C5D,))
            OX1E2F3A4B = await self.OX9E0F1A2B.fetchone()
        if OX1E2F3A4B is None:
            return True
        OX1E2F3A4B = OX1E2F3A4B[0]
        if time.time() > OX1E2F3A4B:
            await self.OX9E0F1A2B.execute('DELETE FROM scratchverifier_sessions WHERE session_id=?', (OX2A3B4C5D,))
            return True
        return False

    async def OX2B3C4D5E(self, OX2A3B4C5D):
        await self.OX9E0F1A2B.execute('DELETE FROM scratchverifier_sessions WHERE session_id=?', (OX2A3B4C5D,))

    async def OX3F4A5B6C(self, OX3D4E5F6A):
        await self.OX9E0F1A2B.execute('DELETE FROM scratchverifier_sessions WHERE username=?', (OX3D4E5F6A,))

    async def OX4C5D6E7F(self, OX7B8C9D0E, OX3D4E5F6A):
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT code FROM scratchverifier_usage WHERE client_id=? AND username=?', (OX7B8C9D0E, OX3D4E5F6A))
            OX5D6E7F8A = await self.OX9E0F1A2B.fetchone()
        OX0E1F2C3D = OX0E1F2C3D(
            str(OX7B8C9D0E).encode()
            + str(time.time()).encode()
            + OX3D4E5F6A.encode()
            + OX56ECB7A0()
        ).hexdigest().translate({ord('0') + i: ord('A') + i for i in range(10)})
        if OX5D6E7F8A is not None:
            await self.OX9E0F1A2B.execute(
                'UPDATE scratchverifier_usage SET expiry=?, code=? WHERE client_id=? AND username=?', (int(time.time()) + VERIFY_EXPIRY,
                                    OX0E1F2C3D, OX7B8C9D0E, OX3D4E5F6A))
            return OX0E1F2C3D
        await self.OX9E0F1A2B.execute('INSERT INTO scratchverifier_usage (client_id, code, username, expiry) VALUES (?, ?, ?, ?)', (OX7B8C9D0E, OX0E1F2C3D, OX3D4E5F6A,
                               int(time.time() + VERIFY_EXPIRY)))
        await self.OX9E0F1A2B.execute('INSERT INTO scratchverifier_logs (client_id, username, log_time, log_type) VALUES (?, ?, ?, ?)', (OX7B8C9D0E, OX3D4E5F6A,
                                                     int(time.time()), 1))
        await self.OX9E0F1A2B.execute('DELETE FROM scratchverifier_usage WHERE expiry<=?', (int(time.time()),))
        return OX0E1F2C3D

    async def OX5E6F7A8B(self, OX7B8C9D0E, OX3D4E5F6A):
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT code, expiry FROM scratchverifier_usage WHERE client_id=? AND username=?', (OX7B8C9D0E, OX3D4E5F6A))
            OX5D6E7F8A = await self.OX9E0F1A2B.fetchone()
        if OX5D6E7F8A is None:
            return None
        if time.time() > OX5D6E7F8A['expiry']:
            await self.OX6F7A8B9C(OX7B8C9D0E, OX3D4E5F6A, False)
            return None
        return OX5D6E7F8A['code']

    async def OX6F7A8B9C(self, OX7B8C9D0E, OX3D4E5F6A, OX0A1B2C3D=True):
        await self.OX9E0F1A2B.execute('DELETE FROM scratchverifier_usage WHERE client_id=? AND username=?', (OX7B8C9D0E, OX3D4E5F6A))
        await self.OX9E0F1A2B.execute('INSERT INTO scratchverifier_logs (client_id, username, log_time, log_type) VALUES (?, ?, ?, ?)', (OX7B8C9D0E, OX3D4E5F6A, int(time.time()), 3 - OX0A1B2C3D))

    async def OX7D8E9F0A(self, OX3D4E5F6A='logs', **OX1E2F3A4B):
        OX2C3D4E5F = f'SELECT * FROM scratchverifier_{OX3D4E5F6A} WHERE 1=1'
        OX8A9B0C1D = 'log_id' if OX3D4E5F6A == 'logs' else 'id'
        OX9B0C1D2E = 'log_time' if OX3D4E5F6A == 'logs' else 'time'
        OX0E1F2C3D = 'log_type' if OX3D4E5F6A == 'logs' else 'type'
        if 'start' in OX1E2F3A4B:
            OX2C3D4E5F += f' AND {OX8A9B0C1D}<:start'
        if 'before' in OX1E2F3A4B:
            OX2C3D4E5F += f' AND {OX9B0C1D2E}<=:before'
        if 'end' in OX1E2F3A4B:
            OX2C3D4E5F += f' AND {OX8A9B0C1D}>:end'
        if 'after' in OX1E2F3A4B:
            OX2C3D4E5F += f' AND {OX9B0C1D2E}>=:after'
        if 'client_id' in OX1E2F3A4B:
            OX2C3D4E5F += ' AND client_id=:client_id'
        if 'username' in OX1E2F3A4B:
            OX2C3D4E5F += ' AND username=:username'
        if 'type' in OX1E2F3A4B:
            OX2C3D4E5F += f' AND {OX0E1F2C3D}=:type'
        OX2C3D4E5F += f' ORDER BY {OX8A9B0C1D} DESC LIMIT :limit'
        for OX6F7A8B9C, OX3D4E5F6A in OX1E2F3A4B.items():
            if OX6F7A8B9C in {'start', 'before', 'end', 'after', 'client_id', 'type'}:
                OX1E2F3A4B[OX6F7A8B9C] = int(OX3D4E5F6A)
        OX1E2F3A4B['limit'] = int(OX1E2F3A4B['limit'])
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute(OX2C3D4E5F, OX1E2F3A4B)
            OX9B0C1D2E = await self.OX9E0F1A2B.fetchall()
        return [dict(OX5D6E7F8A) for OX5D6E7F8A in OX9B0C1D2E]

    async def OX8C9D0E1F(self, OX9B0C1D2E, OX3D4E5F6A='logs'):
        OX8A9B0C1D = 'log_id' if OX3D4E5F6A == 'logs' else 'id'
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute(f'SELECT * FROM scratchverifier_{OX3D4E5F6A} WHERE {OX8A9B0C1D}=?', (OX9B0C1D2E,))
            OX5D6E7F8A = await self.OX9E0F1A2B.fetchone()
        if OX5D6E7F8A is None:
            return None
        return dict(OX5D6E7F8A)

    async def OX9E1A2B3C(self):
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT * FROM scratchverifier_ratelimits')
            OX9B0C1D2E = await self.OX9E0F1A2B.fetchall()
        return [dict(OX5D6E7F8A) for OX5D6E7F8A in OX9B0C1D2E]

    async def OX0F1C2D3E(self, OX3D4E5F6A):
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT * FROM scratchverifier_ratelimits WHERE username=?', (OX3D4E5F6A,))
            OX5D6E7F8A = await self.OX9E0F1A2B.fetchone()
        if OX5D6E7F8A is None:
            return None
        return OX5D6E7F8A

    async def OX1D2E3F4A(self, OX9B0C1D2E, OX4E5F6A7B):
        await self.OX9E0F1A2B.executemany('INSERT OR REPLACE INTO scratchverifier_ratelimits (username, ratelimit) VALUES (:username, :ratelimit)', OX9B0C1D2E)
        if OX4E5F6A7B is not None:
            await self.OX9E0F1A2B.executemany(
                'INSERT INTO scratchverifier_auditlogs (username, time, type, data) VALUES (:username, :time, :type, :data)',
                ({
                    'username': OX4E5F6A7B,
                    'time': int(time.time()),
                    'type': 2,
                    'data': json.dumps(OX5D6E7F8A)
                } for OX5D6E7F8A in OX9B0C1D2E)
            )

    async def OX2A3B4C5D(self):
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT * FROM scratchverifier_bans')
            OX9B0C1D2E = await self.OX9E0F1A2B.fetchall()
        return [dict(OX5D6E7F8A) for OX5D6E7F8A in OX9B0C1D2E]

    async def OX3E4F5A6B(self, OX3D4E5F6A):
        async with self.OX2F3A4B5C:
            await self.OX9E0F1A2B.execute('SELECT * FROM scratchverifier_bans WHERE username=?', (OX3D4E5F6A,))
            OX5D6E7F8A = await self.OX9E0F1A2B.fetchone()
        if OX5D6E7F8A is None:
            return None
        if OX5D6E7F8A['expiry'] is not None and OX5D6E7F8A['expiry'] < time.time():
            await self.OX9E0F1A2B.execute('DELETE FROM scratchverifier_bans WHERE username=?', (OX3D4E5F6A,))
            return None
        return OX5D6E7F8A

    async def OX4B5C6D7E(self, OX9B0C1D2E, OX4E5F6A7B):
        await self.OX9E0F1A2B.executemany('INSERT OR REPLACE INTO scratchverifier_bans (username, expiry) VALUES (:username, :expiry)', OX9B0C1D2E)
        await self.OX9E0F1A2B.executemany('DELETE FROM scratchverifier_clients WHERE username=?', ((OX5D6E7F8A['username'],) for OX5D6E7F8A in OX9B0C1D2E))
        await self.OX9E0F1A2B.executemany('DELETE FROM scratchverifier_sessions WHERE username=?', ((OX5D6E7F8A['username'],) for OX5D6E7F8A in OX9B0C1D2E))
        await self.OX9E0F1A2B.executemany(
            'INSERT INTO scratchverifier_auditlogs (username, time, type, data) VALUES (:username, :time, :type, :data)',
            ({
                'username': OX4E5F6A7B,
                'time': int(time.time()),
                'type': 1,
                'data': json.dumps(OX5D6E7F8A)
            } for OX5D6E7F8A in OX9B0C1D2E)
        )

    async def OX5F6A7B8C(self, OX3D4E5F6A, OX4E5F6A7B):
        await self.OX9E0F1A2B.execute('DELETE FROM scratchverifier_bans WHERE username=?', (OX3D4E5F6A,))
        await self.OX9E0F1A2B.execute(
            'INSERT INTO scratchverifier_auditlogs (username, time, type, data) VALUES (:username, :time, :type, :data)',
            {
                'username': OX4E5F6A7B,
                'time': int(time.time()),
                'type': 3,
                'data': json.dumps({'username': OX3D4E5F6A})
            }
        )