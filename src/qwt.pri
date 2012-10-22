QWT = $$(QWT)
isEmpty( QWT ) {
    win32: QWT = C:/Qwt-6.0.2-svn
    macx|linux-*: QWT = /usr/local/qwt-6.0.2-svn
}

macx {
    INCLUDEPATH += $${QWT}/include
    LIBS += -L$${QWT}/lib -l$$qtLibraryTarget(qwt)
} else {
    INCLUDEPATH += $${QWT}/include
    LIBS += -L$${QWT}/lib -lqwt
}
