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
NSIS	= $(NSIS_DIR)\makensis.exe

TARGET 	= Release|Win32

QTPLATZ_PATH = ..\..\..\qtplatz

all:
	@echo "	---------------------------------------------------------------------"
	@echo "	Hello, Welcome to QtPlatz installer makefile"
	@echo ""
	@echo "	To build binaries,"
	@echo "	'nmake world'
	@echo ""
	@echo "	To create setup from binaries,"
	@echo "	'nmake nsis'
	@echo "	on VS-2009 command console"
	@echo "	---------------------------------------------------------------------"

world: qtplatz

clean:
	devenv $(QTPLATZ_PATH)\qtplatz-vc9.sln /Clean "$(TARGET)"
	devenv $(QTPLATZ_PATH)\contrib\contrib.sln /Clean "$(TARGET)"

qtplatz:
	devenv $(QTPLATZ_PATH)\qtplatz-vc9.sln /Build "$(TARGET)"

contrib:
	devenv $(QTPLATZ_PATH)\contrib\contrib.sln /Build "$(TARGET)"

nsis: version.exe
	git describe | version "!define VERSION	" > version.nsh
	$(NSIS) qtplatz.nsi

version.exe: version.c
	cl version.c

cvs_increment_version:
	cvs -d :ssh:$(CVSUSER)@$(CVSHOST):$(CVSROOT) commit -m "new release" -f version.c
	cl version.c
	version "RELEASE=" > nmake.resp

tag:
	echo "todo"
	echo "git tag -a "v1.3" -m "description..."
	echo "git push --tags"

git_update:
	$(CMD) "cd $(QTPLATZ_PATH); git update"

git_checkout:

veryclean:
	-rm -rf $(SRCDIR)

#---------------------------------------
#
# Clean build directory
#
#---------------------------------------

clean :		
	cd ../qtPlatz
	devenv qtPlatz-vc9.sln /Clean
	cd ..\MC4Utils
	devenv MC4Utils.sln /Clean
	cd ..\..

