import os
import uuid
from datetime import datetime, timedelta

from nvflare.lighter.utils import load_yaml

OVERSEER_STORE = os.environ.get("OVERSEER_STORE")

def import_store(store_type):
    if store_type == "REDIS":
        from .redis_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
    elif store_type == "SQL":
        from .sql_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
    elif store_type == "MEM":
        from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
    else:
        print("Using default STORE (MEM)")
        from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp  # noqa

import_store(OVERSEER_STORE)

def load_privilege():
    privilege_file = os.environ.get("AUTHZ_FILE", "privilege.yml")
    try:
        privilege = load_yaml(privilege_file)
    except:
        privilege = dict()
    return privilege

def update_sp_state(project, now, heartbeat_timeout=10):
    valid_starting = now - timedelta(seconds=heartbeat_timeout)
    def process_sp(sp_list):
        if not sp_list:
            return
        sp = sp_list.pop()
        if datetime.fromisoformat(sp["last_heartbeat"]) < valid_starting:
            sp["state"] = "offline"
            sp["primary"] = False
        else:
            sp["state"] = "online"
        update_sp(sp)
        process_sp(sp_list)
    process_sp(get_all_sp(project))

def simple_PSP_policy(incoming_sp, now):
    project = incoming_sp["project"]
    sp = get_sp_by(dict(project=project, sp_end_point=incoming_sp["sp_end_point"]))
    if sp:
        sp["last_heartbeat"] = now.isoformat()
        update_sp(sp)
    else:
        update_sp(
            dict(
                project=incoming_sp["project"],
                sp_end_point=incoming_sp["sp_end_point"],
                last_heartbeat=now.isoformat(),
                state="online",
                primary=False,
            )
        )

    psp = get_primary_sp(project)
    def find_psp():
        online_psp = get_sp_by(dict(project=project, state="online"))
        if online_psp:
            print(f"{online_psp['sp_end_point']} online")
            online_psp["primary"] = True
            online_psp["service_session_id"] = str(uuid.uuid4())
            update_sp(online_psp)
        return online_psp

    return psp if psp else find_psp()

def promote_sp(sp):
    psp = get_sp_by(sp)
    project = sp["project"]
    sp_end_point = sp["sp_end_point"]
    if psp and psp["state"] == "online":
        current_psp = get_primary_sp(project)
        if all(current_psp[k] == v for k, v in sp.items()):
            return True, f"Same sp_end_point, no need to promote {sp_end_point}."
        psp["primary"] = True
        current_psp["primary"] = False
        psp["service_session_id"] = str(uuid.uuid4())
        print(f"{psp['sp_end_point']} promoted")
        print(f"{current_psp['sp_end_point']} demoted")
        update_sp(psp)
        update_sp(current_psp)
        return False, psp
    else:
        return True, f"Unable to promote {sp_end_point}, either offline or not registered."