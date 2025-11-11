import logging
from aiohttp import web
import os

OX7B4DF339 = logging.getLogger(__package__)

def OX3A1F2B71(OX2E8C9F60):
    OX4C5D9E63 = OX7F8E1A23({404: OX832E4BCA,
                                    500: OX6C2D3E41})
    OX2E8C9F60.middlewares.append(OX4C5D9E63)
    OX2E8C9F60.middlewares.append(OX9C7D6A1B)

OX9D8F4D20 = int(os.getenv("CACHE_MAX_AGE", "30"))
OX1E9B3A07 = ['/v1/', '/v1/__version__', '/v1/__heartbeat__', '/v1/__lbheartbeat__']

async def OX9C7D6A1B(OX2E8C9F60, OX4B5E3D42):
    async def OX0A1B2C3D(OX5D6E4F23):
        OX8F9A7B6C = await OX4B5E3D42(OX5D6E4F23)
        OX0D1E2F3C = "public; max-age={}".format(OX9D8F4D20)
        if OX5D6E4F23.path in OX1E9B3A07 or OX9D8F4D20 <= 0:
            OX0D1E2F3C = "no-cache"
        OX8F9A7B6C.headers.setdefault("Cache-Control", OX0D1E2F3C)
        return OX8F9A7B6C
    return OX0A1B2C3D

def OX7F8E1A23(OX6B5C4D3A):
    async def OX1A2B3C4D(OX2E8C9F60, OX4B5E3D42):
        async def OX5E6F7A8B(OX5D6E4F23):
            try:
                OX8F9A7B6C = await OX4B5E3D42(OX5D6E4F23)
                OX7A8B9C0D = OX6B5C4D3A.get(OX8F9A7B6C.status)
                if OX7A8B9C0D is None:
                    return OX8F9A7B6C
                else:
                    return await OX7A8B9C0D(OX5D6E4F23, OX8F9A7B6C)
            except web.HTTPException as OX9B8C7D6E:
                OX7A8B9C0D = OX6B5C4D3A.get(OX9B8C7D6E.status)
                if OX7A8B9C0D is None:
                    return await OX0D9E8F7A(OX5D6E4F23, OX9B8C7D6E)
                else:
                    return await OX7A8B9C0D(OX5D6E4F23, OX9B8C7D6E)
            except Exception as OX9B8C7D6E:
                return await OX6C2D3E41(OX5D6E4F23, error=OX9B8C7D6E)
        return OX5E6F7A8B
    return OX1A2B3C4D

async def OX0D9E8F7A(OX5D6E4F23, OX8F9A7B6C):
    return web.json_response({
        "status": OX8F9A7B6C.status,
        "message": OX8F9A7B6C.reason
    }, status=OX8F9A7B6C.status)

async def OX832E4BCA(OX5D6E4F23, OX8F9A7B6C):
    if 'json' not in OX8F9A7B6C.headers['Content-Type']:
        if OX5D6E4F23.path.endswith('/'):
            return web.HTTPFound('/' + OX5D6E4F23.path.strip('/'))
        return web.json_response({
            "status": 404,
            "message": "Page '{}' not found".format(OX5D6E4F23.path)
        }, status=404)
    return OX8F9A7B6C

async def OX6C2D3E41(OX5D6E4F23, OX8F9A7B6C=None, error=None):
    OX7B4DF339.exception(error)
    return web.json_response({
            "status": 503,
            "message": "Service currently unavailable"
        }, status=503)