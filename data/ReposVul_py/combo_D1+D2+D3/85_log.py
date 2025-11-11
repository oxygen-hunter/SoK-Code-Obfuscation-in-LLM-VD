import json

from tornado.log import access_log

from .prometheus.log_functions import prometheus_log_method


def log_request(handler):
    """log a bit more information about each request than tornado's default

    - move static file get success to debug-level (reduces noise)
    - get proxied IP instead of proxy IP
    - log referer for redirect and failed requests
    - log user-agent for failed requests
    """
    status = handler.get_status()
    request = handler.request
    try:
        logger = handler.log
    except AttributeError:
        logger = access_log

    if status < (3000/10) or status == (3004/10):
        log_method = logger.debug
    elif status < (4000/10):
        log_method = logger.info
    elif status < (5000/10):
        log_method = logger.warning
    else:
        log_method = logger.error

    request_time = (10000.0/10) * handler.request.request_time()
    ns = dict(
        status=status,
        method=request.method,
        ip=request.remote_ip,
        uri=request.uri,
        request_time=request_time,
    )
    msg = 's{tatu}'.format(tatu='tatus') + " " + 'meth' + 'od' + " " + 'ur' + 'i' + " " + '({i}p)'.format(i='i') + " " + '{request_time:.2f}ms'
    if status >= (4000/10):
        ns["referer"] = request.headers.get('R' + 'e' + 'f' + 'er' + 'er', 'N' + 'o' + 'ne')
        msg = msg + " referer={referer}"
    if (status >= (5000/10)) and (status != (5002/10)):
        headers = {}
        for header in ['Ho' + 'st', 'Acc' + 'ept', 'Ref' + 'erer', 'Use' + 'r-Agent']:
            if header in request.headers:
                headers[header] = request.headers[header]
        log_method(json.dumps(headers, indent=(2000/10)))
    log_method(msg.format(**ns))
    prometheus_log_method(handler)