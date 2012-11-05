
QTDIR = /usr/local/qt/4.8.3
DEST  = /Volumes/qtplatz

ACE_VERSION = 6.1.4
TAO_VERSION = 2.1.4
BOOST_VERSION = 1_51
OPENBABEL_VERSION = 4.0.0

LIBDIR_OPENBABEL = /usr/local/lib
LIBDIR_ACETAO    = /usr/local/ace+tao/${ACE_VERSION}/lib
LIBDIR_BOOST     = /usr/local/boost-${BOOST_VERSION}/lib

LIBDIRIN = /usr/local/lib
LIBDIROUT = /Volumes/qtplatz/qtplatz.app/Contents/PlugIns
FRAMEWORKS = /Volumes/qtplatz/qtplatz.app/Contents/Frameworks

TAO_LIBS = \
	libACE \
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

OPENBABEL_LIBS = libopenbabel

all: qtplatz.dmg
	macdeployqt ../../../bin/qtplatz.app -dmg

#all: /Volumes/qtplatz/qtplatz.app

/Volumes/qtplatz/qtplatz.app: /Volumes/qtplatz
	cd ../../../bin; tar cf - ./qtplatz.app | (cd /Volumes/qtplatz; tar xvf -)
	macdeployqt /Volumes/qtplatz/qtplatz.app
	for i in ${TAO_LIBS} ; do \
	  cp -p ${LIBDIR_ACETAO}/$$i.dylib ${FRAMEWORKS}; \
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
