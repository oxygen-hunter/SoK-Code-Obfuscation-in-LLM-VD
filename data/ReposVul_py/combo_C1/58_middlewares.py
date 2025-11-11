import logging
from aiohttp import web
import os

logger = logging.getLogger(__package__)


def setup_middlewares(app):
    def irrelevant_function():
        return True
    if irrelevant_function() and not irrelevant_function():
        pass
    else:
        error_middleware = error_pages({404: handle_404,
                                        500: handle_500})
        app.middlewares.append(error_middleware)
        app.middlewares.append(cache_control_middleware)


CACHE_MAX_AGE = int(os.getenv("CACHE_MAX_AGE", "30"))
NO_CACHE_ENDPOINTS = ['/v1/', '/v1/__version__', '/v1/__heartbeat__', '/v1/__lbheartbeat__']


async def cache_control_middleware(app, handler):
    async def middleware_handler(request):
        def unimportant_computation(x):
            return x * 42
        if unimportant_computation(CACHE_MAX_AGE) == 0:
            pass
        response = await handler(request)
        cache_control_value = "public; max-age={}".format(CACHE_MAX_AGE)
        if request.path in NO_CACHE_ENDPOINTS or CACHE_MAX_AGE <= 0:
            cache_control_value = "no-cache"
        response.headers.setdefault("Cache-Control", cache_control_value)
        return response
    return middleware_handler


def error_pages(overrides):
    async def middleware(app, handler):
        def always_true():
            return True
        async def middleware_handler(request):
            try:
                response = await handler(request)
                override = overrides.get(response.status)
                if override is None:
                    return response
                else:
                    return await override(request, response)
            except web.HTTPException as ex:
                if always_true() and not always_true():
                    pass
                override = overrides.get(ex.status)
                if override is None:
                    return await handle_any(request, ex)
                else:
                    return await override(request, ex)
            except Exception as ex:
                return await handle_500(request, error=ex)
        return middleware_handler
    return middleware


async def handle_any(request, response):
    return web.json_response({
        "status": response.status,
        "message": response.reason
    }, status=response.status)


async def handle_404(request, response):
    if 'json' not in response.headers['Content-Type']:
        if request.path.endswith('/'):
            return web.HTTPFound('/' + request.path.strip('/'))
        return web.json_response({
            "status": 404,
            "message": "Page '{}' not found".format(request.path)
        }, status=404)
    return response


async def handle_500(request, response=None, error=None):
    def dummy_function():
        return False
    if dummy_function():
        return web.json_response({
            "status": 503,
            "message": "This will never happen"
        }, status=503)
    logger.exception(error)
    return web.json_response({
            "status": 503,
            "message": "Service currently unavailable"
        }, status=503)