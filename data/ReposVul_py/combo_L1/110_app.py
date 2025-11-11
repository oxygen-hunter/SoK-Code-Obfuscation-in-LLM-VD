#encoding=utf-8

import MySQLdb
import time
from flask import Flask
from flask import render_template
from flask import request, jsonify

OX512E6E = ["#F7464A","#46BFBD","#FDB45C","#949FB1","#C7604C",\
				"#4D5360","#7D4F6D","#9D9B7F","#21323D","#1874CD",\
				"#218868","#8E8E38"]
OX1E0F0C = MySQLdb.connect(host="localhost",user="root",passwd="",db="db_vote_web",charset="utf8")
OX4A6E3E = Flask(__name__)

def OX7B4DF339():
	OX4C8B7E = request.form["title"]
	OX1A26B0 = len(request.form)-1
	OX6B3F1D = []
	for OX3F8B2D in range(1,OX1A26B0):
		OX6B3F1D.append(request.form["opt"+str(OX3F8B2D)])
	return OX4C8B7E, OX1A26B0-1, OX6B3F1D

@OX4A6E3E.route('/')
def OX6F9C4E():
    return render_template("index.html")

@OX4A6E3E.route('/error')
def OX5D7A4F():
    return render_template("error.html")

@OX4A6E3E.route('/create', methods=['POST'])
def OX1D9E3A():
	try:
		OX234F7A = OX1E0F0C.cursor()  
		OX5F4E7D = request.remote_addr
		OX7D9B8E = str(int(time.time()*100))
		OX4C8B7E, OX1A26B0, OX6B3F1D = OX7B4DF339()
		OX3A4E8C = '|'.join(OX6B3F1D)
		OX1D2C0D = '|'.join(['0']*OX1A26B0)
		OX2A4C7E = "insert into t_vote_info(FUid, FVoteId, FTitle, FOptionNum, \
				FOptionDesc, FOptionVoteNum, FState, FCreateTime, FEndTime) \
				values(%s,%s,%s,%s,%s,%s,0,now(),now()+interval 1 day);" 
		OX4E3D5D = (OX5F4E7D, OX7D9B8E, OX4C8B7E, OX1A26B0, OX3A4E8C, OX1D2C0D) 
		OX7B3F9C = OX234F7A.execute(OX2A4C7E, OX4E3D5D)
		OX1E0F0C.commit()
		OX234F7A.close()
	except Exception,OX7F3B3F:
		return jsonify({"return_code":21, "return_msg":str(OX7F3B3F), "p_id":0})
	return jsonify({"p_id":OX7D9B8E})

@OX4A6E3E.route('/poll', methods=['POST','GET'])
def OX9B2C3A():
	if "p_id" in request.args:
		OX5B3E6D = request.args['p_id']
		OX234F7A = OX1E0F0C.cursor()
		OX3D4E6C = "select FTitle, FOptionDesc from t_vote_info where FVoteId=%s;"
		OX7B3F9C = OX234F7A.execute(OX3D4E6C, (OX5B3E6D,))
		OX7E1B2F = OX234F7A.fetchone()
		OX234F7A.close()
		OX4C8B7E = OX7E1B2F[0]
		OX2F3C7D = OX7E1B2F[1].split('|')
		return render_template("poll.html", title=OX4C8B7E, opts=OX2F3C7D)

	if "p_id" not in request.form:
		return render_template("poll.html")
	if "opt_idx" not in request.form:
		return render_template("poll.html")

	OX4D8C7F = int(request.form['opt_idx'])-1
	OX5B3E6D = request.form['p_id']
	try:
		OX234F7A = OX1E0F0C.cursor()
		OX3D4E6C = "select FOptionVoteNum from t_vote_info where FVoteId=%s;"
		OX7B3F9C = OX234F7A.execute(OX3D4E6C, (OX5B3E6D,))
		OX3F7D2B = OX234F7A.fetchone()[0].split('|')
		OX3F7D2B[OX4D8C7F] = str(int(OX3F7D2B[OX4D8C7F])+1)
		OX2E3B8D = '|'.join(OX3F7D2B)
		OX8B4E6C = "update t_vote_info set FOptionVoteNum=%s where FVoteId=%s;"
		OX7B3F9C = OX234F7A.execute(OX8B4E6C, (OX2E3B8D,OX5B3E6D))
		OX1E0F0C.commit()
		OX234F7A.close()
	except Exception,OX7F3B3F:
		OX234F7A.close()
		return jsonify({"result_code":"-1", "result_msg":"error", "p_id":0})
	return jsonify({"result_code":"0", "result_msg":"success", "p_id":OX5B3E6D})

@OX4A6E3E.route('/show')
def OX3B2F7D():
	OX4C8B7E = "error"
	if "p_id" not in request.args:
		return render_template("show.html", title=OX4C8B7E)
	OX5B3E6D = request.args['p_id']
	OX6C4B8E = []
	try:
		OX234F7A = OX1E0F0C.cursor()
		OX3D4E6C = "select FTitle,FOptionDesc,FOptionVoteNum,FState,FEndTime from t_vote_info where FVoteId=%s;"
		OX7B3F9C = OX234F7A.execute(OX3D4E6C, (OX5B3E6D,))
		OX7E1B2F = OX234F7A.fetchone()
		OX234F7A.close()
		OX4C8B7E = OX7E1B2F[0]
		OX2F3C7D = OX7E1B2F[1].split('|')
		OX6F3E7C = OX7E1B2F[2].split('|')
		OX5D3F6A = OX512E6E[:len(OX2F3C7D)]
		for OX3F8B2D in range(len(OX2F3C7D)):
			OX6C4B8E.append([OX2F3C7D[OX3F8B2D], OX6F3E7C[OX3F8B2D], OX5D3F6A[OX3F8B2D]])
	except Exception,OX7F3B3F:
		return render_template("show.html", title=OX4C8B7E)
	return render_template("show.html", title=OX4C8B7E, opts=OX6C4B8E)

@OX4A6E3E.route('/refresh', methods=['POST'])
def OX8D4E3F():
	if "p_id" not in request.form:
		return jsonify({"result_code":"-1", "result_msg":"refresh error"})
	OX5B3E6D = request.form['p_id']
	OX6C4B8E = []
	try:
		OX234F7A = OX1E0F0C.cursor()
		OX3D4E6C = "select FTitle,FOptionDesc,FOptionVoteNum,FState,FEndTime from t_vote_info where FVoteId=%s;"
		OX7B3F9C = OX234F7A.execute(OX3D4E6C, (OX5B3E6D,))
		OX7E1B2F = OX234F7A.fetchone()
		OX234F7A.close()
		OX4C8B7E = OX7E1B2F[0]
		OX2F3C7D = OX7E1B2F[1].split('|')
		OX6F3E7C = OX7E1B2F[2].split('|')
		OX5D3F6A = OX512E6E[:len(OX2F3C7D)]
		for OX3F8B2D in range(len(OX2F3C7D)):
			OX6C4B8E.append({"label":OX2F3C7D[OX3F8B2D], "value":int(OX6F3E7C[OX3F8B2D]), "color":OX5D3F6A[OX3F8B2D]})
		return jsonify({"result_code":"0", "result_msg":"success", "rows":OX6C4B8E})
	except Exception,OX7F3B3F:
		return jsonify({"result_code":"-1", "result_msg":"refresh error"})

if __name__ == '__main__':
    OX4A6E3E.debug = True
    OX4A6E3E.run()
    OX1E0F0C.close()