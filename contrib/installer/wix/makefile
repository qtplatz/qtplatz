#!nmake
##################################################
# Makefile for QtPlatz
#
# Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
# Author: Toshinobu Hondo
#
######################################
SH	= CMD.EXE
TOP    	= ..

QTPLATZ_PATH = ..\..\..
INFITOF_PATH = $(QTPLATZ_PATH)\..\infitof

CANDLE = "$(WIX)bin\Candle.exe"
LIGHT = "$(WIX)\bin\Light.exe"
LIT = "$(WIX)\bin\Lit.exe"
DARK = "$(WIX)\bin\Dark.exe"
HEAT = "$(WIX)\bin\Heat.exe"
MELT = "$(WIX)\bin\Melt.exe"
TORCH = "$(WIX)\bin\Torch.exe"
SMOKE = "$(WIX)\bin\Smoke.exe"
PYRO = "$(WIX)\bin\Pyro.exe"

## following 3 commands are workaround where if PATH for cygwin/bin kind exists
RM	= $(SH) /c del
CP	= $(SH) /c copy
MD      = $(SH) /c mkdir
STAGEDIR = .\stage

first:
	@echo "*	make world	will do full rebuild qtplatz, and generate .MSI setup"
	@echo "*	make harvent	make translations; make qt; and make plugins' for update .wxi files from scratch"
	@echo "*	make all	make .msi from existing binary"
	@echo "*	make clean	cleans up current dir"
	@echo "*	make veryclean	CAUTION this cleans all fules according to git repository. == This makes a state right after clean git clone."

all: qtplatz.msi

world: qtplatz infitof qtplatz.msi

clean:
	$(RM) /S /F /Q $(STAGEDIR)
	$(RM) *.msi *.wixobj *.wixpdb version.wxi *.zip *.bat *.exe *.obj *~

veryclean: clean
	cd $(QTPLATZ_PATH)
	git clean -d -x -f
	cd $(MAKEDIR)
	cd $(INFITOF_PATH)
	git clean -d -x -f
	cd $(MAKEDIR)

qtplatz:
	-$(RM) version.wxi
	cd $(QTPLATZ_PATH)
	qmake -tp vc -r
	cd $(MAKEDIR)
	msbuild $(QTPLATZ_PATH)\qtplatz.sln /m:4 /t:Rebuild /p:Configuration=Release

infitof:
	cd $(INFITOF_PATH)
	qmake -tp vc -r
	cd $(MAKEDIR)
	msbuild $(INFITOF_PATH)\infitof.sln /m:4 /t:Rebuild /p:Configuration=Release

version.exe: version.c
	cl version.c

version.wxi: version.exe ../../../.git
	git describe | version > version.wxi

rename.bat: version.exe
	git describe | version -b > rename.bat

qtplatz.msi: qtplatz.wixobj version.wxi translations.wixobj plugins.wixobj qt.wixobj
	$(LIGHT) -cultures:en-US -ext WixUIExtension.dll \
		qtplatz.wixobj translations.wixobj plugins.wixobj qt.wixobj -o qtplatz.msi
	signtool sign /t http://timestamp.digicert.com /a qtplatz.msi
	rename.bat

sign:
	signtool sign /t http://timestamp.digicert.com /a qtplatz.msi

qtplatz.wixobj: version.wxi rename.bat qtplatz.wxs
	$(CANDLE) qtplatz.wxs

translations.wixobj: translations.wxi
	$(CANDLE) -dQtTranslationsDir=$(QTDIR)\translations translations.wxi

plugins.wixobj: plugins.wxi
	$(CANDLE) -dQtPluginsDir=$(QTDIR)\plugins plugins.wxi

qt.wixobj: qt.wxi 
	$(CANDLE) -dQtBinDir=$(QTDIR)\bin qt.wxi

#translations.wxi: translations
#qt.wxi: qt
#plugins.wxi: plugins

harvest: copy_stage_files qt plugins translations

translations:
	$(HEAT) dir $(QTDIR)\translations \
	-dr TRANSLATIONS -cg QtTranslations -sfrag -gg -g1 -sf -srd -var "var.QtTranslationsDir" \
	-t x64transform.xslt -o translations.wxi

plugins:
	$(HEAT) dir $(STAGEDIR)\plugins \
	-dr PLUGINS -cg QtPluginsComponents -sfrag -gg -g1 -sf -srd -var "var.QtPluginsDir" \
	-t x64transform.xslt -o plugins.wxi

qt:
	$(HEAT) dir $(STAGEDIR)\bin \
	-dr BIN -cg QtComponents -sfrag -gg -g1 -sf -srd -var "var.QtBinDir" \
	-t x64transform.xslt -o qt.wxi

qt_full:
	$(HEAT) dir $(QTDIR)\bin \
	-dr BIN -cg QtComponents -sfrag -gg -g1 -sf -srd -var "var.QtBinDir" \
	-t qt.xslt -o qt.wxi

copy_stage_files:
	rm -rf $(STAGEDIR)
	-@$(MD) $(STAGEDIR)\bin
	-@$(MD) $(STAGEDIR)\plugins
	-@$(MD) $(STAGEDIR)\plugins\accessible
	-@$(MD) $(STAGEDIR)\plugins\imageformats
	-@$(MD)	$(STAGEDIR)\plugins\platforms
	-@$(MD)	$(STAGEDIR)\plugins\sqldrivers
	$(CP) $(QTDIR)\bin\icudt52.dll		$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\icudt52.dll		$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\icuin52.dll		$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\icuuc52.dll		$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5CLucene.dll	$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5Core.dll		$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5Gui.dll		$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5Help.dll		$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5Network.dll	$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5PrintSupport.dll	$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5Script.dll	$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5Sql.dll		$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5Svg.dll		$(STAGEDIR)\bin
	$(CP) $(QTDIR)\bin\Qt5Widgets.dll	$(STAGEDIR)\bin
	$(CP) $(QTDIR)\plugins\accessible\qtaccessiblewidgets.dll $(STAGEDIR)\plugins\accessible
	$(CP) $(QTDIR)\plugins\imageformats\qdds.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qgif.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qicns.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qico.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qjp2.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qjpeg.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qmng.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qsvg.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qtga.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qtiff.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qwbmp.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\imageformats\qwebp.dll	$(STAGEDIR)\plugins\imageformats
	$(CP) $(QTDIR)\plugins\platforms\qwindows.dll	$(STAGEDIR)\plugins\platforms
	$(CP) $(QTDIR)\plugins\sqldrivers\qsqlite.dll	$(STAGEDIR)\plugins\sqldrivers
