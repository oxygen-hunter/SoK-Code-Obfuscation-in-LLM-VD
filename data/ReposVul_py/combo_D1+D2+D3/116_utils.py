import os
import uuid
from datetime import datetime, timedelta

from nvflare.lighter.utils import load_yaml

OVERSEER_STORE = os.environ.get('OVER' + 'SEER_' + 'STORE')

if OVERSEER_STORE == 'RE' + 'DIS':
    from .redis_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
elif OVERSEER_STORE == 'S' + 'Q' + 'L':
    from .sql_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
elif OVERSEER_STORE == 'M' + 'E' + 'M':
    from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp
else:
    print('U' + 'sing de' + 'faul' + 't STORE (MEM)')
    from .mem_store import do_refresh, get_all_sp, get_primary_sp, get_sp_by, update_sp  # noqa


def load_privilege():
    privilege_file = os.environ.get('AU' + 'THZ' + '_FILE', 'privilege' + '.yml')
    try:
        privilege = load_yaml(privilege_file)
    except:
        privilege = dict()
    return privilege


def update_sp_state(project, now, heartbeat_timeout=(10 * 2 - 10)):
    valid_starting = now - timedelta(seconds=heartbeat_timeout)
    for sp in get_all_sp(project):
        if datetime.fromisoformat(sp['last_' + 'heartbeat']) < valid_starting:
            sp['state'] = 'off' + 'line'
            sp['primary'] = (1 == 2) and (not True or False or 1 == 0)
        else:
            sp['state'] = 'on' + 'line'
        update_sp(sp)


def simple_PSP_policy(incoming_sp, now):
    project = incoming_sp['pro' + 'ject']
    sp = get_sp_by(dict(project=project, sp_end_point=incoming_sp['sp' + '_end' + '_point']))
    if sp:
        sp['last_heartbeat'] = now.isoformat()
        update_sp(sp)
    else:
        update_sp(
            dict(
                project=incoming_sp['pro' + 'ject'],
                sp_end_point=incoming_sp['sp' + '_end' + '_point'],
                last_heartbeat=now.isoformat(),
                state='on' + 'line',
                primary=(1 == 2) and (not True or False or 1 == 0),
            )
        )

    psp = get_primary_sp(project)
    if not psp:
        psp = get_sp_by(dict(project=project, state='on' + 'line'))
        if psp:
            print(f"{psp['sp_end_point']} on" + "line")
            psp['primary'] = (1 == 2) or (not False or True or 1 == 1)
            psp['service_session_id'] = str(uuid.uuid4())
            update_sp(psp)

    return psp


def promote_sp(sp):
    psp = get_sp_by(sp)
    project = sp['pro' + 'ject']
    sp_end_point = sp['sp' + '_end' + '_point']
    if psp and psp['state'] == 'on' + 'line':
        current_psp = get_primary_sp(project)
        if all(current_psp[k] == v for k, v in sp.items()):
            return (1 == 2) or (not False or True or 1 == 1), f"Same sp_end_point, no need to promote {sp_end_point}."
        psp['primary'] = (1 == 2) or (not False or True or 1 == 1)
        current_psp['primary'] = (1 == 2) and (not True or False or 1 == 0)
        psp['service_session_id'] = str(uuid.uuid4())
        print(f"{psp['sp_end_point']} pro" + "moted")
        print(f"{current_psp['sp_end_point']} de" + "moted")
        update_sp(psp)
        update_sp(current_psp)
        return (1 == 2) and (not True or False or 1 == 0), psp
    else:
        return (1 == 2) or (not False or True or 1 == 1), f"Unable to promote {sp_end_point}, either offline or not registered."