
QTDIR = /usr/local/Qt/4.8.1
DEST  = /Volumes/qtplatz

ACE_VERSION = 6.0.0
TAO_VERSION = 1.6.6

LIBDIRIN = /usr/local/lib
LIBDIROUT = /Volumes/qtplatz/qtplatz.app/Contents/PlugIns

TAO_LIBS = \
	libTAO \
	libTAO_Utils \
	libTAO_PI \
	libTAO_PortableServer \
	libTAO_AnyTypeCode

BOOST_LIBS = \
	libboost_serialization \
	libboost_wserialization \
	libboost_date_time \
	libboost_system \
	libboost_filesystem

all: /Volumes/qtplatz/qtplatz.app

/Volumes/qtplatz/qtplatz.app: /Volumes/qtplatz
	cd ../../../bin; tar cf - ./qtplatz.app | (cd /Volumes/qtplatz; tar xvf -)
	for i in ${BOOST_LIBS} ; do \
	  cp -p ${LIBDIRIN}/$$i.dylib ${LIBDIROUT}; \
	done
	cp -p ${LIBDIRIN}/libACE-${ACE_VERSION}.dylib ${LIBDIROUT}
	( cd ${LIBDIROUT}; ln -s libACE-${ACE_VERSION}.dylib libACE.dylib )
	for i in ${TAO_LIBS} ; do \
	  cp -p ${LIBDIRIN}/$$i.${TAO_VERSION}.dylib ${LIBDIROUT}; \
	  ( cd ${LIBDIROUT}; ln -s $$i.${TAO_VERSION}.dylib $$i.dylib ); \
	done

deploy:
	macdeployqt /Volumes/qtplatz/qtplatz.app

/Volumes/qtplatz: ./qtplatz.dmg
	open ./qtplatz.dmg
	sleep 2

qtplatz.dmg:
	hdiutil create -megabytes 300 -fs HFS+ -volname qtplatz ./qtplatz

clean: eject
	-rm -f ./qtplatz.dmg

eject:
	-umount /Volumes/qtplatz
