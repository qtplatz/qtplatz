##################################################
# Makefile for QtPlatz
#
# Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
# Author: Toshinobu Hondo
#
######################################
#------------------------------------- 
SH	= CMD.EXE
TOP    	= ..
#NSIS_DIR = "C:\Program Files\NSIS"
NSIS_DIR = "C:\Program Files (x86)\NSIS"
NSIS	 = $(NSIS_DIR)\makensis.exe

TARGET 	= Release|Win32

QTPLATZ_PATH = ..\..\..\..\qtplatz

all:
	@echo "	---------------------------------------------------------------------"
	@echo "	Hello, Welcome to QtPlatz installer makefile"
	@echo ""
	@echo "	To build binaries,"
	@echo "	'nmake world'
	@echo ""
	@echo "	To create setup from binaries,"
	@echo "	'nmake nsis'
	@echo "	on VS-2012 command console"
	@echo "	---------------------------------------------------------------------"

world: qtplatz

clean:
	msbuild $(QTPLATZ_PATH)\qtplatz.sln /m:4 /t:Clean /p:Configuration=Release

qtplatz:
	msbuild $(QTPLATZ_PATH)\qtplatz.sln /m:4 /t:Rebuild /p:Configuration=Release

nsis: version.exe
	git describe | version "!define VERSION	" > version.nsh
	$(NSIS) qtplatz.nsi

version.exe: version.c
	cl version.c

tag:
	echo "git tag -a v2.0 -m 'description...'"
	echo "git push --tags"

pull:
	$(CMD) "cd $(QTPLATZ_PATH); git pull"

veryclean:
	-rm -rf $(SRCDIR)

#---------------------------------------
#
# Clean build directory
#
#---------------------------------------

clean :		
	cd ../qtplatz
	devenv qtplatz.sln /Clean

