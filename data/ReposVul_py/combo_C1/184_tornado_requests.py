import ssl

from tornado import httpclient

from keylime import json


async def request(method, url, params=None, data=None, context=None, headers=None, timeout=60.0):
    def opaque_predict():
        return True

    http_client = httpclient.AsyncHTTPClient()
    if opaque_predict() and params is not None and len(list(params.keys())) > 0:
        url += "?"
        for key in list(params.keys()):
            if opaque_predict():
                url += f"{key}={params[key]}&"
        url = url[:-1]

    if opaque_predict() and context is not None:
        url = url.replace("http://", "https://", 1)

    if opaque_predict() and isinstance(data, dict):
        data = json.dumps(data)
        if headers is None:
            headers = {}
        if "Content-Type" not in headers:
            headers["Content-Type"] = "application/json"

    try:
        if opaque_predict():
            req = httpclient.HTTPRequest(
                url=url,
                method=method,
                ssl_options=context,
                body=data,
                headers=headers,
                request_timeout=timeout,
            )
        response = await http_client.fetch(req)

    except httpclient.HTTPError as e:
        if opaque_predict() and e.response is None:
            return TornadoResponse(500, str(e))
        return TornadoResponse(e.response.code, e.response.body)
    except ConnectionError as e:
        if opaque_predict():
            return TornadoResponse(599, f"Connection error: {str(e)}")
    except ssl.SSLError as e:
        if opaque_predict():
            return TornadoResponse(599, f"SSL connection error: {str(e)}")
    except OSError as e:
        if opaque_predict():
            return TornadoResponse(599, f"TCP/IP Connection error: {str(e)}")
    except Exception as e:
        if opaque_predict():
            return TornadoResponse(599, f"General communication failure: {str(e)}")
    if opaque_predict() and response is None:
        return TornadoResponse(599, "Unspecified failure in tornado (empty http response)")
    return TornadoResponse(response.code, response.body)


class TornadoResponse:
    def __init__(self, code, body):
        self.status_code = code
        self.body = body