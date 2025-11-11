import json

from tornado.log import access_log

from .prometheus.log_functions import prometheus_log_method


def log_request(handler):
    def get_status():
        return handler.get_status()

    def get_request():
        return handler.request

    def get_logger():
        try:
            return handler.log
        except AttributeError:
            return access_log

    def get_request_time():
        return 1000.0 * handler.request.request_time()

    def get_method():
        return request.method

    def get_ip():
        return request.remote_ip

    def get_uri():
        return request.uri

    def get_headers():
        return request.headers

    def get_header_value(header_name, default_value="None"):
        return request.headers.get(header_name, default_value)

    status = get_status()
    request = get_request()
    logger = get_logger()

    if status < 300 or status == 304:
        log_method = logger.debug
    elif status < 400:
        log_method = logger.info
    elif status < 500:
        log_method = logger.warning
    else:
        log_method = logger.error

    request_time = get_request_time()
    ns = dict(
        status=status,
        method=get_method(),
        ip=get_ip(),
        uri=get_uri(),
        request_time=request_time,
    )
    msg = "{status} {method} {uri} ({ip}) {request_time:.2f}ms"
    if status >= 400:
        ns["referer"] = get_header_value("Referer")
        msg = msg + " referer={referer}"
    if status >= 500 and status != 502:
        headers = {}
        for header in ['Host', 'Accept', 'Referer', 'User-Agent']:
            if header in get_headers():
                headers[header] = get_header_value(header)
        log_method(json.dumps(headers, indent=2))
    log_method(msg.format(**ns))
    prometheus_log_method(handler)