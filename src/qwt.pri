macx {
    QWT = /usr/local/qwt-6.0.1-svn
    INCLUDEPATH += /usr/local/qwt-6.0.1-svn/lib/qwt.framework/Headers
    LIBS += -L$$QWT/lib -l$$qtLibraryTarget(qwt)
} else {
    INCLUDEPATH += $$(QWT)/include
    LIBS += -L$$(QWT)/lib -lqwt
}
