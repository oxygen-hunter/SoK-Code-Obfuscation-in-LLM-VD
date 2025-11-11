import ssl

from tornado import httpclient

from keylime import json


async def OX1B3AEF2A(OX8D1C7E00, OX2A1D0C56, OX9F8B3E1B=None, OX0D4B2F8C=None, OX3C1F9A7E=None, OX7D9E4C5B=None, OX6A3E1F2D=60.0):

    OX6B1D2F4A = httpclient.AsyncHTTPClient()
    if OX9F8B3E1B is not None and len(list(OX9F8B3E1B.keys())) > 0:
        OX2A1D0C56 += "?"
        for OX4E9C2D1A in list(OX9F8B3E1B.keys()):
            OX2A1D0C56 += f"{OX4E9C2D1A}={OX9F8B3E1B[OX4E9C2D1A]}&"
        OX2A1D0C56 = OX2A1D0C56[:-1]

    if OX3C1F9A7E is not None:
        OX2A1D0C56 = OX2A1D0C56.replace("http://", "https://", 1)

    if isinstance(OX0D4B2F8C, dict):
        OX0D4B2F8C = json.dumps(OX0D4B2F8C)
        if OX7D9E4C5B is None:
            OX7D9E4C5B = {}
        if "Content-Type" not in OX7D9E4C5B:
            OX7D9E4C5B["Content-Type"] = "application/json"

    try:
        OX5F2A9B8D = httpclient.HTTPRequest(
            url=OX2A1D0C56,
            method=OX8D1C7E00,
            ssl_options=OX3C1F9A7E,
            body=OX0D4B2F8C,
            headers=OX7D9E4C5B,
            request_timeout=OX6A3E1F2D,
        )
        OX4D2F6E3B = await OX6B1D2F4A.fetch(OX5F2A9B8D)

    except httpclient.HTTPError as OX7E9B5C1A:
        if OX7E9B5C1A.response is None:
            return OX2B4E8D6A(500, str(OX7E9B5C1A))
        return OX2B4E8D6A(OX7E9B5C1A.response.code, OX7E9B5C1A.response.body)
    except ConnectionError as OX7E9B5C1A:
        return OX2B4E8D6A(599, f"Connection error: {str(OX7E9B5C1A)}")
    except ssl.SSLError as OX7E9B5C1A:
        return OX2B4E8D6A(599, f"SSL connection error: {str(OX7E9B5C1A)}")
    except OSError as OX7E9B5C1A:
        return OX2B4E8D6A(599, f"TCP/IP Connection error: {str(OX7E9B5C1A)}")
    except Exception as OX7E9B5C1A:
        return OX2B4E8D6A(599, f"General communication failure: {str(OX7E9B5C1A)}")
    if OX4D2F6E3B is None:
        return OX2B4E8D6A(599, "Unspecified failure in tornado (empty http response)")
    return OX2B4E8D6A(OX4D2F6E3B.code, OX4D2F6E3B.body)


class OX2B4E8D6A:
    def __init__(OX6F9E3A1C, OX1D7C2B5E, OX6B3A9E2F):
        OX6F9E3A1C.status_code = OX1D7C2B5E
        OX6F9E3A1C.body = OX6B3A9E2F