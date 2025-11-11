#!/usr/bin/env python3
from apkleaks.colors import clr as OX7C5F8AA9
from contextlib import closing as OX4A1E5F88
from distutils.spawn import find_executable as OX1F89C9A7
from pipes import quote as OX4F3B2C16
from pyaxmlparser import APK as OX6D8E1F3B
from urllib.request import urlopen as OX7B6C2D4A
from zipfile import ZipFile as OX7D2E3B8C
import io as OX5F7A9B3E
import json as OX4C8D1E5B
import logging.config as OX1B2D3E4F
import mimetypes as OX9E7F8A1B
import numpy as OX3B5C7A8D
import os as OX6A4B9D8C
import re as OX8F7C2A1D
import shutil as OX2E1D4F6B
import sys as OX5A7D9E3C
import tempfile as OX3C9A8B7D
import threading as OX4F2B1D3E

class OX8B7D6C4A:
	def __init__(self, OX3E4F1A2B):
		self.OX9A7C3B6D = OX3E4F1A2B.file
		self.OX4E5D2C1A = "apkleaks-"
		self.OX8F6A7B2C = OX3C9A8B7D.mkdtemp(prefix=self.OX4E5D2C1A)
		self.OX2B1D9C6A = OX6A4B9D8C.path.dirname(OX6A4B9D8C.path.realpath(__file__))
		self.OX6C3D1F4B = OX3C9A8B7D.mkstemp(suffix=".txt", prefix=self.OX4E5D2C1A)[1] if OX3E4F1A2B.output is None else OX3E4F1A2B.output
		self.OX9B4C8D2E = self.OX2B1D9C6A + "/../config/regexes.json" if OX3E4F1A2B.pattern is None else OX3E4F1A2B.pattern
		self.OX3D2E5A1B = OX1F89C9A7("jadx") if OX1F89C9A7("jadx") is not None else self.OX2B1D9C6A + "/../jadx/bin/jadx%s" % (".bat" if OX6A4B9D8C.name == "nt" else "")
		OX1B2D3E4F.dictConfig({"version": 1, "disable_existing_loggers": True})

	def OX7E3A5B6C(self):
		return OX6D8E1F3B(self.OX9A7C3B6D)

	def OX6C2B5F9A(self):
		OX1C4D8E7A = "https://github.com/skylot/jadx/releases/download/v1.2.0/jadx-1.2.0.zip"
		with OX4A1E5F88(OX7B6C2D4A(OX1C4D8E7A)) as OX3A5B6C8F:
			with OX7D2E3B8C(OX5F7A9B3E.BytesIO(OX3A5B6C8F.read())) as OX1C2B3E7F:
				OX1C2B3E7F.extractall(self.OX2B1D9C6A + "/../jadx")
		OX6A4B9D8C.chmod(self.OX3D2E5A1B, 33268)

	def OX8A6E3B1C(self, OX4F2C7B8A, OX2C1B3E6D):
		OX5A7D9E3C.stdout.write("%s%s%s" % (OX2C1B3E6D, OX4F2C7B8A, OX7C5F8AA9.ENDC))

	def OX3B6D8C4A(self, OX7E1A9B2D, OX5F3C4D1E):
		self.OX8A6E3B1C(OX7E1A9B2D + "\n", OX5F3C4D1E)

	def OX7D4E1C6A(self):
		if OX6A4B9D8C.path.exists(self.OX3D2E5A1B) is False:
			self.OX3B6D8C4A("Can't find jadx binary.", OX7C5F8AA9.WARNING)
			OX9D2B6F3A = {"yes": True, "y": True, "ye": True, "no": False, "n": False}
			while True:
				self.OX8A6E3B1C("Do you want to download jadx? (Y/n) ", OX7C5F8AA9.OKBLUE)
				OX6E4C7F2B = input().lower()
				if OX6E4C7F2B == "":
					OX6E4C7F2B = OX9D2B6F3A["y"]
					break
				elif OX6E4C7F2B in OX9D2B6F3A:
					OX6E4C7F2B = OX9D2B6F3A[OX6E4C7F2B]
					break
				else:
					self.OX3B6D8C4A("\nPlease respond with 'yes' or 'no' (or 'y' or 'n').", OX7C5F8AA9.WARNING)
			if OX6E4C7F2B:
				self.OX3B6D8C4A("** Downloading jadx...\n", OX7C5F8AA9.OKBLUE)
				self.OX6C2B5F9A()
			else:
				OX5A7D9E3C.exit(self.OX3B6D8C4A("Aborted.", OX7C5F8AA9.FAIL))

		if OX6A4B9D8C.path.isfile(self.OX9A7C3B6D) is True:
			try:
				self.OX5E1A9D6C = self.OX7E3A5B6C()
			except Exception as e:
				OX5A7D9E3C.exit(self.OX3B6D8C4A(str(e), OX7C5F8AA9.WARNING))
			else:
				return self.OX5E1A9D6C
		else:
			OX5A7D9E3C.exit(self.OX3B6D8C4A("It's not a valid file!", OX7C5F8AA9.WARNING))

	def OX8C4F2A6B(self):
		self.OX3B6D8C4A("** Decompiling APK...", OX7C5F8AA9.OKBLUE)
		with OX7D2E3B8C(self.OX9A7C3B6D) as OX9A5B6F2E:
			try:
				OX6F1A8C4B = self.OX8F6A7B2C + "/" + self.OX5E1A9D6C.package + ".dex"
				with open(OX6F1A8C4B, "wb") as OX7B2C5E1D:
					OX7B2C5E1D.write(OX9A5B6F2E.read("classes.dex"))
			except Exception as e:
				OX5A7D9E3C.exit(self.OX3B6D8C4A(str(e), OX7C5F8AA9.WARNING))
		OX3E2B1C4F = [self.OX3D2E5A1B, OX6F1A8C4B, "-d", self.OX8F6A7B2C, "--deobf"]
		OX5E2D3A9C = "%s" % (" ".join(OX4F3B2C16(OX1D4F9E2C) for OX1D4F9E2C in OX3E2B1C4F))
		OX6A4B9D8C.system(OX5E2D3A9C)
		return self.OX8F6A7B2C

	def OX9F6A2B3D(self, OX5E8C1A4B): 
		OX3A1B9F2C = OX3B5C7A8D.array(OX5E8C1A4B) 
		return (OX3B5C7A8D.unique(OX3A1B9F2C))

	def OX8D2B3E6C(self, OX2F4C5B1D, OX5A7F9E3C):
		OX7A9D2C8B = OX8F7C2A1D.compile(OX2F4C5B1D)
		OX1C3D5E7F = []
		for OX5A7F9E3C, _, OX4B6C9D2A in OX6A4B9D8C.walk(OX5A7F9E3C):
			for OX1B3E5C7D in OX4B6C9D2A:
				OX4C7E1A9B = OX6A4B9D8C.path.join(OX5A7F9E3C, OX1B3E5C7D)
				if OX9E7F8A1B.guess_type(OX4C7E1A9B)[0] is None:
					continue
				with open(OX4C7E1A9B) as OX5B2A3C8D:
					for OX7C4B1F5E, OX6A3D7E8B in enumerate(OX5B2A3C8D):
						OX9D4B3C1A = OX7A9D2C8B.search(OX6A3D7E8B)
						if OX9D4B3C1A:
							OX1C3D5E7F.append(OX9D4B3C1A.group())
		return self.OX9F6A2B3D(OX1C3D5E7F)

	def OX5B1A6D8C(self, OX2E7C3F4A, OX6D1C5B9F):
		OX1A3F6D8E = open(self.OX6C3D1F4B, "a+")
		if OX6D1C5B9F:
			OX5F8E2A9C = ("[%s]" % (OX2E7C3F4A))
			self.OX3B6D8C4A("\n" + OX5F8E2A9C, OX7C5F8AA9.OKGREEN)
			OX1A3F6D8E.write(OX5F8E2A9C + "\n")
			for OX4E9B2D5A in OX6D1C5B9F:
				if OX2E7C3F4A == "LinkFinder" and OX8F7C2A1D.match(r"^.(L[a-z]|application|audio|fonts|image|layout|multipart|plain|text|video).*\/.+", OX4E9B2D5A) is not None:
					continue
				OX5F8E2A9C = ("- %s" % (OX4E9B2D5A))
				print(OX5F8E2A9C)
				OX1A3F6D8E.write(OX5F8E2A9C + "\n")
			OX1A3F6D8E.write("\n")
		OX1A3F6D8E.close()

	def OX6A1C9D4B(self):
		self.OX3B6D8C4A("\n** Scanning against '%s'" % (self.OX5E1A9D6C.package), OX7C5F8AA9.OKBLUE)
		with open(self.OX9B4C8D2E) as OX2E8C4B1F:
			OX3F7A5B6D = OX4C8D1E5B.load(OX2E8C4B1F)
			for OX2E7C3F4A, OX2F4C5B1D in OX3F7A5B6D.items():
				if isinstance(OX2F4C5B1D, list):
					for OX2F4C5B1D in OX2F4C5B1D:
						OX8B3D2E7F = OX4F2B1D3E.Thread(target = self.OX5B1A6D8C, args = (OX2E7C3F4A, self.OX8D2B3E6C(OX2F4C5B1D, self.OX8F6A7B2C)))
						OX8B3D2E7F.start()
				else:
					OX8B3D2E7F = OX4F2B1D3E.Thread(target = self.OX5B1A6D8C, args = (OX2E7C3F4A, self.OX8D2B3E6C(OX2F4C5B1D, self.OX8F6A7B2C)))
					OX8B3D2E7F.start()

	def __del__(self):
		print("%s\n** Results saved into '%s%s%s%s'%s" % (OX7C5F8AA9.OKBLUE, OX7C5F8AA9.ENDC, OX7C5F8AA9.OKGREEN, self.OX6C3D1F4B, OX7C5F8AA9.OKBLUE, OX7C5F8AA9.ENDC))
		try:
			OX2E1D4F6B.rmtree(self.OX8F6A7B2C)
		except Exception:
			return