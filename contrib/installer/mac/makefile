
DEPLOYQT = /usr/local/Qt5.1.1/5.1.1/clang_64/bin/macdeployqt
APATH = /usr/local/Qt5.1.1/5.1.1/clang_64/lib
APLUGINS = /usr/local/Qt5.1.1/5.1.1/clang_64/plugins
EPATH = @executable_path/../Frameworks
FRAMEWORK = ./qtplatz.app/Contents/Frameworks
PLUGINS   = ./qtplatz.app/Contents/PlugIns

#QT_VERSION = 5.1.1
#ACE_VERSION = 6.1.4
#TAO_VERSION = 2.1.4
#BOOST_VERSION = 1_51
#OPENBABEL_VERSION = 4.0.0

#QTDIR = /usr/local/qt/${QT_VERSION}
#DEST  = /Volumes/qtplatz

$(warning $(ACE_ROOT) $(origin ACE_ROOT))

LIBDIR_OPENBABEL = /usr/local/lib
LIBDIR_ACETAO    = $(ACE_ROOT)/lib

TAO_LIBS = \
	libACE \
	libTAO \
	libTAO_Utils \
	libTAO_PI \
	libTAO_PortableServer \
	libTAO_AnyTypeCode \
	libTAO_CodecFactory

#OPENBABEL_LIBS = libopenbabel

all:
	@echo "################################################"
	@echo "ACE_ROOT is: " $(ACE_ROOT)
	@echo "LIBDIR_ACETAO is: " $(LIBDIR_ACETAO)
	@echo "DEPLOYQT is: " $(DEPLOYQT)
	@echo "################################################"
	@echo " -- do following procedure -- or make world -- "
	@echo "################################################"
	@echo "make app"
	@echo "make deployqt"
	@echo "make fix"
	@echo "make dmg"
	@echo

world:
	make app
	make deployqt
	make fix
	make dmg

# all: qtplatz.app

qtplatz.app:
	cp -rp ../../../bin/qtplatz.app .

app:
	cp -rp ../../../bin/qtplatz.app .

thank:
	sh ./thankslasconic.sh ./qtplatz.app/Contents/MacOS/qtplatz

deploy2:
	mkdir -p ${PLUGINS}/platforms
	cp -p ${APLUGINS}/platforms/libqcocoa.dylib ${PLUGINS}/platforms

deployqt:
	${DEPLOYQT} ./qtplatz.app -verbose=2

#fix: fix_frameworks fix_platforms fix_accessible fix_imageformats fix_printsupport fix_ace_tao
fix: copy_ace_tao fix_ace_tao

fix_frameworks:
	for j in QtCore ; do \
		install_name_tool -change $(APATH)/$$j.framework/Versions/5/$$j $(EPATH)/$$j.framework/Versions/5/$$j \
		        $(FRAMEWORK)/QtGui.framework/Versions/5/QtGui ; \
	done
	for j in QtCore QtGui ; do \
		install_name_tool -change $(APATH)/$$j.framework/Versions/5/$$j $(EPATH)/$$j.framework/Versions/5/$$j \
		        $(FRAMEWORK)/QtWidgets.framework/Versions/5/QtWidgets ;\
	done
	for j in QtCore QtGui QtWidgets ; do \
		install_name_tool -change $(APATH)/$$j.framework/Versions/5/$$j $(EPATH)/$$j.framework/Versions/5/$$j \
		        $(FRAMEWORK)/QtPrintSupport.framework/Versions/5/QtPrintSupport ;\
	done

fix_platforms:
	for i in libqcocoa.dylib ; do \
	    for j in QtCore QtGui QtWidgets QtPrintSupport ; do \
		install_name_tool -change $(APATH)/$$j.framework/Versions/5/$$j $(EPATH)/$$j.framework/Versions/5/$$j \
		        $(PLUGINS)/platforms/$$i; \
	    done; \
	done

fix_accessible:
	for i in libqtaccessiblewidgets.dylib ; do \
	    for j in QtCore QtGui QtWidgets; do \
		install_name_tool -change $(APATH)/$$j.framework/Versions/5/$$j $(EPATH)/$$j.framework/Versions/5/$$j \
		        $(PLUGINS)/accessible/$$i; \
	    done; \
	done

fix_imageformats:
	for i in libqgif.dylib libqico.dylib libqjpeg.dylib libqmng.dylib libqtga.dylib libqtiff.dylib libqwbmp.dylib ; do \
	    for j in QtCore QtGui QtWidgets; do \
		install_name_tool -change $(APATH)/$$j.framework/Versions/5/$$j $(EPATH)/$$j.framework/Versions/5/$$j \
		        $(PLUGINS)/imageformats/$$i; \
	    done; \
	done

fix_printsupport:
	for i in libcocoaprintersupport.dylib ; do \
	    for j in QtCore QtGui QtWidgets QtPrintSupport ; do \
		install_name_tool -change $(APATH)/$$j.framework/Versions/5/$$j $(EPATH)/$$j.framework/Versions/5/$$j \
		        $(PLUGINS)/printsupport/$$i; \
	    done; \
	done

copy_ace_tao:
	for i in $(TAO_LIBS) ; do \
	  cp -p $(ACE_ROOT)/lib/$$i.dylib $(FRAMEWORK); \
	done

fix_ace_tao:
	for i in $(TAO_LIBS) ; do \
	  for j in $(TAO_LIBS); do \
		install_name_tool -change $$j.dylib $(EPATH)/$$j.dylib $(FRAMEWORK)/$$i.dylib ; \
	  done; \
	done

dmg: qtplatz.app
	hdiutil create qtplatz-`git describe`.dmg -srcfolder ./qtplatz.app

mount:
	open ./qtplatz.dmg

clean:
	rm -rf qtplatz.app

otool:
	otool -L qtplatz.app/Contents/MacOS/qtplatz
	otool -L qtplatz.app/Contents/Plugins/QtProject/*.dylib
	otool -L qtplatz.app/Contents/Plugins/*.dylib
	otool -L qtplatz.app/Contents/Plugins/MS-Cheminformatics/*.dylib
	for i in QtCore QtGui QtWidgets QtPrintSupport; do \
		otool -L ${FRAMEWORK}/$$i.framework/Versions/5/$$i; \
	done
	@echo "-------- ---------"
	otool -L ${PLUGINS}/platforms/libqcocoa.dylib

# /Volumes/qtplatz/qtplatz.app: /Volumes/qtplatz
# 	cd ../../../bin; tar cf - ./qtplatz.app | (cd /Volumes/qtplatz; tar xvf -)
# 	-mkdir -p ${FRAMEWORKS}
# 	for i in ${TAO_LIBS} ; do \
# 	  cp -p ${LIBDIR_ACETAO}/$$i.dylib ${FRAMEWORKS}; \
# 	done
# 	/usr/local/qt/4.8.4/bin/macdeployqt /Volumes/qtplatz/qtplatz.app -verbose=2

# /Volumes/qtplatz: ./qtplatz.dmg
# 	open ./qtplatz.dmg
# 	sleep 2

# qtplatz.dmg:
# 	hdiutil create -megabytes 300 -fs HFS+ -volname qtplatz ./qtplatz

# clean: eject
# 	-rm -f ./qtplatz.dmg

# eject:
# 	-umount /Volumes/qtplatz

# mount:
# 	open ./qtplatz.dmg
