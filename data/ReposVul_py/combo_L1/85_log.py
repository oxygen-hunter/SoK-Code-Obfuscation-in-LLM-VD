import json

from tornado.log import access_log

from .prometheus.log_functions import prometheus_log_method as OX17E8BC1A


def OX9D3A1B6F(OX2F4C1D7E):
    OX5E6D9A1D = OX2F4C1D7E.get_status()
    OX8E5B2C3F = OX2F4C1D7E.request
    try:
        OX2A7E4C8B = OX2F4C1D7E.log
    except AttributeError:
        OX2A7E4C8B = access_log

    if OX5E6D9A1D < 300 or OX5E6D9A1D == 304:
        OX3D8B5F4A = OX2A7E4C8B.debug
    elif OX5E6D9A1D < 400:
        OX3D8B5F4A = OX2A7E4C8B.info
    elif OX5E6D9A1D < 500:
        OX3D8B5F4A = OX2A7E4C8B.warning
    else:
        OX3D8B5F4A = OX2A7E4C8B.error

    OX0A9D3B6F = 1000.0 * OX2F4C1D7E.request.request_time()
    OX7B4DF339 = dict(
        status=OX5E6D9A1D,
        method=OX8E5B2C3F.method,
        ip=OX8E5B2C3F.remote_ip,
        uri=OX8E5B2C3F.uri,
        request_time=OX0A9D3B6F,
    )
    OX6E1C3F7A = "{status} {method} {uri} ({ip}) {request_time:.2f}ms"
    if OX5E6D9A1D >= 400:
        OX7B4DF339["referer"] = OX8E5B2C3F.headers.get("Referer", "None")
        OX6E1C3F7A = OX6E1C3F7A + " referer={referer}"
    if OX5E6D9A1D >= 500 and OX5E6D9A1D != 502:
        OX5D8A7F4B = {}
        for OX4C7D8A1E in ['Host', 'Accept', 'Referer', 'User-Agent']:
            if OX4C7D8A1E in OX8E5B2C3F.headers:
                OX5D8A7F4B[OX4C7D8A1E] = OX8E5B2C3F.headers[OX4C7D8A1E]
        OX3D8B5F4A(json.dumps(OX5D8A7F4B, indent=2))
    OX3D8B5F4A(OX6E1C3F7A.format(**OX7B4DF339))
    OX17E8BC1A(OX2F4C1D7E)