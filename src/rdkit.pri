# RDKit
include ( config.pri )
include ( ./cleanpath.pri )

RDBASE=$$(RDBASE)

isEmpty( RDBASE ) {
  RDBASE=$$cleanPath( $$PWD/../../rdkit )
}

INCLUDEPATH += $${RDBASE}/Code

win32 {
  CONFIG(debug, debug|release) {
    LIBS += -L$${RDBASE}/build/lib/Debug
  } else {
    LIBS += -L$${RDBASE}/build/lib/Release
  }
} else {
  LIBS += -L$${RDBASE}/lib
}
