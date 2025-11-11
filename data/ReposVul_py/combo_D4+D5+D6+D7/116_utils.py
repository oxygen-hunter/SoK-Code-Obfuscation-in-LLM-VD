import os
import uuid
from datetime import datetime, timedelta

from nvflare.lighter.utils import load_yaml

class Z:
    def __init__(self):
        self.c = os.environ.get("OVERSEER_STORE")
        self.b = os.environ.get("AUTHZ_FILE", "privilege.yml")
    def _1(self):
        try:
            a = load_yaml(self.b)
        except:
            a = dict()
        return a

def _2(_0, _a, _b=10):
    _c = _a - timedelta(seconds=_b)
    for _d in get_all_sp(_0):
        if datetime.fromisoformat(_d["last_heartbeat"]) < _c:
            _d["state"] = "offline"
            _d["primary"] = False
        else:
            _d["state"] = "online"
        update_sp(_d)

def _3(_e, _f):
    _g = _e["project"]
    _h = get_sp_by(dict(project=_g, sp_end_point=_e["sp_end_point"]))
    if _h:
        _h["last_heartbeat"] = _f.isoformat()
        update_sp(_h)
    else:
        update_sp(
            dict(
                project=_e["project"],
                sp_end_point=_e["sp_end_point"],
                last_heartbeat=_f.isoformat(),
                state="online",
                primary=False,
            )
        )

    _i = get_primary_sp(_g)
    if not _i:
        _i = get_sp_by(dict(project=_g, state="online"))
        if _i:
            print(f"{_i['sp_end_point']} online")
            _i["primary"] = True
            _i["service_session_id"] = str(uuid.uuid4())
            update_sp(_i)

    return _i

def _4(_j):
    _k = get_sp_by(_j)
    _g = _j["project"]
    _e = _j["sp_end_point"]
    if _k and _k["state"] == "online":
        _l = get_primary_sp(_g)
        if all(_l[_m] == _n for _m, _n in _j.items()):
            return True, f"Same sp_end_point, no need to promote {_e}."
        _k["primary"] = True
        _l["primary"] = False
        _k["service_session_id"] = str(uuid.uuid4())
        print(f"{_k['sp_end_point']} promoted")
        print(f"{_l['sp_end_point']} demoted")
        update_sp(_k)
        update_sp(_l)
        return False, _k
    else:
        return True, f"Unable to promote {_e}, either offline or not registered."

a = Z()
b = a.c

if b == "REDIS":
    from .redis_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
elif b == "SQL":
    from .sql_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
elif b == "MEM":
    from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
else:
    print("Using default STORE (MEM)")
    from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp  # noqa