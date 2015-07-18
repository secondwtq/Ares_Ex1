# coding: utf-8

# no shebang since it does not run on Unix!
#
# Copyright (c) 2015, secondwtq
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	Hououin 2013.12.30EVE-2013.12.31
#		Ares(++) builder with Python
#		still many things to do
#		there is a no-law-to-see structure for this
#		
#	2014.01.18PM
#		Added error report feature, with exception
#		Will use subprocess for processing command, BUUUUT maybe a total rewrite
#	2015.07.13EVE
#		Fixed a bug of 'lib' command with source path containing spaces
#		py3k fix
#		other updates
#	2015.07.14
#		minor update
#

import os
import os.path
import sys

Path_yrpp = r"E:\Ares\YRpp-master"
Path_Aressrc = r"E:\Ares\Ares-src"
BuildPath = r"E:\Ares\Build";
# Path_WinSDK = r"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include"
# Path_WinSDK_Lib = r"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib"
Path_WinSDK = r"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include"
Path_WinSDK_Lib = r"C:\Program Files\Microsoft SDKs\Windows\v6.0A\Lib"
Path_CPP_Libstd = r"D:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include"
Path_CPP_Libstd_lib = r"D:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\lib"

global_config = {
	'command': {
		'cl': 'cl',
		'link': 'link',
		'lib': 'lib',
		'rc': 'rc'
	},
	'library': {
		# 150714: Newer YRpp requires ole32 for YRComHelpers
		#	you can leave it empty
		'ole32': 'ole32.lib'
	},
	'target': {
		'dll': r"E:\Ares\Ares_Ex.dll",
		'yrpp_lib': r"E:\Ares\Build\YRPP.lib"
	}
}

def pathit(src):
	return '"' + src + '"'

def path_addext(src):
	if src[-1] != '\\':
		src += '\\'
	return src

def path_rmext(src):
	if src[-1] == '\\':
		src = src[0, -1]
	return src

def path_getfilename(src):
	return os.path.splitext(src)[0]

def path_getext(src):
	return os.path.splitext(src)[-1]

def composite_linker_cmd(cpps, out, yrpp_lib):
	ret = global_config['command']['link'] + " /MANIFEST:NO /DLL /LIBPATH:" + pathit(Path_WinSDK_Lib) + ' /LIBPATH:' + pathit(Path_CPP_Libstd_lib)
	for cpp in cpps:
		ret += ' ' + pathit(cpp.get_buildpath_full())

	lib_ole32 = ''
	if global_config['library']['ole32']:
		lib_ole32 = pathit(global_config['library']['ole32'])

	ret += ' /SUBSYSTEM:WINDOWS /DEFAULTLIB:"user32.lib" "dbghelp.lib" ' + lib_ole32 + ' /IMPLIB:' + pathit(yrpp_lib) + " /OUT:" + pathit(out)
	return ret

def composite_compiler_cmd(incl, src, out):
	ret = global_config['command']['cl'] + " /Od /W4"
	if incl != "":
		ret += " /I " + pathit(incl)
	ret +=  " /I " + pathit(Path_WinSDK) + " /I " + pathit(Path_CPP_Libstd)
	ret += ' /wd4100 /wd4731 /wd4740 /DWIN32 /D_WINDOWS /D_CRT_SECURE_NO_WARNINGS /DNOMINMAX /EHsc /Gz /Zp8 /Gm /Zi /GS- /nologo /we4035 /we4715' # + " /P"
	# ret += ' /D_USING_V110_SDK71_'
	ret += " /c " + pathit(src)
	ret += " /Fo" + pathit(out)
	return ret

def composite_resource_cmd(incl, src, out):
	ret = global_config['command']['rc'] + " /i " + pathit(Path_CPP_Libstd) + " /i " + pathit(Path_WinSDK)
	if incl != "":
		ret += " /i " + pathit(incl)
	ret += " /v /fo " + pathit(out) + " " + pathit(src)
	return ret

def runcmd(cmd, out=False, extra=""):
	try:
		po = os.popen(cmd)
		info = po.read()
		if out:
			print(info)
		if po.close():
			raise CompileErrorException(extra, info)
			return False
		else:
			return True
	except IOError:
		print("IOError")
		return False

class BuildUnit:
	stype = "unit_base"
	filename = ""
	filepath = ""

	def __init__(self, name, path):
		self.filename = name
		self.filepath = path

	def __repr__(self):
		return self.stype + ' ' + self.filepath + " " + self.filename

	def __str__(self):
		return self.__repr__()

	def get_fullpath(self):
		return Path_Aressrc + self.filepath + self.filename

	def get_filename(self):
		return path_getfilename(self.filename)

	def get_buildpath_full(self):
		return BuildPath+self.filepath+self.get_filename()+".obj"

	def getbuildcmd(self):
		return ""

	def build(self, out=False):
		try:
			os.makedirs(BuildPath+self.filepath)
		except WindowsError:
			pass
		cmd = self.getbuildcmd()
		print(cmd)
		runcmd(cmd, out, self.filepath + self.filename)

class BuildUnitCXX(BuildUnit):
	stype = "unit_cpp"

	def getbuildcmd(self):
		return composite_compiler_cmd(Path_yrpp, self.get_fullpath(), self.get_buildpath_full())

class BuildUnitRC(BuildUnit):
	stype = "unit_rc"

	def get_buildpath_full(self):
		return BuildPath+self.filepath+self.get_filename()+".res"

	def getbuildcmd(self):
		return composite_resource_cmd(Path_yrpp, self.get_fullpath(), self.get_buildpath_full())

class CompileErrorException(Exception):
	'''Exception for some file failed compiling'''

	path = ""
	desc = ""

	def __init__(self, filepath, description):
		Exception.__init__(self)
		self.path = filepath
		self.desc = description

	def __str__(self):
		return "Compile Error At " + self.path + "\n\n" + self.desc

	def __repr__(self):
		return self.__str__()

def getDirs(src):
	src = path_addext(src)
	a = os.listdir(src)
	b = [ src+x for x in a if os.path.isdir(src+x) ]
	return b

def construct_unit(filename, path):
	if path_getext(filename).lower() == ".cpp":
		return BuildUnitCXX(filename, path)
	elif path_getext(filename).lower() == ".rc":
		return BuildUnitRC(filename, path)

def get_cxxs(src, base=""):
	src = path_addext(src)
	base = path_rmext(base)
	a = os.listdir(src)
	b = [ construct_unit(x, src[src.find(base)+len(base) : ]) for x in a if os.path.isfile(src+x) and (os.path.splitext(x)[-1].lower() == ".cpp" or os.path.splitext(x)[-1].lower() == ".rc") ]
	return b

def build_yrpp():
	cmd = composite_compiler_cmd(Path_yrpp, Path_yrpp + "\\StaticInits.cpp", BuildPath + "\\YRPP.obj")
	print(cmd)
	runcmd(cmd, True, "\\StaticInits.cpp")
	cmd = global_config['command']['lib'] + ' ' + pathit(BuildPath + "\\YRPP.obj")
	print(cmd)
	runcmd(cmd, True, "\\StaticInits.cpp")

def get_all_cxxs(src, base):
	if getDirs(src) == []:
		return get_cxxs(src, base)
	ret = []
	for x in getDirs(src):
		ret += get_all_cxxs(x, base)
	return ret + get_cxxs(src, base)

def build_all(units):
	for x in units:
		print(x)
		x.build(True)

def link(units, target, lib):
	cmdlnk = composite_linker_cmd(units, target, lib)
	print(cmdlnk)
	runcmd(cmdlnk, True)

def main():
	build_yrpp();
	cpps = get_all_cxxs(Path_Aressrc, Path_Aressrc)
	build_all(cpps)
	link(cpps, global_config['target']['dll'], global_config['target']['yrpp_lib'])
	sys.stderr.write("\nBuild completed successfully.\n")
	return 0

if __name__ == '__main__':
	STATUS = main()
	sys.exit(STATUS)
