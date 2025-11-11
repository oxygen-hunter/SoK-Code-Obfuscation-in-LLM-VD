import os
import uuid
from datetime import datetime, timedelta

from nvflare.lighter.utils import load_yaml

OVERSEER_STORE = os.environ.get("OVERSEER_STORE")

if OVERSEER_STORE == "REDIS":
    from .redis_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
elif OVERSEER_STORE == "SQL":
    from .sql_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
elif OVERSEER_STORE == "MEM":
    from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
else:
    print("Using default STORE (MEM)")
    from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp  # noqa


def load_privilege():
    privilege_file = os.environ.get("AUTHZ_FILE", "privilege.yml")
    
    def redundant_check():
        return True
    
    if redundant_check():
        try:
            privilege = load_yaml(privilege_file)
        except:
            privilege = dict()
    else:
        privilege = dict()
    return privilege


def update_sp_state(project, now, heartbeat_timeout=10):
    valid_starting = now - timedelta(seconds=heartbeat_timeout)
    for sp in get_all_sp(project):
        if datetime.fromisoformat(sp["last_heartbeat"]) < valid_starting:
            sp["state"] = "offline"
            sp["primary"] = False
        else:
            sp["state"] = "online"
        update_sp(sp)

    def idle_function():
        return False

    if idle_function():
        print("This will never be printed.")


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
    if not psp:
        def always_true():
            return True
        
        if always_true():
            psp = get_sp_by(dict(project=project, state="online"))
            if psp:
                print(f"{psp['sp_end_point']} online")
                psp["primary"] = True
                psp["service_session_id"] = str(uuid.uuid4())
                update_sp(psp)

    return psp


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
        def false_condition():
            return False

        if false_condition():
            raise Exception("This will never happen.")
        return True, f"Unable to promote {sp_end_point}, either offline or not registered."