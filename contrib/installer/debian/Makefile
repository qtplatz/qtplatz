#!GNUmake

ARCH := $(shell uname -s | tr '[A-Z]' '[a-z]')

KERNEL_RELEASE=`uname -r`

QT_VERSION  = 4.8.4
ACE_VERSION = 6.1.8
TAO_VERSION = 2.1.8
QTPLATZDIR  = ../../..

# target app version
VERSION = 1.3.1

# source directory prefix
PREFIX = /usr/local
#PREFIX = /opt

TAO_LIBDIR = ${PREFIX}/ace+tao/${ACE_VERSION}

DESTDIR = opt/qtplatz
#TAO_DESTDIR = /opt/ace+tao/${TAO_VERSION}
#QT_DESTDIR  = /opt/qt/${QT_VERSION}

TAODIR = ${PREFIX}/ace+tao/${TAO_VERSION}

TAO_FILES = libACE.so.${ACE_VERSION} \
	libTAO.so.${TAO_VERSION} \
	libTAO_Utils.so.${TAO_VERSION} \
	libTAO_PI.so.${TAO_VERSION} \
	libTAO_AnyTypeCode.so.${TAO_VERSION} \
	libTAO_PortableServer.so.${TAO_VERSION} \
	libTAO_CodecFactory.so.${TAO_VERSION} \
	libTAO_Codeset.so.${TAO_VERSION} \
	libTAO_Compression.so.${TAO_VERSION} \
	libTAO_ZIOP.so.${TAO_VERSION} \
	libTAO_RTCORBA.so.${TAO_VERSION}

all: debian.deb

clean:
	-rm -rf md5sums ./debian *.deb *~ control postinst prerm

debian.deb: Makefile control postinst prerm
	mkdir -p ./debian/DEBIAN
	mkdir -p ./debian/${DESTDIR}
	( cd ${QTPLATZDIR}; \
	  tar cf - ./bin ./lib ./share/translations ) | \
			( cd ./debian/${DESTDIR}; tar xvf - )
	for i in ${TAO_FILES };\
	  do \
		cp -p ${TAO_LIBDIR}/$${i} ./debian/${DESTDIR}/lib; \
	  done
	find ./debian -type d | xargs chmod 755
	( cd ./debian; find . -type f -exec md5sum {} \; > ../md5sums )
	cp -p control md5sums postinst prerm ./debian/DEBIAN
	chmod 555 ./debian/DEBIAN/postinst ./debian/DEBIAN/prerm
	dpkg-deb --build debian

control: Makefile
	sed 's/Version:.*$$/Version: ${VERSION}/' < control.in > control

postinst: Makefile
	sed 's/VERSION=.*$$/VERSION=${VERSION}/' < postinst.in > postinst

prerm: Makefile
	sed 's/VERSION=.*$$/VERSION=${VERSION}/' < prerm.in > prerm

