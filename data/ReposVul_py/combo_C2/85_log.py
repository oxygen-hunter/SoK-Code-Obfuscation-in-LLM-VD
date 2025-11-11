import json

from tornado.log import access_log
from .prometheus.log_functions import prometheus_log_method

def log_request(handler):
    status = handler.get_status()
    request = handler.request
    try:
        logger = handler.log
    except AttributeError:
        logger = access_log

    state = 0
    while True:
        if state == 0:
            if status < 300 or status == 304:
                log_method = logger.debug
                state = 1
            else:
                state = 2
        elif state == 1:
            request_time = 1000.0 * handler.request.request_time()
            ns = dict(
                status=status,
                method=request.method,
                ip=request.remote_ip,
                uri=request.uri,
                request_time=request_time,
            )
            msg = "{status} {method} {uri} ({ip}) {request_time:.2f}ms"
            state = 8
        elif state == 2:
            if status < 400:
                log_method = logger.info
                state = 1
            else:
                state = 3
        elif state == 3:
            if status < 500:
                log_method = logger.warning
                state = 1
            else:
                log_method = logger.error
                state = 1
        elif state == 8:
            if status >= 400:
                ns["referer"] = request.headers.get("Referer", "None")
                msg = msg + " referer={referer}"
                state = 9
            else:
                state = 10
        elif state == 9:
            state = 10
        elif state == 10:
            if status >= 500 and status != 502:
                headers = {}
                for header in ['Host', 'Accept', 'Referer', 'User-Agent']:
                    if header in request.headers:
                        headers[header] = request.headers[header]
                log_method(json.dumps(headers, indent=2))
                state = 11
            else:
                state = 11
        elif state == 11:
            log_method(msg.format(**ns))
            prometheus_log_method(handler)
            break