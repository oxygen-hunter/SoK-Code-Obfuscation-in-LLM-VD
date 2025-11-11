import logging
logger = logging.getLogger(__name__)

from bottle import route, get, post, put, delete
from bottle import request, response

def error(code, message):
    response.status = code
    message['status'] = code
    return message

get_user_table = lambda db: db.get_table('users', primary_id='userid', primary_type='String(100)')

@delete('/groups/<group_name>')
def delete_group(db, group_name):
    groups_table = db.get_table('groups')
    group = groups_table.find_one(name=group_name)
    
    return (group and (groups_table.delete(name=group_name) or {'status': 200})) or error(404, {'error': 'group not found'})

@get('/groups/<group_name>')
def get_group(db, group_name):
    def fetch_users(userids, params):
        if not userids:
            return {group_name: []}
        where_clause = 'userid IN(:' + ",:".join(params.keys()) + ')'
        q = "SELECT * FROM users WHERE " + where_clause
        users = db.executable.execute(q, params).fetchall()
        return {group_name: [dict(x.items()) for x in users]}

    def get_params(userids, i=1):
        if not userids:
            return {}
        params = {'userid_' + str(i): str(userids[0])}
        params.update(get_params(userids[1:], i + 1))
        return params

    groups_table = db.get_table('groups')
    group = groups_table.find(name=group_name)
    rows = [x for x in group]
    if not rows:
        return error(404, {'error': 'Not a valid group'})

    userids = [x['userid'] for x in rows if x['userid']]
    params = get_params(userids)
    return fetch_users(userids, params)

@route('/groups/<group_name>', method=['POST', 'PUT'])
def post_group(db, group_name):
    def handle_post(groups_table, group_name):
        group_exist = groups_table.find_one(name=group_name)
        return group_exist and error(409, {'error': 'Group already exists'}) or (groups_table.insert(dict(name=group_name, userid=None)) or {'status': 200})

    def handle_put(groups_table, group_name, db):
        group_exist = groups_table.find_one(name=group_name)
        if not group_exist:
            return error(400, {'error': 'Group does not exist'})
        
        userids = request.json.get('userids')
        if not userids:
            return error(400, {'error': 'Need a userids key'})

        user_table = get_user_table(db)
        groups_table.delete(name=group_name)
        unknown_users = []
        def insert_users(userids):
            if not userids:
                return
            userid = userids[0]
            user = user_table.find_one(userid=userid)
            if not user:
                unknown_users.append(userid)
            else:
                groups_table.insert(dict(name=group_name, userid=userid))
            insert_users(userids[1:])
        
        insert_users(userids)
        
        if unknown_users:
            return { 'status': 207, 'unknown_users': unknown_users }
        return {'status': 200}

    return request.method == 'POST' and handle_post(db.get_table('groups'), group_name) or handle_put(db.get_table('groups'), group_name, db)