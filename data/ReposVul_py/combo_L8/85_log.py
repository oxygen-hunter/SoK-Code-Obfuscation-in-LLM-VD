import json
from ctypes import CDLL, c_double, c_int, c_char_p

from tornado.log import access_log
from .prometheus.log_functions import prometheus_log_method

c_library = CDLL('./log_helper.so')  # Assume there's a C library compiled as log_helper.so

c_library.get_log_method.restype = c_int
c_library.get_log_method.argtypes = [c_int]
c_library.log_headers.restype = None
c_library.log_headers.argtypes = [c_char_p]

def log_request(handler):
    status = handler.get_status()
    request = handler.request
    try:
        logger = handler.log
    except AttributeError:
        logger = access_log

    log_level = c_library.get_log_method(status)
    log_methods = [logger.debug, logger.info, logger.warning, logger.error]
    log_method = log_methods[log_level]

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
        headers = {}
        relevant_headers = ['Host', 'Accept', 'Referer', 'User-Agent']
        for header in relevant_headers:
            if header in request.headers:
                headers[header] = request.headers[header]
        c_library.log_headers(json.dumps(headers, indent=2).encode('utf-8'))
    log_method(msg.format(**ns))
    prometheus_log_method(handler)
```

C code (log_helper.c):
```c
#include <string.h>
#include <stdio.h>

int get_log_method(int status) {
    if (status < 300 || status == 304) return 0;
    if (status < 400) return 1;
    if (status < 500) return 2;
    return 3;
}

void log_headers(const char* headers) {
    printf("%s\n", headers);
}
```

Compile C code:
```bash
gcc -shared -o log_helper.so -fPIC log_helper.c