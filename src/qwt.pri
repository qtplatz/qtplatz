QWT = $$(QWT)
isEmpty( QWT ) {
    win32: QWT = C:/Qwt-6.0.3-svn
    macx|linux-*: QWT = /usr/local/qwt-6.0.3-svn
}

INCLUDEPATH += $${QWT}/include
LIBS += -L$${QWT}/lib -l$$qtLibraryTarget(qwt)


