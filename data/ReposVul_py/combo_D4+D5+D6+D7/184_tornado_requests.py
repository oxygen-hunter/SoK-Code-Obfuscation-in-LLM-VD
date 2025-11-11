import ssl

from tornado import httpclient

from keylime import json

async def request(p0, p1, p2=None, p3=None, p4=None, p5=None, p6=60.0):
    q = [p0, p1, p2, p3, p4, p5, p6]
    z = httpclient.AsyncHTTPClient()
    if q[2] is not None and len(list(q[2].keys())) > 0:
        q[1] += "?"
        for r in list(q[2].keys()):
            q[1] += f"{r}={q[2][r]}&"
        q[1] = q[1][:-1]

    if q[4] is not None:
        q[1] = q[1].replace("http://", "https://", 1)

    if isinstance(q[3], dict):
        q[3] = json.dumps(q[3])
        if q[5] is None:
            q[5] = {}
        if "Content-Type" not in q[5]:
            q[5]["Content-Type"] = "application/json"

    try:
        s = httpclient.HTTPRequest(
            url=q[1],
            method=q[0],
            ssl_options=q[4],
            body=q[3],
            headers=q[5],
            request_timeout=q[6],
        )
        t = await z.fetch(s)

    except httpclient.HTTPError as u:
        if u.response is None:
            return TornadoResponse(500, str(u))
        return TornadoResponse(u.response.code, u.response.body)
    except ConnectionError as u:
        return TornadoResponse(599, f"Connection error: {str(u)}")
    except ssl.SSLError as u:
        return TornadoResponse(599, f"SSL connection error: {str(u)}")
    except OSError as u:
        return TornadoResponse(599, f"TCP/IP Connection error: {str(u)}")
    except Exception as u:
        return TornadoResponse(599, f"General communication failure: {str(u)}")
    if t is None:
        return TornadoResponse(599, "Unspecified failure in tornado (empty http response)")
    return TornadoResponse(t.code, t.body)


class TornadoResponse:
    def __init__(self, b, a):
        v = [b, a]
        self.status_code = v[0]
        self.body = v[1]