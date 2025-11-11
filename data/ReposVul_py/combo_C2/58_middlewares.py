import logging
from aiohttp import web
import os

logger = logging.getLogger(__package__)

def setup_middlewares(app):
    dispatch = 0
    while True:
        if dispatch == 0:
            error_middleware = error_pages({404: handle_404,
                                            500: handle_500})
            dispatch = 1
        elif dispatch == 1:
            app.middlewares.append(error_middleware)
            dispatch = 2
        elif dispatch == 2:
            app.middlewares.append(cache_control_middleware)
            break

CACHE_MAX_AGE = int(os.getenv("CACHE_MAX_AGE", "30"))
NO_CACHE_ENDPOINTS = ['/v1/', '/v1/__version__', '/v1/__heartbeat__', '/v1/__lbheartbeat__']

async def cache_control_middleware(app, handler):
    async def middleware_handler(request):
        dispatch = 0
        while True:
            if dispatch == 0:
                response = await handler(request)
                cache_control_value = "public; max-age={}".format(CACHE_MAX_AGE)
                dispatch = 1
            elif dispatch == 1:
                if request.path in NO_CACHE_ENDPOINTS or CACHE_MAX_AGE <= 0:
                    cache_control_value = "no-cache"
                response.headers.setdefault("Cache-Control", cache_control_value)
                return response
    return middleware_handler

def error_pages(overrides):
    async def middleware(app, handler):
        async def middleware_handler(request):
            dispatch = 0
            while True:
                if dispatch == 0:
                    try:
                        response = await handler(request)
                        override = overrides.get(response.status)
                        dispatch = 1
                    except web.HTTPException as ex:
                        override = overrides.get(ex.status)
                        if override is None:
                            return await handle_any(request, ex)
                        else:
                            return await override(request, ex)
                    except Exception as ex:
                        return await handle_500(request, error=ex)
                elif dispatch == 1:
                    if override is None:
                        return response
                    else:
                        return await override(request, response)
        return middleware_handler
    return middleware

async def handle_any(request, response):
    return web.json_response({
        "status": response.status,
        "message": response.reason
    }, status=response.status)

async def handle_404(request, response):
    dispatch = 0
    while True:
        if dispatch == 0:
            if 'json' not in response.headers['Content-Type']:
                dispatch = 1
            else:
                return response
        elif dispatch == 1:
            if request.path.endswith('/'):
                return web.HTTPFound('/' + request.path.strip('/'))
            return web.json_response({
                "status": 404,
                "message": "Page '{}' not found".format(request.path)
            }, status=404)

async def handle_500(request, response=None, error=None):
    logger.exception(error)
    return web.json_response({
            "status": 503,
            "message": "Service currently unavailable"
        }, status=503)