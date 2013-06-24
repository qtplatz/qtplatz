include( config.pri )

INCLUDEPATH += $${QWT}/include
LIBS += -L$${QWT}/lib -l$$qtLibraryTarget(qwt)


