#encoding=utf-8
# test for flask

import MySQLdb
import time
from flask import Flask
from flask import render_template
from flask import request, jsonify


COLOR_CHART = ["#F7464A","#46BFBD","#FDB45C","#949FB1","#C7604C",\
				"#4D5360","#7D4F6D","#9D9B7F","#21323D","#1874CD",\
				"#218868","#8E8E38"]
conn=MySQLdb.connect(host="localhost",user="root",passwd="",db="db_vote_web",charset="utf8")
app = Flask(__name__)

def parse_req_recursive(i, n, l_dsc):
	if i < n:
		l_dsc.append(request.form["opt"+str(i)])
		return parse_req_recursive(i + 1, n, l_dsc)
	return l_dsc

def parse_req():
	title = request.form["title"]
	n = len(request.form)-1
	l_dsc = parse_req_recursive(1, n, [])
	return title, n-1, l_dsc

@app.route('/')
def hello_world():
    return render_template("index.html")

@app.route('/error')
def error():
    return render_template("error.html")

@app.route('/create', methods=['POST'])
def create_poll():
	try:
		cursor = conn.cursor()  
		uid = request.remote_addr
		vid = str(int(time.time()*100))
		title, optn, l_dsc = parse_req()
		optdsc = '|'.join(l_dsc)
		optnum = '|'.join(['0']*optn)
		sql = "insert into t_vote_info(FUid, FVoteId, FTitle, FOptionNum, \
				FOptionDesc, FOptionVoteNum, FState, FCreateTime, FEndTime) \
				values(%s,%s,%s,%s,%s,%s,0,now(),now()+interval 1 day);" 
		param = (uid, vid, title, optn, optdsc, optnum) 
		res = cursor.execute(sql, param)
		conn.commit()
		cursor.close()
	except Exception as e:
		return jsonify({"return_code":21, "return_msg":str(e), "p_id":0})
	return jsonify({"p_id":vid})

@app.route('/poll', methods=['POST','GET'])
def do_poll():
	def fetch_data():
		cursor = conn.cursor()
		sql_s = "select FTitle, FOptionDesc from t_vote_info where FVoteId=%s;"
		res = cursor.execute(sql_s, (p_id,))
		r = cursor.fetchone()
		cursor.close()
		return r[0], r[1].split('|')

	if "p_id" in request.args:
		p_id = request.args['p_id']
		title, opts_desc = fetch_data()
		return render_template("poll.html", title=title, opts=opts_desc)

	if "p_id" not in request.form or "opt_idx" not in request.form:
		return render_template("poll.html")

	o_id = int(request.form['opt_idx'])-1
	p_id = request.form['p_id']
	try:
		cursor = conn.cursor()
		sql_s = "select FOptionVoteNum from t_vote_info where FVoteId=%s;"
		res = cursor.execute(sql_s, (p_id,))
		opt_pre = cursor.fetchone()[0].split('|')
		opt_pre[o_id] = str(int(opt_pre[o_id])+1)
		opt_new = '|'.join(opt_pre)
		sql_u = "update t_vote_info set FOptionVoteNum=%s where FVoteId=%s;"
		res = cursor.execute(sql_u, (opt_new,p_id))
		conn.commit()
		cursor.close()
	except Exception as e:
		cursor.close()
		return jsonify({"result_code":"-1", "result_msg":"error", "p_id":0})
	return jsonify({"result_code":"0", "result_msg":"success", "p_id":p_id})

@app.route('/show')
def show_poll():
	def assemble_rows(i, n, opts_desc, opts_num, opts_col, rows):
		if i < n:
			rows.append([opts_desc[i], opts_num[i], opts_col[i]])
			return assemble_rows(i + 1, n, opts_desc, opts_num, opts_col, rows)
		return rows

	title = "error"
	if "p_id" not in request.args:
		return render_template("show.html", title=title)
	p_id = request.args['p_id']
	rows = []
	try:
		cursor = conn.cursor()
		sql_s = "select FTitle,FOptionDesc,FOptionVoteNum,FState,FEndTime from t_vote_info where FVoteId=%s;"
		res = cursor.execute(sql_s, (p_id,))
		r = cursor.fetchone()
		cursor.close()
		title = r[0]
		opts_desc = r[1].split('|')
		opts_num = r[2].split('|')
		opts_col = COLOR_CHART[:len(opts_desc)]
		rows = assemble_rows(0, len(opts_desc), opts_desc, opts_num, opts_col, [])
	except Exception as e:
		return render_template("show.html", title=title)
	return render_template("show.html", title=title, opts=rows)

@app.route('/refresh', methods=['POST'])
def show_refresh():
	def assemble_json_rows(i, n, opts_desc, opts_num, opts_col, rows):
		if i < n:
			rows.append({"label":opts_desc[i], "value":int(opts_num[i]), "color":opts_col[i]})
			return assemble_json_rows(i + 1, n, opts_desc, opts_num, opts_col, rows)
		return rows

	if "p_id" not in request.form:
		return jsonify({"result_code":"-1", "result_msg":"refresh error"})
	p_id = request.form['p_id']
	rows = []
	try:
		cursor = conn.cursor()
		sql_s = "select FTitle,FOptionDesc,FOptionVoteNum,FState,FEndTime from t_vote_info where FVoteId=%s;"
		res = cursor.execute(sql_s, (p_id,))
		r = cursor.fetchone()
		cursor.close()
		title = r[0]
		opts_desc = r[1].split('|')
		opts_num = r[2].split('|')
		opts_col = COLOR_CHART[:len(opts_desc)]
		rows = assemble_json_rows(0, len(opts_desc), opts_desc, opts_num, opts_col, [])
		return jsonify({"result_code":"0", "result_msg":"success", "rows":rows})
	except Exception as e:
		return jsonify({"result_code":"-1", "result_msg":"refresh error"})


if __name__ == '__main__':
    app.debug = True
    app.run()
    conn.close()