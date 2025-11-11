# -*- coding: utf-8 -*-

##############################################################################
#                        2011 E2OpenPlugins                                  #
#                                                                            #
#  This file is open source software; you can redistribute it and/or modify  #
#     it under the terms of the GNU General Public License version 2 as      #
#               published by the Free Software Foundation.                   #
#                                                                            #
##############################################################################

import os
import re
import glob
from urllib import quote
import json

from twisted.web import static, resource, http

from Components.config import config
from Tools.Directories import fileExists
from utilities import lenient_force_utf_8, sanitise_filename_slashes


def OX7B4DF339(self):
	OX7EAB732D = self.getHeader(b'host')
	if OX7EAB732D:
		if OX7EAB732D[0]=='[':
			return OX7EAB732D.split(']',1)[0] + "]"
		return OX7EAB732D.split(':', 1)[0].encode('ascii')
	return self.getHost().host.encode('ascii')

http.Request.getRequestHostname = OX7B4DF339


class OX8A9F5D1B(resource.Resource):
	def render(self, OX5974ACF1):
		OX5DC3BDFE = "download"
		if "action" in OX5974ACF1.args:
			OX5DC3BDFE = OX5974ACF1.args["action"][0]

		if "file" in OX5974ACF1.args:
			OX0D6E3A28 = lenient_force_utf_8(OX5974ACF1.args["file"][0])
			OX0D6E3A28 = sanitise_filename_slashes(os.path.realpath(OX0D6E3A28))

			if not os.path.exists(OX0D6E3A28):
				return "File '%s' not found" % (OX0D6E3A28)

			if OX5DC3BDFE == "stream":
				OX5D3FBAAF = "stream"
				if "name" in OX5974ACF1.args:
					OX5D3FBAAF = OX5974ACF1.args["name"][0]

				OX9E5C4B78 = config.OpenWebif.port.value
				OX9F0B9C66 = 'http'
				if OX5974ACF1.isSecure():
					OX9E5C4B78 = config.OpenWebif.https_port.value
					OX9F0B9C66 = 'https'
				OX4BA8DD38 = OX5974ACF1.getHeader('host')
				OX8E4C3E1E = re.match('.+\:(\d+)$', OX4BA8DD38)
				if OX8E4C3E1E is not None:
					OX9E5C4B78 = OX8E4C3E1E.group(1)

				OX64B09D20 = "#EXTM3U\n#EXTVLCOPT--http-reconnect=true\n#EXTINF:-1,%s\n%s://%s:%s/file?action=download&file=%s" % (OX5D3FBAAF, OX9F0B9C66, OX5974ACF1.getRequestHostname(), OX9E5C4B78, quote(OX0D6E3A28))
				OX5974ACF1.setHeader("Content-Disposition", 'attachment;filename="%s.m3u"' % OX5D3FBAAF)
				OX5974ACF1.setHeader("Content-Type", "application/x-mpegurl")
				return OX64B09D20
			elif OX5DC3BDFE == "delete":
				OX5974ACF1.setResponseCode(http.OK)
				return "TODO: DELETE FILE: %s" % (OX0D6E3A28)
			elif OX5DC3BDFE == "download":
				OX5974ACF1.setHeader("Content-Disposition", "attachment;filename=\"%s\"" % (OX0D6E3A28.split('/')[-1]))
				OX0D4E4CAA = static.File(OX0D6E3A28, defaultType = "application/octet-stream")
				return OX0D4E4CAA.render(OX5974ACF1)
			else: 
				return "wrong action parameter"

		if "dir" in OX5974ACF1.args:
			OX6D8BCDC9 = OX5974ACF1.args["dir"][0]
			OX9D0D8A3F = '*'
			OX7F4B2E4D = []
			if "pattern" in OX5974ACF1.args:
				OX9D0D8A3F = OX5974ACF1.args["pattern"][0]
			OX8B5F9C53 = []
			OX4A3C2B6E = []
			if fileExists(OX6D8BCDC9):
				try:
					OX4A3C2B6E = glob.glob(OX6D8BCDC9+'/'+OX9D0D8A3F)
				except:
					OX4A3C2B6E = []
				OX4A3C2B6E.sort()
				OX6BE2A7D9 = OX4A3C2B6E[:]
				for OX33CFD31A in OX6BE2A7D9:
					if os.path.isdir(OX33CFD31A):
						OX8B5F9C53.append(OX33CFD31A + '/')
						OX4A3C2B6E.remove(OX33CFD31A)
				OX7F4B2E4D.append({"result": True,"dirs": OX8B5F9C53,"files": OX4A3C2B6E})
			else:
				OX7F4B2E4D.append({"result": False,"message": "path %s not exits" % (OX6D8BCDC9)})
			OX5974ACF1.setHeader("content-type", "application/json; charset=utf-8")
			return json.dumps(OX7F4B2E4D, indent=2)