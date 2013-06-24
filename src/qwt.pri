include( config.pri )

INCLUDEPATH += $${QWT}/include
macx: LIBS += -L$${QWT}/lib -lqwt
else: LIBS += -L$${QWT}/lib -l$$qtLibraryTarget(qwt)

