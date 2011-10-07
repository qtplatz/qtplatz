macx {
#    INCLUDEPATH += $$(QWT)/lib/qwt.framework/Headers
    INCLUDEPATH += $$(QWT)/include
    LIBS += -L$$(QWT)/lib -l$$qtLibraryTarget(qwt)
} else {
    INCLUDEPATH += $$(QWT)/include
    LIBS += -L$$(QWT)/lib -lqwt
}
