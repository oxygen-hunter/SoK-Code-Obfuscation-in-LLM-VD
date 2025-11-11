import logging
from aiohttp import web
import os

logger = logging.getLogger(__package__)

def setup_middlewares(app):
    error_middleware = _error_pages({404: _handle_404,
                                    500: _handle_500})
    app.middlewares.append(error_middleware)
    app.middlewares.append(_cache_control_middleware)

CACHE_MAX_AGE = int(os.getenv("CACHE_MAX_AGE", "30"))
NO_CACHE_ENDPOINTS = ['/v1/', '/v1/__version__', '/v1/__heartbeat__', '/v1/__lbheartbeat__']

async def _cache_control_middleware(app, handler):
    async def _middleware_handler(request):
        response = await handler(request)
        _cache_control_value = "public; max-age={}".format(CACHE_MAX_AGE)
        if request.path in NO_CACHE_ENDPOINTS or CACHE_MAX_AGE <= 0:
            _cache_control_value = "no-cache"
        response.headers.setdefault("Cache-Control", _cache_control_value)
        return response
    return _middleware_handler

def _error_pages(overrides):
    async def _middleware(app, handler):
        async def _middleware_handler(request):
            try:
                response = await handler(request)
                override = overrides.get(response.status)
                if override is None:
                    return response
                else:
                    return await override(request, response)
            except web.HTTPException as ex:
                override = overrides.get(ex.status)
                if override is None:
                    return await _handle_any(request, ex)
                else:
                    return await override(request, ex)
            except Exception as ex:
                return await _handle_500(request, error=ex)
        return _middleware_handler
    return _middleware

async def _handle_any(request, response):
    return web.json_response({
        "status": response.status,
        "message": response.reason
    }, status=response.status)

async def _handle_404(request, response):
    if 'json' not in response.headers['Content-Type']:
        if request.path.endswith('/'):
            return web.HTTPFound('/' + request.path.strip('/'))
        return web.json_response({
            "status": 404,
            "message": "Page '{}' not found".format(request.path)
        }, status=404)
    return response

async def _handle_500(request, response=None, error=None):
    logger.exception(error)
    return web.json_response({
            "status": 503,
            "message": "Service currently unavailable"
        }, status=503)