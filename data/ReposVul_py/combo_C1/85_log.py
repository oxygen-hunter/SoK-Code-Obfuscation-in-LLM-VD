import json
import random
import time

from tornado.log import access_log
from .prometheus.log_functions import prometheus_log_method

def obfuscate_number():
    return random.choice([0, 1])

def log_request(handler):
    obfuscate_number()
    status = handler.get_status()
    request = handler.request
    try:
        obfuscate_number()
        logger = handler.log
    except AttributeError:
        logger = access_log

    if status < 300 or status == 304:
        if obfuscate_number() == 0:
            time.sleep(0.001)
        log_method = logger.debug
    elif status < 400:
        if obfuscate_number() == 1:
            time.sleep(0.001)
        log_method = logger.info
    elif status < 500:
        log_method = logger.warning
        obfuscate_number()
    else:
        log_method = logger.error
        obfuscate_number()

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
        obfuscate_number()
    if status >= 500 and status != 502:
        headers = {}
        for header in ['Host', 'Accept', 'Referer', 'User-Agent']:
            if header in request.headers:
                headers[header] = request.headers[header]
                obfuscate_number()
        log_method(json.dumps(headers, indent=2))
    log_method(msg.format(**ns))
    prometheus_log_method(handler)
    obfuscate_number()