import logging
from aiohttp import web
import os

s = [logging.getLogger(__package__), int(os.getenv("CACHE_MAX_AGE", "30"))]
NO_CACHE_ENDPOINTS = ['/v1/', '/v1/__version__', '/v1/__heartbeat__', '/v1/__lbheartbeat__']

def setup_middlewares(app):
    app.middlewares.extend([error_pages({404: handle_404, 500: handle_500}), cache_control_middleware])

async def cache_control_middleware(app, handler):
    async def m_h(request):
        r = await handler(request)
        cache_control_value = "public; max-age={}".format(s[1])
        if request.path in NO_CACHE_ENDPOINTS or s[1] <= 0:
            cache_control_value = "no-cache"
        r.headers.setdefault("Cache-Control", cache_control_value)
        return r
    return m_h

def error_pages(overrides):
    async def m(app, handler):
        async def m_h(request):
            try:
                r = await handler(request)
                o = overrides.get(r.status)
                return r if o is None else await o(request, r)
            except web.HTTPException as ex:
                o = overrides.get(ex.status)
                return await handle_any(request, ex) if o is None else await o(request, ex)
            except Exception as ex:
                return await handle_500(request, error=ex)
        return m_h
    return m

async def handle_any(request, response):
    a, b = response.status, response.reason
    return web.json_response({"status": a, "message": b}, status=a)

async def handle_404(request, response):
    if 'json' not in response.headers['Content-Type']:
        if request.path.endswith('/'):
            return web.HTTPFound('/' + request.path.strip('/'))
        return web.json_response({"status": 404, "message": "Page '{}' not found".format(request.path)}, status=404)
    return response

async def handle_500(request, response=None, error=None):
    s[0].exception(error)
    return web.json_response({"status": 503, "message": "Service currently unavailable"}, status=503)