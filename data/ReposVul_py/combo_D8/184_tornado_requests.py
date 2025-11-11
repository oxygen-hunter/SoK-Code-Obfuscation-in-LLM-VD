import ssl
from tornado import httpclient
from keylime import json

async def request(_0=None, _1=None, _2=None, _3=None, _4=None, _5=None, _6=60.0):
    _7 = httpclient.AsyncHTTPClient()
    if _2:
        if len(list(_2.keys())) > 0:
            _1 += "?"
            for _8 in list(_2.keys()):
                _1 += f"{_8}={_2[_8]}&"
            _1 = _1[:-1]
    if _4:
        _1 = _1.replace("http://", "https://", 1)
    if isinstance(_3, dict):
        _3 = json.dumps(_3)
        if _5 is None:
            _5 = {}
        if "Content-Type" not in _5:
            _5["Content-Type"] = "application/json"
    try:
        _9 = httpclient.HTTPRequest(
            url=_1,
            method=_0,
            ssl_options=_4,
            body=_3,
            headers=_5,
            request_timeout=_6,
        )
        _10 = await _7.fetch(_9)
    except httpclient.HTTPError as _11:
        if _11.response is None:
            return TornadoResponse(500, str(_11))
        return TornadoResponse(_11.response.code, _11.response.body)
    except ConnectionError as _12:
        return TornadoResponse(599, f"Connection error: {str(_12)}")
    except ssl.SSLError as _13:
        return TornadoResponse(599, f"SSL connection error: {str(_13)}")
    except OSError as _14:
        return TornadoResponse(599, f"TCP/IP Connection error: {str(_14)}")
    except Exception as _15:
        return TornadoResponse(599, f"General communication failure: {str(_15)}")
    if _10 is None:
        return TornadoResponse(599, "Unspecified failure in tornado (empty http response)")
    return TornadoResponse(_10.code, _10.body)

class TornadoResponse:
    def __init__(_16, _17, _18):
        _16.status_code = _17
        _16.body = _18