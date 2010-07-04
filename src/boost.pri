win32 {
    INCLUDEPATH += $$(BOOST_ROOT)/include/boost-1_43
    LIBS += -L$$(BOOST_ROOT)/lib
} else {
    INCLUDEPATH += /usr/local/include
}
