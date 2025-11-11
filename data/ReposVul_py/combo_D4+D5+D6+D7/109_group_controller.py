import logging
logger = logging.getLogger(__name__)

from bottle import route, get, post, put, delete
from bottle import request, response

def error(code, message):
    response.status = code
    message['status'] = code
    return message

def _get_user_table(db):
    return db.get_table('users', primary_id='userid', primary_type='String(100)')

get_user_table = lambda db: _get_user_table(db)

@delete('/groups/<group_name>')
def delete_group(db, group_name):
    data = [db.get_table('groups'), group_name, None]
    data[2] = data[0].find_one(name=data[1])
    if not data[2]:
        return error(404, {'error': 'group not found'})
    else:
        data[0].delete(name=data[1])
        return {'status': 200}

@get('/groups/<group_name>')
def get_group(db, group_name):
    data = [None, None, None, None]
    data[0] = db.get_table('groups')
    data[1] = data[0].find(name=group_name)
    data[2] = [x for x in data[1]]
    if not data[2]:
        return error(404, {'error': 'Not a valid group'})

    userids = [x['userid'] for x in data[2] if x['userid']]
    if not userids:
        return {group_name: []}

    params = {}
    for i, userid in enumerate(userids,1):
        params['userid_' + str(i)] = str(userid)
    where_clause = 'userid IN(:' + ",:".join(params.keys()) + ')'
    q = "SELECT * FROM users WHERE " + where_clause
    data[3] = db.executable.execute(q, params).fetchall()
    ret = {group_name: [dict(x.items()) for x in data[3]] }
    return ret

@route('/groups/<group_name>', method=['POST', 'PUT'])
def post_group(db, group_name):
    data = [None, None, None, None]
    data[0] = db.get_table('groups')
    data[1] = data[0].find_one(name=group_name)
    if request.method=='POST':
        if data[1]:
            return error(409, {'error': 'Group already exists'})
        else:
            data[0].insert(dict(name=group_name, userid=None))
            return {'status': 200}

    elif request.method == 'PUT':
        if not data[1]:
            return error(400, {'error': 'Group does not exist'})
        else:
            userids = request.json.get('userids')
            if not userids:
                return error(400, {'error': 'Need a userids key'})

            user_table = get_user_table(db)
            data[0].delete(name=group_name)
            unknown_users = []
            for userid in userids:
                user = user_table.find_one(userid=userid)
                if not user:
                    unknown_users.append(userid)
                else:
                    data[0].insert(dict(name=group_name, userid=userid))

            ret = {'status': 200}
            if unknown_users:
                ret = { 'status': 207, 'unknown_users': unknown_users }
            return ret