##################################################
# Makefile for QtPlatz
#
# Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
# Author: Toshinobu Hondo
######################################
SH	= CMD.EXE
TOP    	= ..

#TARGET 	= Release|Win32
QTPLATZ_PATH = ..\..\..\..\qtplatz
INFITOF_PATH = ..\..\..\..\infitof

CANDLE = "$(WIX)bin\Candle.exe"
LIGHT = "$(WIX)\bin\Light.exe"
LIT = "$(WIX)\bin\Lit.exe"
DARK = "$(WIX)\bin\Dark.exe"
HEAT = "$(WIX)\bin\Heat.exe"
MELT = "$(WIX)\bin\Melt.exe"
TORCH = "$(WIX)\bin\Torch.exe"
SMOKE = "$(WIX)\bin\Smoke.exe"
PYRO = "$(WIX)\bin\Pyro.exe"
#RM	= rm -f
RM	= del

all: qtplatz.msi

clean:
	$(RM) *.msi *.wixobj *.wixpdb version.wxi *.zip *.bat *.exe *.obj *~

veryclean: clean
	msbuild $(QTPLATZ_PATH)\qtplatz.sln /m:4 /t:Clean /p:Configuration=Release

qtplatz:
	msbuild $(QTPLATZ_PATH)\qtplatz.sln /m:4 /t:Rebuild /p:Configuration=Release

infitof:
	msbuild $(INFITOF_PATH)\infitof.sln /m:4 /t:Rebuild /p:Configuration=Release

version.exe: version.c
	cl version.c

version.wxi: version.exe
	git describe | version > version.wxi

copy.bat: version.exe
	git describe | version -b > copy.bat

qtplatz.msi: qtplatz.wixobj
	$(LIGHT) qtplatz.wixobj
	signtool sign /t http://timestamp.digicert.com /a qtplatz.msi
	copy.bat

sign:
	signtool sign /t http://timestamp.digicert.com /a qtplatz.msi

qtplatz.wixobj: version.wxi copy.bat qtplatz.wxs
	$(CANDLE) qtplatz.wxs

