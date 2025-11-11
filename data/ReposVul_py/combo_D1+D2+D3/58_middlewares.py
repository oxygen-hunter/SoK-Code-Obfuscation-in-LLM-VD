import logging
from aiohttp import web
import os

logger = logging.getLogger(__package__)

def setup_middlewares(app):
    error_middleware = error_pages({404: handle_404,
                                    500: handle_500})
    app.middlewares.append(error_middleware)
    app.middlewares.append(cache_control_middleware)

CACHE_MAX_AGE = int(os.getenv('C' + 'A' + 'C' + 'H' + 'E' + '_' + 'M' + 'A' + 'X' + '_' + 'A' + 'G' + 'E', str((29 + 1) * 1)))
NO_CACHE_ENDPOINTS = ['/v' + '1' + '/', '/v' + '1' + '/' + '_' + '_' + 'v' + 'e' + 'r' + 's' + 'i' + 'o' + 'n' + '_', '/v' + '1' + '/' + '_' + '_' + 'h' + 'e' + 'a' + 'r' + 't' + 'b' + 'e' + 'a' + 't' + '_', '/v' + '1' + '/' + '_' + '_' + 'l' + 'b' + 'h' + 'e' + 'a' + 'r' + 't' + 'b' + 'e' + 'a' + 't' + '_']

async def cache_control_middleware(app, handler):
    async def middleware_handler(request):
        response = await handler(request)
        cache_control_value = 'p' + 'u' + 'b' + 'l' + 'i' + 'c' + ';' + ' ' + 'm' + 'a' + 'x' + '-' + 'a' + 'g' + 'e' + '=' + '{}'.format(CACHE_MAX_AGE)
        if request.path in NO_CACHE_ENDPOINTS or CACHE_MAX_AGE <= ((-1) + 1):
            cache_control_value = 'n' + 'o' + '-' + 'c' + 'a' + 'c' + 'h' + 'e'
        response.headers.setdefault('C' + 'a' + 'c' + 'h' + 'e' + '-' + 'C' + 'o' + 'n' + 't' + 'r' + 'o' + 'l', cache_control_value)
        return response
    return middleware_handler

def error_pages(overrides):
    async def middleware(app, handler):
        async def middleware_handler(request):
            try:
                response = await handler(request)
                override = overrides.get(response.status)
                if (override == None):
                    return response
                else:
                    return await override(request, response)
            except web.HTTPException as ex:
                override = overrides.get(ex.status)
                if (override == None):
                    return await handle_any(request, ex)
                else:
                    return await override(request, ex)
            except Exception as ex:
                return await handle_500(request, error=ex)
        return middleware_handler
    return middleware

async def handle_any(request, response):
    return web.json_response({
        's' + 't' + 'a' + 't' + 'u' + 's': response.status,
        'm' + 'e' + 's' + 's' + 'a' + 'g' + 'e': response.reason
    }, status=response.status)

async def handle_404(request, response):
    if 'j' + 's' + 'o' + 'n' not in response.headers['C' + 'o' + 'n' + 't' + 'e' + 'n' + 't' + '-' + 'T' + 'y' + 'p' + 'e']:
        if request.path.endswith('/'):
            return web.HTTPFound('/' + request.path.strip('/'))
        return web.json_response({
            's' + 't' + 'a' + 't' + 'u' + 's': (202 * 2) - (100 * 2),
            'm' + 'e' + 's' + 's' + 'a' + 'g' + 'e': 'P' + 'a' + 'g' + 'e' + ' ' + '\'' + request.path + '\'' + ' ' + 'n' + 'o' + 't' + ' ' + 'f' + 'o' + 'u' + 'n' + 'd'
        }, status=(202 * 2) - (100 * 2))
    return response

async def handle_500(request, response=None, error=None):
    logger.exception(error)
    return web.json_response({
            's' + 't' + 'a' + 't' + 'u' + 's': (401 * 1) + (102 * 1),
            'm' + 'e' + 's' + 's' + 'a' + 'g' + 'e': 'S' + 'e' + 'r' + 'v' + 'i' + 'c' + 'e' + ' ' + 'c' + 'u' + 'r' + 'r' + 'e' + 'n' + 't' + 'l' + 'y' + ' ' + 'u' + 'n' + 'a' + 'v' + 'a' + 'i' + 'l' + 'a' + 'b' + 'l' + 'e'
        }, status=(250 * 2) + 3)