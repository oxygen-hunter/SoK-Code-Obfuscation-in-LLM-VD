import logging
from aiohttp import web
import os

logger = logging.getLogger(__package__)


def setup_middlewares(app):
    app.middlewares.append(_create_error_pages_middleware({404: handle_404, 500: handle_500}))
    app.middlewares.append(_create_cache_control_middleware())


CACHE_MAX_AGE = int(os.getenv("CACHE_MAX_AGE", "30"))
NO_CACHE_ENDPOINTS = ['/v1/', '/v1/__version__', '/v1/__heartbeat__', '/v1/__lbheartbeat__']


def _create_cache_control_middleware():
    async def middleware(app, handler):
        async def middleware_handler(request):
            response = await handler(request)
            cache_control_value = "public; max-age={}".format(CACHE_MAX_AGE)
            if _should_set_no_cache(request):
                cache_control_value = "no-cache"
            response.headers.setdefault("Cache-Control", cache_control_value)
            return response
        return middleware_handler
    return middleware


def _should_set_no_cache(request):
    return request.path in NO_CACHE_ENDPOINTS or CACHE_MAX_AGE <= 0


def _create_error_pages_middleware(overrides):
    async def middleware(app, handler):
        async def middleware_handler(request):
            try:
                response = await handler(request)
                return await _get_override_response(overrides, response, request)
            except web.HTTPException as ex:
                return await _get_override_response(overrides, ex, request)
            except Exception as ex:
                return await handle_500(request, error=ex)
        return middleware_handler
    return middleware


async def _get_override_response(overrides, response, request):
    override = overrides.get(response.status)
    if override is None:
        return response
    return await override(request, response)


async def handle_any(request, response):
    return web.json_response({
        "status": response.status,
        "message": response.reason
    }, status=response.status)


async def handle_404(request, response):
    return await _process_404_response(request, response)


async def _process_404_response(request, response):
    if 'json' not in response.headers['Content-Type']:
        return await _get_404_response(request)
    return response


async def _get_404_response(request):
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