# Makefile for qtPlatz/InfiTOF
#
# Copyright (C) 2010 Toshinobu Hondo, Ph.D.
# Author: Toshinobu Hondo
#
######################################

#------------------------------------- 
#
# Target builds for MC3.1 Software
#
#------------------------------------- 
SH	= CMD.EXE
TOP    	= ..
NSIS_DIR = "C:\Program Files\NSIS"
NSIS	= $(NSIS_DIR)\makensis.exe

TARGET 	= Release|Win32

QTPLATZ_PATH = .

all:
	@echo "	---------------------------------------------------------------"
	@echo "	Hello, Welcome to InfiTOF/qtPlatz distribution makefile"
	@echo "	---------------------------------------------------------------"

world: qtplatz

debug-world:
	@(MAKE) TARGET="Debug|Win32"

clean:
	devenv $(QTPLATZ_PATH)\qtPlatz-vc9.sln /Clean "$(TARGET)"

qtplatz:
	devenv $(QTPLATZ_PATH)\qtPlatz-vc9.sln /Build "$(TARGET)"

#nsis: version.exe
#	git describe | version "!define VERSION	" > $(INFITOF_PATH)\installer\infiTof\version.nsh
#	$(NSIS) $(INFITOF_PATH)\installer\infiTof\infitof.nsi

version.exe: version.c
	cl version.c

tag:
	version "cvs -d :ssh:$(CVSUSER)@$(CVSHOST):$(CVSROOT) tag " > $(SRCDIR)\MC\tag.bat
	cd $(SRCDIR)\MC
	tag.bat

update:
	git update

pull:
	git pull

push:
	git push


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

