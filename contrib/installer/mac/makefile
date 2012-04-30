
QTDIR = /usr/local/Qt/4.8.0
DEST  = /Volumes/qtplatz

QTDLLS = \
	QtCLucene4.dylib \
	QtCore4.dylib \
	QtDeclarative4.dylib \
	QtGui4.dylib \
	QtHelp4.dylib \
	QtMultimedia4.dylib \
	QtNetwork4.dylib \
	QtOpenGL4.dylib \
	QtScript4.dylib \
	QtScriptTools4.dylib \
	QtSql4.dylib \
	QtSvg4.dylib \
	QtWebKit4.dylib \
	QtXml4.dylib \
	QtXmlPatterns4.dylib \
	phonon4.dylib

all: /Volumes/qtplatz /Volumes/qtplatz/qtplatz.app


/Volumes/qtplatz/qtplatz.app: /Volumes/qtplatz
	echo cd ../../../bin; tar cf - ./qtplatz.app | (cd /Volumes/qtplatz; tar xvf -)
	for i in $(QTDLLS) ; do \
		cp -p $(QTDIR)/$$i $(DEST)/qtplatz.app/Contents/PlugIns ; \
	done


/Volumes/qtplatz: ./qtplatz.dmg
	open ./qtplatz.dmg
	sleep 3

qtplatz.dmg:
	hdiutil create -megabytes 200 -fs HFS+ -volname qtplatz ./qtplatz

clean: eject
	-rm -f ./qtplatz.dmg

eject:
	-umount /Volumes/qtplatz
