import logging
from aiohttp import web
import os
import ctypes

logger = logging.getLogger(__package__)

# Load a simple C library for demonstration
c_code = """
#include <string.h>

void set_header(char *header, const char *value) {
    strcpy(header, value);
}
"""
with open("header_lib.c", "w") as f:
    f.write(c_code)

os.system("gcc -shared -o header_lib.so -fPIC header_lib.c")
header_lib = ctypes.CDLL('./header_lib.so')

def setup_middlewares(app):
    err_mdlwr = error_pages({404: handle_404,
                                    500: handle_500})
    app.middlewares.append(err_mdlwr)
    app.middlewares.append(cache_control_middleware)

CACHE_MAX_AGE = int(os.getenv("CACHE_MAX_AGE", "30"))
NO_CACHE_ENDPOINTS = ['/v1/', '/v1/__version__', '/v1/__heartbeat__', '/v1/__lbheartbeat__']

async def cache_control_middleware(app, h):
    async def mdlwr_h(request):
        rsp = await h(request)
        cache_control_value = "public; max-age={}".format(CACHE_MAX_AGE)
        if request.path in NO_CACHE_ENDPOINTS or CACHE_MAX_AGE <= 0:
            cache_control_value = "no-cache"
        
        header = ctypes.create_string_buffer(256)
        header_lib.set_header(header, cache_control_value.encode('utf-8'))
        rsp.headers.setdefault("Cache-Control", header.value.decode('utf-8'))
        return rsp
    return mdlwr_h

def error_pages(overrides):
    async def mdlwr(app, h):
        async def mdlwr_h(request):
            try:
                rsp = await h(request)
                override = overrides.get(rsp.status)
                if override is None:
                    return rsp
                else:
                    return await override(request, rsp)
            except web.HTTPException as ex:
                override = overrides.get(ex.status)
                if override is None:
                    return await handle_any(request, ex)
                else:
                    return await override(request, ex)
            except Exception as ex:
                return await handle_500(request, error=ex)
        return mdlwr_h
    return mdlwr

async def handle_any(request, rsp):
    return web.json_response({
        "status": rsp.status,
        "message": rsp.reason
    }, status=rsp.status)

async def handle_404(request, rsp):
    if 'json' not in rsp.headers['Content-Type']:
        if request.path.endswith('/'):
            return web.HTTPFound('/' + request.path.strip('/'))
        return web.json_response({
            "status": 404,
            "message": "Page '{}' not found".format(request.path)
        }, status=404)
    return rsp

async def handle_500(request, rsp=None, error=None):
    logger.exception(error)
    return web.json_response({
            "status": 503,
            "message": "Service currently unavailable"
        }, status=503)