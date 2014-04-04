##################################################
# Makefile for QtPlatz/QtSPCView
#
# Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
# Author: Toshinobu Hondo
######################################
SH	= CMD.EXE
TOP    	= ..

TARGET 	= Release|Win32
QTPLATZ_PATH = ..\..\..\..\qtplatz
RM	= rm -f

CANDLE = "$(WIX)bin\Candle.exe"
LIGHT = "$(WIX)\bin\Light.exe"
LIT = "$(WIX)\bin\Lit.exe"
DARK = "$(WIX)\bin\Dark.exe"
HEAT = "$(WIX)\bin\Heat.exe"
MELT = "$(WIX)\bin\Melt.exe"
TORCH = "$(WIX)\bin\Torch.exe"
SMOKE = "$(WIX)\bin\Smoke.exe"
PYRO = "$(WIX)\bin\Pyro.exe"

all: qtspcview.msi

clean:
	$(RM) *.msi *.wixobj *.wixpdb version.wxi *.zip *.bat *.exe *.obj *~

version.exe: version.c
	cl version.c

version.wxi: version.exe
	git describe | version > version.wxi

copy.bat: version.exe
	git describe | version -b > copy.bat

qtspcview.msi: qtspcview.wixobj
	$(LIGHT) qtspcview.wixobj
	signtool sign /t http://timestamp.digicert.com /a qtspcview.msi
	copy.bat

sign:
	signtool sign /t http://timestamp.digicert.com /a qtspcview.msi

qtspcview.wixobj: version.wxi copy.bat qtspcview.wxs
	$(CANDLE) qtspcview.wxs

