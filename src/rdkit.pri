# RDKit
include ( config.pri )
include ( ./cleanpath.pri )

RDBASE=$$(RDBASE)

isEmpty( RDBASE ) {
  RDBASE=$$cleanPath( $$PWD/../../rdkit )
}

INCLUDEPATH += $${RDBASE}/Code
# message( "RDBASE=" $${RDBASE} )

win32 {
  CONFIG(debug, debug|release) {
    LIBS += -L$${RDBASE}/lib/Debug
  } else {
    LIBS += -L$${RDBASE}/lib/Release
  }
} else {
  LIBS += -L$${RDBASE}/lib
}
