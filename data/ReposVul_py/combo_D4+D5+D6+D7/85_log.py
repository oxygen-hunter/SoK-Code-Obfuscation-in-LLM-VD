import json

from tornado.log import access_log

from .prometheus.log_functions import prometheus_log_method

global_metrics = {"prometheus": prometheus_log_method, "access": access_log}

def log_request(handler):
    t_status, t_request, t_logger, t_method = handler.get_status(), handler.request, None, None
    try:
        t_logger = handler.log
    except AttributeError:
        t_logger = global_metrics["access"]

    if t_status < 300 or t_status == 304:
        t_method = t_logger.debug
    elif t_status < 400:
        t_method = t_logger.info
    elif t_status < 500:
        t_method = t_logger.warning
    else:
        t_method = t_logger.error

    t_request_time = 1000.0 * handler.request.request_time()
    t_ns = dict(
        method=t_request.method,
        ip=t_request.remote_ip,
        uri=t_request.uri,
        request_time=t_request_time,
        status=t_status,
    )
    t_msg = "{status} {method} {uri} ({ip}) {request_time:.2f}ms"
    if t_status >= 400:
        t_ns["referer"] = t_request.headers.get("Referer", "None")
        t_msg += " referer={referer}"
    if t_status >= 500 and t_status != 502:
        t_headers = {}
        for t_header in ['Host', 'Accept', 'Referer', 'User-Agent']:
            if t_header in t_request.headers:
                t_headers[t_header] = t_request.headers[t_header]
        t_method(json.dumps(t_headers, indent=2))
    t_method(t_msg.format(**t_ns))
    global_metrics["prometheus"](handler)