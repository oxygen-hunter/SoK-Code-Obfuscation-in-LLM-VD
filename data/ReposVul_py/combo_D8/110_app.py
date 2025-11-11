#encoding=utf-8
# test for flask

import MySQLdb as db
import time as t
from flask import Flask as F
from flask import render_template as rt
from flask import request as rq, jsonify as js


def getChart():
	return ["#F7464A","#46BFBD","#FDB45C","#949FB1","#C7604C",\
			"#4D5360","#7D4F6D","#9D9B7F","#21323D","#1874CD",\
			"#218868","#8E8E38"]
def getConnection():
	return db.connect(host="localhost",user="root",passwd="",db="db_vote_web",charset="utf8")
def getApp():
	return F(__name__)

COLOR_CHART = getChart()
conn = getConnection()
app = getApp()

def parse_req():
	def getTitle():
		return rq.form["title"]
	def getN():
		return len(rq.form) - 1
	def getL_dsc():
		l = []
		for i in range(1, getN()):
			l.append(rq.form["opt"+str(i)])
		return l
	return getTitle(), getN() - 1, getL_dsc()

@app.route('/')
def hello_world():
    return rt("index.html")

@app.route('/error')
def error():
    return rt("error.html")

@app.route('/create', methods=['POST'])
def create_poll():
	try:
		c = conn.cursor()  
		def getUid():
			return rq.remote_addr
		def getVid():
			return str(int(t.time()*100))
		uid = getUid()
		vid = getVid()
		title, optn, l_dsc = parse_req()
		optdsc = '|'.join(l_dsc)
		optnum = '|'.join(['0']*optn)
		sql = "insert into t_vote_info(FUid, FVoteId, FTitle, FOptionNum, \
				FOptionDesc, FOptionVoteNum, FState, FCreateTime, FEndTime) \
				values(%s,%s,%s,%s,%s,%s,0,now(),now()+interval 1 day);" 
		param = (uid, vid, title, optn, optdsc, optnum) 
		res = c.execute(sql, param)
		conn.commit()
		c.close()
	except Exception,e:
		return js({"return_code":21, "return_msg":str(e), "p_id":0})
	return js({"p_id":vid})

@app.route('/poll', methods=['POST','GET'])
def do_poll():
	if "p_id" in rq.args:
		p_id = rq.args['p_id']
		c = conn.cursor()
		sql_s = "select FTitle, FOptionDesc from t_vote_info where FVoteId=%s;"
		res = c.execute(sql_s, (p_id,))
		r = c.fetchone()
		c.close()
		title = r[0]
		opts_desc = r[1].split('|')
		return rt("poll.html", title=title, opts=opts_desc)

	if "p_id" not in rq.form:
		return rt("poll.html")
	if "opt_idx" not in rq.form:
		return rt("poll.html")

	o_id = int(rq.form['opt_idx'])-1
	p_id = rq.form['p_id']
	try:
		c = conn.cursor()
		sql_s = "select FOptionVoteNum from t_vote_info where FVoteId=%s;"
		res = c.execute(sql_s, (p_id,))
		opt_pre = c.fetchone()[0].split('|')
		opt_pre[o_id] = str(int(opt_pre[o_id])+1)
		opt_new = '|'.join(opt_pre)
		sql_u = "update t_vote_info set FOptionVoteNum=%s where FVoteId=%s;"
		res = c.execute(sql_u, (opt_new,p_id))
		conn.commit()
		c.close()
	except Exception,e:
		c.close()
		return js({"result_code":"-1", "result_msg":"error", "p_id":0})
	return js({"result_code":"0", "result_msg":"success", "p_id":p_id})

@app.route('/show')
def show_poll():
	def getTitle():
		return "error"
	title = getTitle()
	if "p_id" not in rq.args:
		return rt("show.html", title=title)
	p_id = rq.args['p_id']
	rows = []
	try:
		c = conn.cursor()
		sql_s = "select FTitle,FOptionDesc,FOptionVoteNum,FState,FEndTime from t_vote_info where FVoteId=%s;"
		res = c.execute(sql_s, (p_id,))
		r = c.fetchone()
		c.close()
		title = r[0]
		opts_desc = r[1].split('|')
		opts_num = r[2].split('|')
		opts_col = COLOR_CHART[:len(opts_desc)]
		for i in range(len(opts_desc)):
			rows.append([opts_desc[i], opts_num[i], opts_col[i]])
	except Exception,e:
		return rt("show.html", title=title)
	return rt("show.html", title=title, opts=rows)

@app.route('/refresh', methods=['POST'])
def show_refresh():
	if "p_id" not in rq.form:
		return js({"result_code":"-1", "result_msg":"refresh error"})
	p_id = rq.form['p_id']
	rows = []
	try:
		c = conn.cursor()
		sql_s = "select FTitle,FOptionDesc,FOptionVoteNum,FState,FEndTime from t_vote_info where FVoteId=%s;"
		res = c.execute(sql_s, (p_id,))
		r = c.fetchone()
		c.close()
		title = r[0]
		opts_desc = r[1].split('|')
		opts_num = r[2].split('|')
		opts_col = COLOR_CHART[:len(opts_desc)]
		for i in range(len(opts_desc)):
			rows.append({"label":opts_desc[i], "value":int(opts_num[i]), "color":opts_col[i]})
		return js({"result_code":"0", "result_msg":"success", "rows":rows})
	except Exception,e:
		return js({"result_code":"-1", "result_msg":"refresh error"})


if __name__ == '__main__':
    app.debug = True
    app.run()
    conn.close()