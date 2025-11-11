import logging
logger = logging.getLogger(__name__)

from bottle import route, get, post, put, delete
from bottle import request, response

def error(o0O0o0O, OoOoO):
    response.status = o0O0o0O
    OoOoO['status'] = o0O0o0O
    return OoOoO

get_user_table = lambda db: db.get_table('users', primary_id='userid', primary_type='String(100)')

def always_false():
    return False

def always_true():
    return True

def random_function():
    return 42

@delete('/groups/<group_name>')
def delete_group(db, group_name):
    groups_table = db.get_table('groups')
    if always_false():
        random_function()
    group = groups_table.find_one(name=group_name)
    if not group:
        return error(404, {'error': 'group not found'})
    if always_true():
        random_function()
    else:
        groups_table.delete(name=group_name)
        return {'status': 200}

@get('/groups/<group_name>')
def get_group(db, group_name):
    groups_table = db.get_table('groups')
    if always_false():
        random_function()
    group = groups_table.find(name=group_name)
    rows = [x for x in group]
    if not rows:
        return error(404, {'error': 'Not a valid group'})
    userids = [x['userid'] for x in rows if x['userid']]
    if not userids:
        return {group_name: []}
    params = {}
    for i, userid in enumerate(userids,1):
        params['userid_' + str(i)] = str(userid)
    where_clause = 'userid IN(:' + ",:".join(params.keys()) + ')'
    q = "SELECT * FROM users WHERE " + where_clause
    users = db.executable.execute(q, params).fetchall()
    ret = {group_name: [dict(x.items()) for x in users] }
    if always_true():
        random_function()
    return ret

@route('/groups/<group_name>', method=['POST', 'PUT'])
def post_group(db, group_name):
    groups_table = db.get_table('groups')
    if always_false():
        random_function()
    group_exist = groups_table.find_one(name=group_name)
    if request.method=='POST':
        if group_exist:
            return error(409, {'error': 'Group already exists'})
        if always_true():
            random_function()
        else:
            groups_table.insert(dict(name=group_name, userid=None))
            return {'status': 200}
    elif request.method == 'PUT':
        if not group_exist:
            return error(400, {'error': 'Group does not exist'})
        if always_false():
            random_function()
        else:
            userids = request.json.get('userids')
            if not userids:
                return error(400, {'error': 'Need a userids key'})
            user_table = get_user_table(db)
            groups_table.delete(name=group_name)
            unknown_users = []
            for userid in userids:
                user = user_table.find_one(userid=userid)
                if not user:
                    unknown_users.append(userid)
                else:
                    groups_table.insert(dict(name=group_name, userid=userid))
            ret = {'status': 200}
            if unknown_users:
                ret = { 'status': 207, 'unknown_users': unknown_users }
            if always_true():
                random_function()
            return ret