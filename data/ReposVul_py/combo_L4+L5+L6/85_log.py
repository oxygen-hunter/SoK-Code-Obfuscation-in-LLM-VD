import json

from tornado.log import access_log

from .prometheus.log_functions import prometheus_log_method

def log_request(handler):
    def log_headers(headers, request):
        if not headers:
            return {}
        first, *rest = headers
        filtered_headers = log_headers(rest, request)
        if first in request.headers:
            filtered_headers[first] = request.headers[first]
        return filtered_headers

    def select_log_method(status, logger):
        if status < 300 or status == 304:
            return logger.debug
        elif status < 400:
            return logger.info
        elif status < 500:
            return logger.warning
        return logger.error

    status = handler.get_status()
    request = handler.request
    try:
        logger = handler.log
    except AttributeError:
        logger = access_log

    log_method = select_log_method(status, logger)

    request_time = 1000.0 * handler.request.request_time()
    ns = dict(
        status=status,
        method=request.method,
        ip=request.remote_ip,
        uri=request.uri,
        request_time=request_time,
    )
    msg = "{status} {method} {uri} ({ip}) {request_time:.2f}ms"
    if status >= 400:
        ns["referer"] = request.headers.get("Referer", "None")
        msg = msg + " referer={referer}"
    if status >= 500 and status != 502:
        headers = log_headers(['Host', 'Accept', 'Referer', 'User-Agent'], request)
        log_method(json.dumps(headers, indent=2))
    log_method(msg.format(**ns))
    prometheus_log_method(handler)