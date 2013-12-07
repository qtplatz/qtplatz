# RDKit
include ( config.pri )
include ( ./cleanpath.pri )

RDBASE=$$(RDBASE)     # only requied for windows

isEmpty( RDBASE ) {
  RDBASE=$$cleanPath( $$PWD/../../rdkit )
}

INCLUDEPATH += $${RDBASE}/Code
LIBS += -L$${RDBASE}/lib
