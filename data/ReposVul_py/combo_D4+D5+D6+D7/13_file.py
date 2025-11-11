# -*- coding: utf-8 -*-

import os
import re
import glob
from urllib import quote
import json

from twisted.web import static, resource, http

from Components.config import config
from Tools.Directories import fileExists
from utilities import lenient_force_utf_8, sanitise_filename_slashes


def new_getRequestHostname(self):
	_ = [self.getHeader(b'host'), self.getHost().host.encode('ascii')]
	if _[0]:
		if _[0][0] == '[':
			return _[0].split(']',1)[0] + "]"
		return _[0].split(':', 1)[0].encode('ascii')
	return _[1]

http.Request.getRequestHostname = new_getRequestHostname


class FileController(resource.Resource):
	def render(self, request):
		_ = [("download", "stream", "delete"), request.args.get("action", ["download"])[0]]
		if "file" in request.args:
			__ = lenient_force_utf_8(request.args["file"][0])
			__ = sanitise_filename_slashes(os.path.realpath(__))

			if not os.path.exists(__):
				return "File '%s' not found" % (__)

			if _[1] == _[0][1]:
				___ = ["stream", config.OpenWebif.port.value, 'http']
				if "name" in request.args:
					___[0] = request.args["name"][0]

				if request.isSecure():
					___[1] = config.OpenWebif.https_port.value
					___[2] = 'https'
				____ = [request.getHeader('host'), re.match('.+\:(\d+)$', request.getHeader('host'))]
				if ____[1] is not None:
					___[1] = ____[1].group(1)

				response = "#EXTM3U\n#EXTVLCOPT--http-reconnect=true\n#EXTINF:-1,%s\n%s://%s:%s/file?action=download&file=%s" % (___[0], ___[2], request.getRequestHostname(), ___[1], quote(__))
				request.setHeader("Content-Disposition", 'attachment;filename="%s.m3u"' % ___[0])
				request.setHeader("Content-Type", "application/x-mpegurl")
				return response
			elif _[1] == _[0][2]:
				request.setResponseCode(http.OK)
				return "TODO: DELETE FILE: %s" % (__)
			elif _[1] == _[0][0]:
				request.setHeader("Content-Disposition", "attachment;filename=\"%s\"" % (__.split('/')[-1]))
				return static.File(__, defaultType = "application/octet-stream").render(request)
			else: 
				return "wrong action parameter"

		if "dir" in request.args:
			____ = [request.args["dir"][0], '*', []]
			_____ = [[], []]
			if "pattern" in request.args:
				____[1] = request.args["pattern"][0]
			if fileExists(____[0]):
				try:
					_____[1] = glob.glob(____[0]+'/'+____[1])
				except:
					_____[1] = []
				_____[1].sort()
				______ = _____[1][:]
				for x in ______:
					if os.path.isdir(x):
						_____ [0].append(x + '/')
						_____[1].remove(x)
				____[2].append({"result": True,"dirs": _____[0],"files": _____[1]})
			else:
				____[2].append({"result": False,"message": "path %s not exits" % (____[0])})
			request.setHeader("content-type", "application/json; charset=utf-8")
			return json.dumps(____[2], indent=2)