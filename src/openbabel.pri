
include ( ./cleanpath.pri )

isEmpty( $$OPENBABEL_SRC ) {
    win32: OPENBABEL_SRC = $$cleanPath( $$PWD/../../openbabel )
}

isEmpty( $$OPENBABEL_ROOT ) {
    win32: OPENBABEL_ROOT = $${OPENBABEL_SRC}/windows-vc2008/install
    else: OPENBABEL_ROOT = /usr/local/openbabel
}

win32 {
    INCLUDEPATH += $${OPENBABEL_ROOT}/include/openbabel-2.0 \
		   $${OPENBABEL_SRC}/windows-vc2008/include
    LIBS += -L$${OPENBABEL_ROOT}/bin 
} else {
    INCLUDEPATH += /usr/local/openbabel/include
    QMAKE_LFLAGS += -L/usr/local/openbabel/lib
}

LIBS += -lopenbabel-2

win32 {
    OPENBABEL_EXTLIB_DIR = $${OPENBABEL_SRC}/windows-vc2008/libs/i386
    OPENBABEL_DLLS = 	$${OPENBABEL_ROOT}/bin/openbabel-2.dll \
			$${OPENBABEL_EXTLIB_DIR}/freetype6.dll \
			$${OPENBABEL_EXTLIB_DIR}/iconv.dll \
			$${OPENBABEL_EXTLIB_DIR}/libcairo-2.dll \
			$${OPENBABEL_EXTLIB_DIR}/libexpat-1.dll \
			$${OPENBABEL_EXTLIB_DIR}/libfontconfig-1.dll \
			$${OPENBABEL_EXTLIB_DIR}/libinchi.dll \
			$${OPENBABEL_EXTLIB_DIR}/libpng14-14.dll \
			$${OPENBABEL_EXTLIB_DIR}/libxml2.dll \
			$${OPENBABEL_EXTLIB_DIR}/xdr-0.dll \
			$${OPENBABEL_EXTLIB_DIR}/zlib1.dll
    OPENBABEL_FILES = 	$${OPENBABEL_ROOT}/bin/formats_cairo.obf \
			$${OPENBABEL_ROOT}/bin/formats_common.obf \
			$${OPENBABEL_ROOT}/bin/formats_compchem.obf \
			$${OPENBABEL_ROOT}/bin/formats_misc.obf \
			$${OPENBABEL_ROOT}/bin/formats_utility.obf \
			$${OPENBABEL_ROOT}/bin/formats_xml.obf \
			$${OPENBABEL_ROOT}/bin/plugin_charges.obf \
			$${OPENBABEL_ROOT}/bin/plugin_descriptors.obf \
			$${OPENBABEL_ROOT}/bin/plugin_fingerprints.obf \
			$${OPENBABEL_ROOT}/bin/plugin_forcefields.obf \
			$${OPENBABEL_ROOT}/bin/plugin_ops.obf
}
