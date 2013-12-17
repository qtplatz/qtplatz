# RDKit
include ( config.pri )
include ( ./cleanpath.pri )

RDBASE=$$(RDBASE)

isEmpty( RDBASE ) {
  RDBASE=$$cleanPath( $$PWD/../../rdkit )
}

INCLUDEPATH += $${RDBASE}/Code

win32 {

   win32-msvc2008: VC=90
   win32-msvc2010: VC=100
   win32-msvc2012: VC=110
   win32-msvc2013: VC=120
   
   RDKIT_LIBRARY=$${RDBASE}/build_$${QMAKE_HOST.arch}_$${VC}/lib
   # message( "LIB" $${RDKIT_LIBRARY} )      

  CONFIG(debug, debug|release) {
    LIBS += -L$${RDKIT_LIBRARY}/Debug
  } else {
    LIBS += -L$${RDKIT_LIBRARY}/Release
  }
} else {
  LIBS += -L$${RDBASE}/lib
}
