# OpenBabel
include ( config.pri )
include ( ./cleanpath.pri )

INCLUDEPATH += $${OPENBABEL_INCLUDE}

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
} else {
    OPENBABEL_LIBDIR = /usr/local/lib
    macx: OPENBABEL_DLLS   = $${OPENBABEL_LIBDIR}/libopenbabel.4.0.0.dylib
    linux-*: OPENBABEL_DLLS = 
}
