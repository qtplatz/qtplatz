
QT_VERSION = 4.8.4
ACE_VERSION = 6.1.4
TAO_VERSION = 2.1.4
BOOST_VERSION = 1_51
OPENBABEL_VERSION = 4.0.0

QTDIR = /usr/local/qt/${QT_VERSION}
DEST  = /Volumes/qtplatz

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
	libTAO_AnyTypeCode \
	libTAO_CodecFactory

BOOST_LIBS = \
	libboost_serialization \
	libboost_wserialization \
	libboost_date_time \
	libboost_system \
	libboost_filesystem

OPENBABEL_LIBS = libopenbabel

installer: /Volumes/qtplatz/qtplatz.app

all:
	${QTDIR}/bin/macdeployqt ../../../bin/qtplatz.app -verbose=2 -dmg

/Volumes/qtplatz/qtplatz.app: /Volumes/qtplatz
	cd ../../../bin; tar cf - ./qtplatz.app | (cd /Volumes/qtplatz; tar xvf -)
	-mkdir -p ${FRAMEWORKS}
	for i in ${TAO_LIBS} ; do \
	  cp -p ${LIBDIR_ACETAO}/$$i.dylib ${FRAMEWORKS}; \
	done
	/usr/local/qt/4.8.4/bin/macdeployqt /Volumes/qtplatz/qtplatz.app -verbose=2

/Volumes/qtplatz: ./qtplatz.dmg
	open ./qtplatz.dmg
	sleep 2

qtplatz.dmg:
	hdiutil create -megabytes 300 -fs HFS+ -volname qtplatz ./qtplatz

clean: eject
	-rm -f ./qtplatz.dmg

eject:
	-umount /Volumes/qtplatz

mount:
	open ./qtplatz.dmg
