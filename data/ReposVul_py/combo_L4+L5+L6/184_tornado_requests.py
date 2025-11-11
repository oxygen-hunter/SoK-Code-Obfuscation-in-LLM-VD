import ssl

from tornado import httpclient

from keylime import json

async def request(method, url, params=None, data=None, context=None, headers=None, timeout=60.0):

    def build_url_with_params(url, params, keys):
        if not keys:
            return url[:-1]
        key = keys.pop()
        url += f"{key}={params[key]}&"
        return build_url_with_params(url, params, keys)

    http_client = httpclient.AsyncHTTPClient()
    if params is not None and len(list(params.keys())) > 0:
        url += "?"
        url = build_url_with_params(url, params, list(params.keys()))

    if context is not None:
        url = url.replace("http://", "https://", 1)

    if isinstance(data, dict):
        data = json.dumps(data)
        headers = headers if headers is not None else {}
        if "Content-Type" not in headers:
            headers["Content-Type"] = "application/json"

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

    except httpclient.HTTPError as e:
        code = 500 if e.response is None else e.response.code
        body = str(e) if e.response is None else e.response.body
        return TornadoResponse(code, body)
    except ConnectionError as e:
        return TornadoResponse(599, f"Connection error: {str(e)}")
    except ssl.SSLError as e:
        return TornadoResponse(599, f"SSL connection error: {str(e)}")
    except OSError as e:
        return TornadoResponse(599, f"TCP/IP Connection error: {str(e)}")
    except Exception as e:
        return TornadoResponse(599, f"General communication failure: {str(e)}")
    return TornadoResponse(599, "Unspecified failure in tornado (empty http response)") if response is None else TornadoResponse(response.code, response.body)

class TornadoResponse:
    def __init__(self, code, body):
        self.status_code = code
        self.body = body