import ssl
from tornado import httpclient
from keylime import json

async def request(method, url, params=None, data=None, context=None, headers=None, timeout=60.0):
    __state__ = 0
    while True:
        if __state__ == 0:
            http_client = httpclient.AsyncHTTPClient()
            __state__ = 1
        elif __state__ == 1:
            if params is not None and len(list(params.keys())) > 0:
                url += "?"
                __state__ = 2
            else:
                __state__ = 5
        elif __state__ == 2:
            for key in list(params.keys()):
                url += f"{key}={params[key]}&"
            url = url[:-1]
            __state__ = 5
        elif __state__ == 5:
            if context is not None:
                url = url.replace("http://", "https://", 1)
            __state__ = 6
        elif __state__ == 6:
            if isinstance(data, dict):
                data = json.dumps(data)
                if headers is None:
                    headers = {}
                if "Content-Type" not in headers:
                    headers["Content-Type"] = "application/json"
            __state__ = 7
        elif __state__ == 7:
            try:
                req = httpclient.HTTPRequest(
                    url=url,
                    method=method,
                    ssl_options=context,
                    body=data,
                    headers=headers,
                    request_timeout=timeout,
                )
                response = await http_client.fetch(req)
                __state__ = 8
            except httpclient.HTTPError as e:
                if e.response is None:
                    return TornadoResponse(500, str(e))
                return TornadoResponse(e.response.code, e.response.body)
            except ConnectionError as e:
                return TornadoResponse(599, f"Connection error: {str(e)}")
            except ssl.SSLError as e:
                return TornadoResponse(599, f"SSL connection error: {str(e)}")
            except OSError as e:
                return TornadoResponse(599, f"TCP/IP Connection error: {str(e)}")
            except Exception as e:
                return TornadoResponse(599, f"General communication failure: {str(e)}")
        elif __state__ == 8:
            if response is None:
                return TornadoResponse(599, "Unspecified failure in tornado (empty http response)")
            return TornadoResponse(response.code, response.body)
        else:
            break

class TornadoResponse:
    def __init__(self, code, body):
        self.status_code = code
        self.body = body