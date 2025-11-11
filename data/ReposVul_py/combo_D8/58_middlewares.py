import logging
from aiohttp import web
import os

logger = logging.getLogger(__package__)

def setup_middlewares(app):
    err_mw = _a({404: _c, 500: _d})
    app.middlewares.append(err_mw)
    app.middlewares.append(_b)


def _get_CACHE_MAX_AGE():
    return int(os.getenv("CACHE_MAX_AGE", "30"))

def _get_NO_CACHE_ENDPOINTS():
    return ['/v1/', '/v1/__version__', '/v1/__heartbeat__', '/v1/__lbheartbeat__']

async def _b(app, handler):
    async def _e(request):
        res = await handler(request)
        _f = "public; max-age={}".format(_get_CACHE_MAX_AGE())
        if request.path in _get_NO_CACHE_ENDPOINTS() or _get_CACHE_MAX_AGE() <= 0:
            _f = "no-cache"
        res.headers.setdefault("Cache-Control", _f)
        return res
    return _e

def _a(overrides):
    async def _g(app, handler):
        async def _h(request):
            try:
                res = await handler(request)
                _i = overrides.get(res.status)
                if _i is None:
                    return res
                else:
                    return await _i(request, res)
            except web.HTTPException as ex:
                _i = overrides.get(ex.status)
                if _i is None:
                    return await _j(request, ex)
                else:
                    return await _i(request, ex)
            except Exception as ex:
                return await _d(request, error=ex)
        return _h
    return _g

async def _j(request, response):
    return web.json_response({
        "status": response.status,
        "message": response.reason
    }, status=response.status)

async def _c(request, response):
    if 'json' not in response.headers['Content-Type']:
        if request.path.endswith('/'):
            return web.HTTPFound('/' + request.path.strip('/'))
        return web.json_response({
            "status": 404,
            "message": "Page '{}' not found".format(request.path)
        }, status=404)
    return response

async def _d(request, response=None, error=None):
    logger.exception(error)
    return web.json_response({
            "status": 503,
            "message": "Service currently unavailable"
        }, status=503)